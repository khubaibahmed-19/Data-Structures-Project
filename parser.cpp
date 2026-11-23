#include "parser.h"
#include <sstream>

const int MAX_TOK = 8;
const int MAX_RECORDS = 1000000;
const int PARAM_COUNT = 5;
const char* PARAM_NAMES[PARAM_COUNT] = {"max_steps", "speed", "battery_per_cost_unit",
                                        "battery_per_work_step", "recharge_per_step"};

static std::string num(int x)
{
    std::ostringstream out;
    out << x;
    return out.str();
}

// ---------------- construction / cleanup ----------------

Parser::Parser()
    : loaded(false), locations(nullptr), locationCount(0), connections(nullptr), connectionCount(0),
      robots(nullptr), robotCount(0), requests(nullptr), requestCount(0), events(nullptr), eventCount(0),
      requestLines(nullptr), eventLines(nullptr), locIndex(nullptr), connIndex(nullptr), robIndex(nullptr),
      reqIndex(nullptr), eventList(nullptr), requestList(nullptr)
{
    clear();
}

Parser::~Parser()
{
    clear();
}

void Parser::clear()
{
    delete[] locations;
    locations = nullptr;
    locationCount = 0;
    delete[] connections;
    connections = nullptr;
    connectionCount = 0;
    delete[] robots;
    robots = nullptr;
    robotCount = 0;
    delete[] requests;
    requests = nullptr;
    requestCount = 0;
    delete[] events;
    events = nullptr;
    eventCount = 0;
    delete[] requestLines;
    requestLines = nullptr;
    delete[] eventLines;
    eventLines = nullptr;
    delete locIndex;
    locIndex = nullptr;
    delete connIndex;
    connIndex = nullptr;
    delete robIndex;
    robIndex = nullptr;
    delete reqIndex;
    reqIndex = nullptr;
    delete eventList;
    eventList = nullptr;
    delete requestList;
    requestList = nullptr;

    params.maxSteps = 0;
    params.speed = 0;
    params.batteryPerCost = 0;
    params.batteryPerWork = 0;
    params.rechargePerStep = 0;
    for (int i = 0; i < PARAM_COUNT; i++)
    {
        paramSet[i] = false;
    }

    scenarioName = "";
    error = "";
    loaded = false;
}

// ---------------- public load ----------------

bool Parser::load(const std::string& filename)
{
    clear();
    if (loadFile(filename))
    {
        loaded = true;
        return true;
    }
    std::string saved = error;
    clear(); // never keep half-loaded data
    error = saved;
    return false;
}

bool Parser::loadFile(const std::string& filename)
{
    std::ifstream in(filename.c_str());
    if (!in)
    {
        return fail(0, "cannot open file '" + filename + "'");
    }

    std::string tok[MAX_TOK];
    int n = 0;
    int lineNo = 0;
    std::string lastSection = "";
    int lastCount = 0;
    bool ended = false;
    int endLine = 0;

    while (nextLine(in, lineNo, tok, n))
    {
        const std::string key = tok[0];
        int count = 0;

        if (key == "END")
        {
            if (n != 1)
            {
                return fail(lineNo, "END must be alone on its line");
            }
            ended = true;
            endLine = lineNo;
            break;
        }
        else if (key == "SCENARIO")
        {
            if (n < 2)
            {
                return fail(lineNo, "SCENARIO needs a name");
            }
            if (!scenarioName.empty())
            {
                return fail(lineNo, "SCENARIO given twice");
            }
            scenarioName = tok[1];
            lastSection = "";
        }
        else if (key == "PARAM")
        {
            if (!readParam(tok, n, lineNo))
            {
                return false;
            }
            lastSection = "";
        }
        else if (key == "LOCATIONS")
        {
            if (locIndex)
            {
                return fail(lineNo, "LOCATIONS section given twice");
            }
            if (!readCount(tok, n, lineNo, 1, count))
            {
                return false;
            }
            if (!readLocations(in, lineNo, count))
            {
                return false;
            }
            lastSection = key;
            lastCount = count;
        }
        else if (key == "CONNECTIONS")
        {
            if (connIndex)
            {
                return fail(lineNo, "CONNECTIONS section given twice");
            }
            if (!locIndex)
            {
                return fail(lineNo, "LOCATIONS must come before CONNECTIONS");
            }
            if (!readCount(tok, n, lineNo, 1, count))
            {
                return false;
            }
            if (!readConnections(in, lineNo, count))
            {
                return false;
            }
            lastSection = key;
            lastCount = count;
        }
        else if (key == "ROBOTS")
        {
            if (robIndex)
            {
                return fail(lineNo, "ROBOTS section given twice");
            }
            if (!locIndex)
            {
                return fail(lineNo, "LOCATIONS must come before ROBOTS");
            }
            if (!readCount(tok, n, lineNo, 1, count))
            {
                return false;
            }
            if (!readRobots(in, lineNo, count))
            {
                return false;
            }
            lastSection = key;
            lastCount = count;
        }
        else if (key == "REQUESTS")
        {
            if (reqIndex)
            {
                return fail(lineNo, "REQUESTS section given twice");
            }
            if (!locIndex)
            {
                return fail(lineNo, "LOCATIONS must come before REQUESTS");
            }
            if (!readCount(tok, n, lineNo, 0, count))
            {
                return false;
            }
            if (!readRequests(in, lineNo, count))
            {
                return false;
            }
            lastSection = key;
            lastCount = count;
        }
        else if (key == "EVENTS")
        {
            if (eventList)
            {
                return fail(lineNo, "EVENTS section given twice");
            }
            if (!locIndex || !connIndex || !robIndex || !reqIndex)
            {
                return fail(lineNo, "EVENTS must come after LOCATIONS, CONNECTIONS, ROBOTS and REQUESTS");
            }
            if (!readCount(tok, n, lineNo, 0, count))
            {
                return false;
            }
            if (!readEvents(in, lineNo, count))
            {
                return false;
            }
            lastSection = key;
            lastCount = count;
        }
        else
        {
            if (!lastSection.empty())
            {
                return fail(lineNo, lastSection + " says " + num(lastCount) +
                                        " records but there are more lines than that");
            }
            return fail(lineNo, "unknown keyword '" + key + "'");
        }
    }

    if (!ended)
    {
        return fail(lineNo, "END line is missing");
    }
    if (nextLine(in, lineNo, tok, n))
    {
        return fail(lineNo, "text found after END");
    }

    if (scenarioName.empty())
    {
        std::string base = filename;
        int slash = -1;
        for (int i = 0; i < (int)base.size(); i++)
        {
            if (base[i] == '/' || base[i] == '\\')
            {
                slash = i;
            }
        }
        base = base.substr(slash + 1);
        int dot = -1;
        for (int i = 0; i < (int)base.size(); i++)
        {
            if (base[i] == '.')
            {
                dot = i;
            }
        }
        if (dot > 0)
        {
            base = base.substr(0, dot);
        }
        scenarioName = base;
    }

    return finalChecks(endLine);
}

// ---------------- reading helpers ----------------

// Reads the next line that is not blank and not a comment, split into words.
bool Parser::nextLine(std::ifstream& in, int& lineNo, std::string tok[], int& n)
{
    std::string line;
    while (std::getline(in, line))
    {
        lineNo++;
        // remove Windows '\r' and surrounding spaces
        int start = 0, end = (int)line.size() - 1;
        while (start <= end && (line[start] == ' ' || line[start] == '\t' || line[start] == '\r'))
        {
            start++;
        }
        while (end >= start && (line[end] == ' ' || line[end] == '\t' || line[end] == '\r'))
        {
            end--;
        }
        if (start > end)
        {
            continue;
        }
        if (line[start] == '#')
        {
            continue;
        }

        std::istringstream words(line.substr(start, end - start + 1));
        std::string w;
        n = 0;
        for (int i = 0; i < MAX_TOK; i++)
        {
            tok[i] = "";
        }
        while (words >> w)
        {
            if (n < MAX_TOK)
            {
                tok[n] = w;
            }
            n++;
        }
        return true;
    }
    n = 0;
    return false;
}

bool Parser::fail(int lineNo, const std::string& reason)
{
    if (lineNo > 0)
    {
        error = "Error line " + num(lineNo) + ": " + reason;
    }
    else
    {
        error = "Error: " + reason;
    }
    return false;
}

bool Parser::readCount(const std::string tok[], int n, int lineNo, int minCount, int& count)
{
    if (n != 2)
    {
        return fail(lineNo, tok[0] + " header must be '" + tok[0] + " count'");
    }
    if (!toInt(tok[1], count))
    {
        return fail(lineNo, tok[0] + " count '" + tok[1] + "' is not a number");
    }
    if (count < minCount)
    {
        return fail(lineNo, tok[0] + " count must be at least " + num(minCount));
    }
    if (count > MAX_RECORDS)
    {
        return fail(lineNo, tok[0] + " count is too large");
    }
    return true;
}

// Reads one record line; fails if the file ends or a new section starts too early.
bool Parser::needsRecord(std::ifstream& in, int& lineNo, std::string tok[], int& n,
                         const std::string& section, int done, int count)
{
    if (!nextLine(in, lineNo, tok, n) || isKeyword(tok[0]))
    {
        return fail(lineNo,
                    section + " says " + num(count) + " records but only " + num(done) + " were found");
    }
    return true;
}

bool Parser::readParam(const std::string tok[], int n, int lineNo)
{
    if (n != 3)
    {
        return fail(lineNo, "PARAM must be 'PARAM key value'");
    }
    int which = -1;
    for (int i = 0; i < PARAM_COUNT; i++)
    {
        if (tok[1] == PARAM_NAMES[i])
        {
            which = i;
        }
    }
    if (which == INVALID)
    {
        return fail(lineNo, "unknown PARAM '" + tok[1] + "'");
    }
    if (paramSet[which])
    {
        return fail(lineNo, "PARAM " + tok[1] + " given twice");
    }

    int value;
    if (!toInt(tok[2], value))
    {
        return fail(lineNo, "PARAM " + tok[1] + " value '" + tok[2] + "' is not a number");
    }
    if ((which == 0 || which == 1) && value < 1)
    {
        return fail(lineNo, "PARAM " + tok[1] + " must be at least 1");
    }
    if (value < 0)
    {
        return fail(lineNo, "PARAM " + tok[1] + " cannot be negative");
    }

    if (which == 0)
    {
        params.maxSteps = value;
    }
    else if (which == 1)
    {
        params.speed = value;
    }
    else if (which == 2)
    {
        params.batteryPerCost = value;
    }
    else if (which == 3)
    {
        params.batteryPerWork = value;
    }
    else
    {
        params.rechargePerStep = value;
    }
    paramSet[which] = true;
    return true;
}

bool Parser::readLocations(std::ifstream& in, int& lineNo, int count)
{
    locations = new LocationRec[count];
    locationCount = count;
    locIndex = new HashTable(count);
    std::string tok[MAX_TOK];
    int n;

    for (int i = 0; i < count; i++)
    {
        if (!needsRecord(in, lineNo, tok, n, "LOCATIONS", i, count))
        {
            return false;
        }
        if (n != 3)
        {
            return fail(lineNo, "location must be 'id name type'");
        }
        int id;
        if (!toInt(tok[0], id) || id <= 0)
        {
            return fail(lineNo, "location id '" + tok[0] + "' must be a positive number");
        }
        int type = locationType(tok[2]);
        if (type == INVALID)
        {
            return fail(lineNo, "unknown location type '" + tok[2] + "'");
        }
        if (!locIndex->put(id, i))
        {
            return fail(lineNo, "duplicate location id " + num(id));
        }

        locations[i].id = id;
        locations[i].name = tok[1];
        locations[i].type = type;
        locations[i].stationUp = true;
    }
    return true;
}

bool Parser::readConnections(std::ifstream& in, int& lineNo, int count)
{
    connections = new ConnectionRec[count];
    connectionCount = count;
    connIndex = new HashTable(count);
    std::string tok[MAX_TOK];
    int n;

    for (int i = 0; i < count; i++)
    {
        if (!needsRecord(in, lineNo, tok, n, "CONNECTIONS", i, count))
        {
            return false;
        }
        if (n != 5)
        {
            return fail(lineNo, "connection must be 'id location_a location_b cost status'");
        }
        int id, a, b, cost;
        if (!toInt(tok[0], id) || id <= 0)
        {
            return fail(lineNo, "connection id '" + tok[0] + "' must be a positive number");
        }
        if (!toInt(tok[1], a))
        {
            return fail(lineNo, "location '" + tok[1] + "' is not a number");
        }
        if (!toInt(tok[2], b))
        {
            return fail(lineNo, "location '" + tok[2] + "' is not a number");
        }
        if (!toInt(tok[3], cost))
        {
            return fail(lineNo, "cost '" + tok[3] + "' is not a number");
        }

        int aIdx = locIndex->get(a);
        int bIdx = locIndex->get(b);
        if (aIdx == INVALID)
        {
            return fail(lineNo,
                        "connection " + num(id) + " joins location " + num(a) + ", which does not exist");
        }
        if (bIdx == INVALID)
        {
            return fail(lineNo,
                        "connection " + num(id) + " joins location " + num(b) + ", which does not exist");
        }
        if (a == b)
        {
            return fail(lineNo, "connection " + num(id) + " joins location " + num(a) + " to itself");
        }
        if (cost < 1)
        {
            return fail(lineNo, "connection " + num(id) + " cost must be at least 1");
        }

        int status;
        if (tok[4] == "OPEN")
        {
            status = OPEN;
        }
        else if (tok[4] == "BLOCKED")
        {
            status = BLOCKED;
        }
        else
        {
            return fail(lineNo, "unknown connection status '" + tok[4] + "'");
        }

        if (!connIndex->put(id, i))
        {
            return fail(lineNo, "duplicate connection id " + num(id));
        }

        connections[i].id = id;
        connections[i].a = a;
        connections[i].b = b;
        connections[i].aIdx = aIdx;
        connections[i].bIdx = bIdx;
        connections[i].cost = cost;
        connections[i].status = status;
    }
    return true;
}

bool Parser::readRobots(std::ifstream& in, int& lineNo, int count)
{
    robots = new Robot[count];
    robotCount = count;
    robIndex = new HashTable(count);
    std::string tok[MAX_TOK];
    int n;

    for (int i = 0; i < count; i++)
    {
        if (!needsRecord(in, lineNo, tok, n, "ROBOTS", i, count))
        {
            return false;
        }
        if (n != 5)
        {
            return fail(lineNo, "robot must be 'id start_location capability max_battery initial_battery'");
        }
        int id, place, maxB, startB;
        if (!toInt(tok[0], id) || id <= 0)
        {
            return fail(lineNo, "robot id '" + tok[0] + "' must be a positive number");
        }
        if (!toInt(tok[1], place))
        {
            return fail(lineNo, "location '" + tok[1] + "' is not a number");
        }
        if (locIndex->get(place) == INVALID)
        {
            return fail(lineNo,
                        "robot " + num(id) + " starts at location " + num(place) + ", which does not exist");
        }
        int skill = skillCode(tok[2]);
        if (skill == INVALID)
        {
            return fail(lineNo, "unknown capability '" + tok[2] + "'");
        }
        if (!toInt(tok[3], maxB))
        {
            return fail(lineNo, "max battery '" + tok[3] + "' is not a number");
        }
        if (!toInt(tok[4], startB))
        {
            return fail(lineNo, "initial battery '" + tok[4] + "' is not a number");
        }
        if (maxB < 1)
        {
            return fail(lineNo, "robot " + num(id) + " max battery must be at least 1");
        }
        if (startB < 0 || startB > maxB)
        {
            return fail(lineNo, "robot " + num(id) + " battery " + num(startB) + " must be between 0 and " +
                                    num(maxB));
        }
        if (!robIndex->put(id, i))
        {
            return fail(lineNo, "duplicate robot id " + num(id));
        }

        robots[i].init(id, place, skill, maxB, startB);
    }
    return true;
}

bool Parser::readRequests(std::ifstream& in, int& lineNo, int count)
{
    if (count > 0)
    {
        requests = new RescueRequest[count];
        requestLines = new int[count];
    }
    requestCount = count;
    reqIndex = new HashTable(count > 0 ? count : 1);
    std::string tok[MAX_TOK];
    int n;

    for (int i = 0; i < count; i++)
    {
        if (!needsRecord(in, lineNo, tok, n, "REQUESTS", i, count))
        {
            return false;
        }
        if (n != 6)
        {
            return fail(lineNo, "request must be 'id report_time location category urgency duration'");
        }
        int id, time, place, urgency, duration;
        if (!toInt(tok[0], id) || id <= 0)
        {
            return fail(lineNo, "request id '" + tok[0] + "' must be a positive number");
        }
        if (!toInt(tok[1], time))
        {
            return fail(lineNo, "report time '" + tok[1] + "' is not a number");
        }
        if (time < 0)
        {
            return fail(lineNo, "request " + num(id) + " report time cannot be negative");
        }
        if (!toInt(tok[2], place))
        {
            return fail(lineNo, "location '" + tok[2] + "' is not a number");
        }
        if (locIndex->get(place) == INVALID)
        {
            return fail(lineNo,
                        "request " + num(id) + " is at location " + num(place) + ", which does not exist");
        }
        int category = skillCode(tok[3]);
        if (category == INVALID || category == GENERAL)
        {
            return fail(lineNo, "unknown request category '" + tok[3] + "'");
        }
        if (!toInt(tok[4], urgency))
        {
            return fail(lineNo, "urgency '" + tok[4] + "' is not a number");
        }
        if (urgency < URGENCY_MIN || urgency > URGENCY_MAX)
        {
            return fail(lineNo, "request " + num(id) + " urgency " + num(urgency) + " must be 1 to 5");
        }
        if (!toInt(tok[5], duration))
        {
            return fail(lineNo, "duration '" + tok[5] + "' is not a number");
        }
        if (duration < 1)
        {
            return fail(lineNo, "request " + num(id) + " duration must be at least 1");
        }
        if (!reqIndex->put(id, i))
        {
            return fail(lineNo, "duplicate request id " + num(id));
        }

        requests[i].init(id, time, place, category, urgency, duration);
        requestLines[i] = lineNo;
    }
    return true;
}

bool Parser::readEvents(std::ifstream& in, int& lineNo, int count)
{
    if (count > 0)
    {
        events = new EventRec[count];
        eventLines = new int[count];
    }
    eventCount = count;
    eventList = new LinkedList();
    std::string tok[MAX_TOK];
    int n;

    for (int i = 0; i < count; i++)
    {
        if (!needsRecord(in, lineNo, tok, n, "EVENTS", i, count))
        {
            return false;
        }
        if (n < 3)
        {
            return fail(lineNo, "event must be 'time TYPE id [value]'");
        }
        int time, target, value = 0;
        if (!toInt(tok[0], time))
        {
            return fail(lineNo, "event time '" + tok[0] + "' is not a number");
        }
        if (time < 0)
        {
            return fail(lineNo, "event time cannot be negative");
        }
        int type = eventType(tok[1]);
        if (type == INVALID)
        {
            return fail(lineNo, "unknown event type '" + tok[1] + "'");
        }

        int wanted = (type == EV_SET_URGENCY) ? 4 : 3;
        if (n != wanted)
        {
            return fail(lineNo, tok[1] + " needs " + num(wanted - 2) + " value(s) after it");
        }
        if (!toInt(tok[2], target))
        {
            return fail(lineNo, "event id '" + tok[2] + "' is not a number");
        }

        if (type == EV_BLOCK || type == EV_UNBLOCK)
        {
            if (connIndex->get(target) == INVALID)
            {
                return fail(lineNo,
                            tok[1] + " refers to connection " + num(target) + ", which does not exist");
            }
        }
        else if (type == EV_STATION_DOWN || type == EV_STATION_UP)
        {
            int idx = locIndex->get(target);
            if (idx == INVALID)
            {
                return fail(lineNo, tok[1] + " refers to location " + num(target) + ", which does not exist");
            }
            if (locations[idx].type != CHARGING)
            {
                return fail(lineNo, tok[1] + " refers to location " + num(target) +
                                        ", which is not a charging station");
            }
        }
        else if (type == EV_SET_URGENCY)
        {
            if (reqIndex->get(target) == INVALID)
            {
                return fail(lineNo,
                            "SET_URGENCY refers to request " + num(target) + ", which does not exist");
            }
            if (!toInt(tok[3], value))
            {
                return fail(lineNo, "urgency '" + tok[3] + "' is not a number");
            }
            if (value < URGENCY_MIN || value > URGENCY_MAX)
            {
                return fail(lineNo, "urgency " + num(value) + " must be 1 to 5");
            }
        }
        else
        {
            if (robIndex->get(target) == INVALID)
            {
                return fail(lineNo, tok[1] + " refers to robot " + num(target) + ", which does not exist");
            }
        }

        events[i].time = time;
        events[i].type = type;
        events[i].target = target;
        events[i].value = value;
        eventLines[i] = lineNo;
    }
    return true;
}

// Checks that need the whole file, then builds the two time-sorted lists.
bool Parser::finalChecks(int endLine)
{
    for (int i = 0; i < PARAM_COUNT; i++)
    {
        if (!paramSet[i])
        {
            return fail(endLine, "PARAM " + std::string(PARAM_NAMES[i]) + " is missing");
        }
    }
    if (!locIndex)
    {
        return fail(endLine, "LOCATIONS section is missing");
    }
    if (!connIndex)
    {
        return fail(endLine, "CONNECTIONS section is missing");
    }
    if (!robIndex)
    {
        return fail(endLine, "ROBOTS section is missing");
    }
    if (!reqIndex)
    {
        return fail(endLine, "REQUESTS section is missing");
    }
    if (!eventList)
    {
        eventList = new LinkedList(); // EVENTS is optional
    }

    for (int i = 0; i < requestCount; i++)
    {
        if (requests[i].getReportTime() > params.maxSteps)
        {
            return fail(requestLines[i], "request " + num(requests[i].getId()) + " report time " +
                                             num(requests[i].getReportTime()) + " is after max_steps " +
                                             num(params.maxSteps));
        }
    }
    for (int i = 0; i < eventCount; i++)
    {
        if (events[i].time > params.maxSteps)
        {
            return fail(eventLines[i], "event at step " + num(events[i].time) + " is after max_steps " +
                                           num(params.maxSteps));
        }
    }

    // insertSorted keeps file order for equal times
    requestList = new LinkedList();
    for (int i = 0; i < requestCount; i++)
    {
        requestList->insertSorted(requests[i].getReportTime(), i);
    }
    for (int i = 0; i < eventCount; i++)
    {
        eventList->insertSorted(events[i].time, i);
    }
    return true;
}

// ---------------- word / number helpers ----------------

bool Parser::toInt(const std::string& s, int& out)
{
    int i = 0, len = (int)s.size();
    bool neg = false;
    if (len == 0)
    {
        return false;
    }
    if (s[0] == '-' || s[0] == '+')
    {
        neg = (s[0] == '-');
        i = 1;
    }
    if (i == len || len - i > 9)
    {
        return false;
    }
    int value = 0;
    for (; i < len; i++)
    {
        if (s[i] < '0' || s[i] > '9')
        {
            return false;
        }
        value = value * 10 + (s[i] - '0');
    }
    out = neg ? -value : value;
    return true;
}

bool Parser::isKeyword(const std::string& s)
{
    return s == "SCENARIO" || s == "PARAM" || s == "LOCATIONS" || s == "CONNECTIONS" || s == "ROBOTS" ||
           s == "REQUESTS" || s == "EVENTS" || s == "END";
}

int Parser::locationType(const std::string& s)
{
    if (s == "JUNCTION")
    {
        return JUNCTION;
    }
    if (s == "BUILDING")
    {
        return BUILDING;
    }
    if (s == "HOSPITAL")
    {
        return HOSPITAL;
    }
    if (s == "DEPOT")
    {
        return DEPOT;
    }
    if (s == "CHARGING")
    {
        return CHARGING;
    }
    if (s == "COMMAND")
    {
        return COMMAND;
    }
    return INVALID;
}

int Parser::skillCode(const std::string& s)
{
    if (s == "MEDICAL")
    {
        return MEDICAL;
    }
    if (s == "FIRE")
    {
        return FIRE;
    }
    if (s == "SUPPLY")
    {
        return SUPPLY;
    }
    if (s == "SEARCH")
    {
        return SEARCH;
    }
    if (s == "GENERAL")
    {
        return GENERAL;
    }
    return INVALID;
}

int Parser::eventType(const std::string& s)
{
    if (s == "BLOCK")
    {
        return EV_BLOCK;
    }
    if (s == "UNBLOCK")
    {
        return EV_UNBLOCK;
    }
    if (s == "STATION_DOWN")
    {
        return EV_STATION_DOWN;
    }
    if (s == "STATION_UP")
    {
        return EV_STATION_UP;
    }
    if (s == "SET_URGENCY")
    {
        return EV_SET_URGENCY;
    }
    if (s == "ROBOT_OFFLINE")
    {
        return EV_ROBOT_OFFLINE;
    }
    if (s == "ROBOT_ONLINE")
    {
        return EV_ROBOT_ONLINE;
    }
    return INVALID;
}

const char* Parser::locationTypeName(int t)
{
    if (t == JUNCTION)
    {
        return "JUNCTION";
    }
    if (t == BUILDING)
    {
        return "BUILDING";
    }
    if (t == HOSPITAL)
    {
        return "HOSPITAL";
    }
    if (t == DEPOT)
    {
        return "DEPOT";
    }
    if (t == CHARGING)
    {
        return "CHARGING";
    }
    if (t == COMMAND)
    {
        return "COMMAND";
    }
    return "UNKNOWN";
}

const char* Parser::eventTypeName(int t)
{
    if (t == EV_BLOCK)
    {
        return "BLOCK";
    }
    if (t == EV_UNBLOCK)
    {
        return "UNBLOCK";
    }
    if (t == EV_STATION_DOWN)
    {
        return "STATION_DOWN";
    }
    if (t == EV_STATION_UP)
    {
        return "STATION_UP";
    }
    if (t == EV_SET_URGENCY)
    {
        return "SET_URGENCY";
    }
    if (t == EV_ROBOT_OFFLINE)
    {
        return "ROBOT_OFFLINE";
    }
    if (t == EV_ROBOT_ONLINE)
    {
        return "ROBOT_ONLINE";
    }
    return "UNKNOWN";
}

// ---------------- getters ----------------

bool Parser::isLoaded() const
{
    return loaded;
}
const std::string& Parser::getError() const
{
    return error;
}
const std::string& Parser::getScenarioName() const
{
    return scenarioName;
}
const SimParams& Parser::getParams() const
{
    return params;
}

LocationRec* Parser::getLocations()
{
    return locations;
}
int Parser::getLocationCount() const
{
    return locationCount;
}
ConnectionRec* Parser::getConnections()
{
    return connections;
}
int Parser::getConnectionCount() const
{
    return connectionCount;
}
Robot* Parser::getRobots()
{
    return robots;
}
int Parser::getRobotCount() const
{
    return robotCount;
}
RescueRequest* Parser::getRequests()
{
    return requests;
}
int Parser::getRequestCount() const
{
    return requestCount;
}
EventRec* Parser::getEvents()
{
    return events;
}
int Parser::getEventCount() const
{
    return eventCount;
}

LinkedList* Parser::getEventList()
{
    return eventList;
}
LinkedList* Parser::getRequestList()
{
    return requestList;
}

int Parser::locationIndex(int id)
{
    return locIndex ? locIndex->get(id) : INVALID;
}
int Parser::connectionIndex(int id)
{
    return connIndex ? connIndex->get(id) : INVALID;
}
int Parser::robotIndex(int id)
{
    return robIndex ? robIndex->get(id) : INVALID;
}
int Parser::requestIndex(int id)
{
    return reqIndex ? reqIndex->get(id) : INVALID;
}