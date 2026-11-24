#include "logger.h"
#include <iostream>
using namespace std;

Logger::Logger()
{
    echo = false;
    reset();
}

Logger::~Logger()
{
    close();
}

void Logger::reset()
{
    completed = 0;
    failed = 0;
    conflicts = 0;
    reroutes = 0;
    releases = 0;
    totalCost = 0;
    batteryUsed = 0;
    responseSum = 0;
    responseMax = 0;
    lines = 0;
}

bool Logger::open(const string& scenarioName)
{
    close();
    scenario = scenarioName;
    logPath = "output/" + scenario + "_log.txt";
    file.open(logPath.c_str());
    if (!file)
    {
        file.clear();
        logPath = scenario + "_log.txt";
        file.open(logPath.c_str());
        if (!file)
        {
            logPath = "";
            return false;
        }
        cout << "Note: output folder not found, writing log to " << logPath << "\n";
    }
    file << "# Simulation record for scenario " << scenario << "\n";
    file << "# Format: T=step TYPE details\n";
    return true;
}

void Logger::close()
{
    if (file.is_open())
    {
        file.close();
    }
}

void Logger::setEcho(bool on)
{
    echo = on;
}

void Logger::write(int step, const string& type, const string& details)
{
    int s = step;
    if (s < 0)
    {
        s = 0;
    }

    string zeros = "";
    if (s < 100)
    {
        zeros += "0";
    }
    if (s < 10)
    {
        zeros += "0";
    }

    string line = "T=" + zeros + to_string(s) + " " + type;
    if (!details.empty())
    {
        line += " " + details;
    }

    if (file.is_open())
    {
        file << line << "\n";
        lines++;
        if (lines % 200 == 0)
        {
            file.flush();
        }
    }
    if (echo)
    {
        cout << line << "\n";
    }
}

void Logger::addCompleted(int responseTime)
{
    completed++;
    if (responseTime < 0)
    {
        responseTime = 0;
    }
    responseSum += responseTime;
    if (responseTime > responseMax)
    {
        responseMax = responseTime;
    }
}

void Logger::addFailed()
{
    failed++;
}

void Logger::addConflict()
{
    conflicts++;
}

void Logger::addReroute()
{
    reroutes++;
}

void Logger::addRelease()
{
    releases++;
}

void Logger::addCost(int cost)
{
    if (cost > 0)
    {
        totalCost += cost;
    }
}

void Logger::addBattery(int amount)
{
    if (amount > 0)
    {
        batteryUsed += amount;
    }
}

int Logger::getCompleted() const
{
    return completed;
}

int Logger::getFailed() const
{
    return failed;
}

int Logger::getConflicts() const
{
    return conflicts;
}

int Logger::getReroutes() const
{
    return reroutes;
}

int Logger::getReleases() const
{
    return releases;
}

long long Logger::getTotalCost() const
{
    return totalCost;
}

long long Logger::getBatteryUsed() const
{
    return batteryUsed;
}

int Logger::getMaxResponse() const
{
    return responseMax;
}

const string& Logger::getLogPath() const
{
    return logPath;
}

double Logger::getAverageResponse() const
{
    if (completed == 0)
    {
        return 0.0;
    }
    return (double)responseSum / completed;
}

void Logger::printStats(ostream& out, Robot* robots, int robotCount,
                        int requestCount, int stepsRun, double runMs) const
{
    ios::fmtflags oldFlags = out.flags();
    streamsize oldPrec = out.precision();
    out.setf(ios::fixed);
    out.precision(2);

    int left = requestCount - completed - failed;
    if (left < 0)
    {
        left = 0;
    }

    out << "===== Final statistics: " << scenario << " =====\n";
    out << "Steps run:              " << stepsRun << "\n";
    out << "Requests total:         " << requestCount << "\n";
    out << "Requests completed:     " << completed << "\n";
    out << "Requests failed:        " << failed << "\n";
    out << "Requests left:          " << left << "\n";
    out << "Average response time:  " << getAverageResponse() << " steps\n";
    out << "Longest response time:  " << responseMax << " steps\n";
    out << "Total cost moved:       " << totalCost << "\n";
    out << "Total battery used:     " << batteryUsed << "\n";
    out << "Conflicts:              " << conflicts << "\n";
    out << "Reroutes:               " << reroutes << "\n";
    out << "Releases:               " << releases << "\n";
    out << "Run time:               " << runMs << " ms\n";

    out << "\nRobot utilisation (busy steps / steps run):\n";
    double busySum = 0.0;
    for (int i = 0; i < robotCount; i++)
    {
        double pct = 0.0;
        if (stepsRun > 0)
        {
            pct = robots[i].getBusySteps() * 100.0 / stepsRun;
        }
        busySum += pct;
        out << "  Robot " << robots[i].getId() << " (" << categoryName(robots[i].getSkill()) << "): "
            << robots[i].getBusySteps() << " busy steps, " << pct << "%\n";
    }
    if (robotCount > 0)
    {
        out << "Average utilisation:    " << busySum / robotCount << "%\n";
    }

    out.flags(oldFlags);
    out.precision(oldPrec);
}

bool Logger::writeStatsFile(Robot* robots, int robotCount, int requestCount,
                            int stepsRun, double runMs, string& pathOut) const
{
    pathOut = "output/" + scenario + "_stats.txt";
    ofstream out(pathOut.c_str());
    if (!out)
    {
        pathOut = scenario + "_stats.txt";
        out.open(pathOut.c_str());
        if (!out)
        {
            return false;
        }
    }
    printStats(out, robots, robotCount, requestCount, stepsRun, runMs);
    return true;
}