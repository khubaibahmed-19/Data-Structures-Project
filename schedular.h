#pragma once
#include"envoirnment.h"
#include"MaxHeap.h"
#include"robot.h"
#include"rescue_request.h"
#include"logger.h"

class Scheduler{
private:
    Environment* env;
    Robot* robots;
    int robotCount;
    RescueRequest* requests;
    int requestCount;
    Logger* logger;
    int batteryPerCost;
    int batteryPerWork;

    MaxHeap pending;
    int* dist;
    int* prev;
    int* retry;

    bool isStale(ReqItem item);
    bool enoughBattery(int robotIdx,int travelCost,int duration);
    int pickRobot(int reqIdx,int& travelCost,int& candidates);

public:
    Scheduler(Environment* env,Robot* robots,int robotCount,RescueRequest* requests,int requestCount,Logger* logger,int batteryPerCost,int batteryPerWork);
    ~Scheduler();
    Scheduler(const Scheduler& other);
    Scheduler& operator=(const Scheduler& other);

    void addRequest(int reqIdx);
    int assign(int step);
    bool hasPending();
    int pendingCount();
}; 