#pragma once
#include<string>
#include<ostream>
#include"parser.h"
#include"envoirnment.h"
#include"schedular.h"
#include"logger.h"
#include"linklist.h"

using namespace std;

class Simulation{
private:
    Parser* parser;
    Environment* env;
    Scheduler* scheduler;
    Logger logger;

    SimParams params;
    Robot* robots;
    int robotCount;
    RescueRequest* requests;
    int requestCount;
    EventRec* events;
    int eventCount;

    LinkedList eventQueue;
    LinkedList requestQueue;

    int step;
    bool started;
    bool finished;
    double runMs;

    bool buildEnvironment();
    void processEvents();
    void reportRequests();
    void moveRobots();
    void doWork();
    void handleBattery();

    bool startLeg(int robotIdx);
    void reroute(int robotIdx);
    void arrive(int robotIdx);
    void releaseRequest(int robotIdx,const string& reason);
    void sendToCharger(int robotIdx);
    int requestOf(int robotIdx);
    bool allDone();

public:
    Simulation(Parser* p);
    ~Simulation();
    Simulation(const Simulation&)=delete;
    Simulation& operator=(const Simulation&)=delete;

    bool init(bool echo);
    bool stepOnce();
    void runAll();

    int getStep() const;
    bool isFinished() const;
    Environment* getEnvironment();

    void printRobots(ostream& out);
    void printRequests(ostream& out);
    void printEnvironment(ostream& out);
    void printStats(ostream& out);
    bool saveStats(string& pathOut);
};