#pragma once
#include <string>
#include <fstream>
#include <ostream>
#include "robot.h"
using namespace std;

class Logger
{
private:
    ofstream file;
    string scenario;
    string logPath;
    bool echo;
    int lines;

    int completed;
    int failed;
    int conflicts;
    int reroutes;
    int releases;
    long long totalCost;
    long long batteryUsed;
    long long responseSum;
    int responseMax;

public:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    bool open(const string& scenarioName);
    void close();
    void reset();
    void setEcho(bool on);

    void write(int step, const string& type, const string& details);

    void addCompleted(int responseTime);
    void addFailed();
    void addConflict();
    void addReroute();
    void addRelease();
    void addCost(int cost);
    void addBattery(int amount);

    int getCompleted() const;
    int getFailed() const;
    int getConflicts() const;
    int getReroutes() const;
    int getReleases() const;
    long long getTotalCost() const;
    long long getBatteryUsed() const;
    double getAverageResponse() const;
    int getMaxResponse() const;
    const string& getLogPath() const;

    void printStats(ostream& out, Robot* robots, int robotCount,
                    int requestCount, int stepsRun, double runMs) const;
    bool writeStatsFile(Robot* robots, int robotCount, int requestCount,
                        int stepsRun, double runMs, string& pathOut) const;
};