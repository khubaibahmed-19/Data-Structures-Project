#pragma once
#include "Types.h"
#include "hash_table.h"
#include "linklist.h"
#include "rescue_request.h"
#include "robot.h"
#include <fstream>
#include <string>

// One row from each section of the scenario file.
// All references are stored as IDs, exactly as written in the file.
struct LocationRec
{
    int id;
    std::string name;
    int type; // JUNCTION, BUILDING, HOSPITAL, DEPOT, CHARGING or COMMAND
    bool stationUp; // only used for CHARGING locations
};

struct ConnectionRec
{
    int id;
    int a, b; // location IDs
    int aIdx, bIdx; // array positions of a and b
    int cost;
    int status; // OPEN or BLOCKED
};

struct EventRec
{
    int time;
    int type; // EV_BLOCK ... EV_ROBOT_ONLINE
    int target; // connection, location, request or robot ID
    int value; // new urgency for EV_SET_URGENCY, otherwise 0
};

struct SimParams
{
    int maxSteps;
    int speed;
    int batteryPerCost;
    int batteryPerWork;
    int rechargePerStep;
};

class Parser
{
  private:
    std::string scenarioName;
    std::string error;
    bool loaded;

    SimParams params;
    bool paramSet[5];

    LocationRec* locations;
    int locationCount;
    ConnectionRec* connections;
    int connectionCount;
    Robot* robots;
    int robotCount;
    RescueRequest* requests;
    int requestCount;
    EventRec* events;
    int eventCount;

    int* requestLines;
    int* eventLines;

    HashTable* locIndex;
    HashTable* connIndex;
    HashTable* robIndex;
    HashTable* reqIndex;

    LinkedList* eventList; // key = time,        value = event position
    LinkedList* requestList; // key = report time, value = request position

    bool loadFile(const std::string& filename);
    bool nextLine(std::ifstream& in, int& lineNo, std::string tok[], int& n);
    bool fail(int lineNo, const std::string& reason);
    bool readCount(const std::string tok[], int n, int lineNo, int minCount, int& count);
    bool needsRecord(std::ifstream& in, int& lineNo, std::string tok[], int& n, const std::string& section,
                     int done, int count);

    bool readParam(const std::string tok[], int n, int lineNo);
    bool readLocations(std::ifstream& in, int& lineNo, int count);
    bool readConnections(std::ifstream& in, int& lineNo, int count);
    bool readRobots(std::ifstream& in, int& lineNo, int count);
    bool readRequests(std::ifstream& in, int& lineNo, int count);
    bool readEvents(std::ifstream& in, int& lineNo, int count);
    bool finalChecks(int endLine);

    static bool toInt(const std::string& s, int& out);
    static bool isKeyword(const std::string& s);
    static int locationType(const std::string& s);
    static int skillCode(const std::string& s);
    static int eventType(const std::string& s);

  public:
    Parser();
    ~Parser();
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    bool load(const std::string& filename);
    void clear();

    bool isLoaded() const;
    const std::string& getError() const;
    const std::string& getScenarioName() const;
    const SimParams& getParams() const;

    LocationRec* getLocations();
    int getLocationCount() const;
    ConnectionRec* getConnections();
    int getConnectionCount() const;
    Robot* getRobots();
    int getRobotCount() const;
    RescueRequest* getRequests();
    int getRequestCount() const;
    EventRec* getEvents();
    int getEventCount() const;

    LinkedList* getEventList();
    LinkedList* getRequestList();

    // ID -> array position, or INVALID if the ID does not exist
    int locationIndex(int id);
    int connectionIndex(int id);
    int robotIndex(int id);
    int requestIndex(int id);

    static const char* locationTypeName(int t);
    static const char* eventTypeName(int t);
};