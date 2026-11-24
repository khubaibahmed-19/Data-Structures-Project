#pragma once
#include "Types.h"

const int URGENCY_MIN = 1;
const int URGENCY_MAX = 5;

class RescueRequest
{
private:
    int id;
    int reportTime;
    int place;
    int category;
    int urgency;
    int duration;
    int state;
    bool reported;
    int robotId;
    int workDone;
    int version;
    int startStep;
    int finishStep;

public:
    RescueRequest();
    void init(int id, int reportTime, int place, int category, int urgency, int duration);

    int getId() const;
    int getReportTime() const;
    int getPlace() const;
    int getCategory() const;
    int getUrgency() const;
    int getDuration() const;
    int getState() const;
    bool isReported() const;
    int getRobotId() const;
    int getWorkDone() const;
    int getVersion() const;
    int getStartStep() const;
    int getFinishStep() const;
    int responseTime() const;

    bool changeState(int newState);

    bool report();
    bool assign(int robot);
    bool startWork(int step);
    bool doWork();
    bool complete(int step);
    bool fail(int step);
    bool release();
    bool setUrgency(int u);

    bool isFinal() const;
};