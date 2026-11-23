#pragma once
#include "Types.h"
#include "Queue.h"

const int LOW_BATTERY_PERCENT = 25;

class Robot
{
private:
    int id;
    int place;
    int skill;
    int maxBattery;
    int battery;
    int state;
    int requestId;
    Queue route;
    int costLeft;
    int nextPlace;
    int busySteps;

public:
    Robot();
    void init(int id, int place, int skill, int maxBattery, int battery);

    int getId() const;
    int getPlace() const;
    int getSkill() const;
    int getMaxBattery() const;
    int getBattery() const;
    int getState() const;
    int getRequestId() const;
    int getCostLeft() const;
    int getNextPlace() const;
    int getBusySteps() const;
    Queue& getRoute();

    bool changeState(int newState);

    int drainMove(int cost, int perUnit);
    int drainWork(int perStep);
    int charge(int amount);

    bool needsCharge() const;
    bool canServe(int category) const;
    bool isFree() const;
    bool goingToCharger() const;

    void setPlace(int p);
    void setRequest(int reqId);
    void clearRequest();
    void setCostLeft(int c);
    void setNextPlace(int p);
    void addBusyStep();
};
