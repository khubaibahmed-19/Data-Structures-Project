#include "robot.h"

Robot::Robot()
{
    id = INVALID;
    place = INVALID;
    skill = GENERAL;
    maxBattery = 0;
    battery = 0;
    state = AVAILABLE;
    requestId = INVALID;
    costLeft = 0;
    nextPlace = INVALID;
    busySteps = 0;
}

void Robot::init(int newId, int newPlace, int newSkill, int newMax, int newBattery)
{
    id = newId;
    place = newPlace;
    skill = newSkill;
    maxBattery = newMax;
    battery = newBattery;

    if (battery < 0)
    {
        battery = 0;
    }
    if (battery > maxBattery)
    {
        battery = maxBattery;
    }

    state = AVAILABLE;
    requestId = INVALID;
    route.clear();
    costLeft = 0;
    nextPlace = INVALID;
    busySteps = 0;
}

int Robot::getId() const
{
    return id;
}

int Robot::getPlace() const
{
    return place;
}

int Robot::getSkill() const
{
    return skill;
}

int Robot::getMaxBattery() const
{
    return maxBattery;
}

int Robot::getBattery() const
{
    return battery;
}

int Robot::getState() const
{
    return state;
}

int Robot::getRequestId() const
{
    return requestId;
}

int Robot::getCostLeft() const
{
    return costLeft;
}

int Robot::getNextPlace() const
{
    return nextPlace;
}

int Robot::getBusySteps() const
{
    return busySteps;
}

Queue& Robot::getRoute()
{
    return route;
}

bool Robot::changeState(int newState)
{
    if (newState < AVAILABLE || newState > UNAVAILABLE)
    {
        return false;
    }

    bool ok = false;
    if (newState == UNAVAILABLE)
    {
        ok = true;
    }
    else if (state == AVAILABLE)
    {
        ok = (newState == TRAVELLING || newState == RECHARGING);
    }
    else if (state == TRAVELLING)
    {
        ok = (newState == PERFORMING || newState == AVAILABLE || newState == RECHARGING);
    }
    else if (state == PERFORMING)
    {
        ok = (newState == AVAILABLE);
    }
    else if (state == RECHARGING)
    {
        ok = (newState == AVAILABLE || newState == TRAVELLING);
    }
    else if (state == UNAVAILABLE)
    {
        ok = (newState == AVAILABLE);
    }

    if (ok)
    {
        state = newState;
    }
    return ok;
}

int Robot::drainMove(int cost, int perUnit)
{
    if (cost <= 0 || perUnit <= 0)
    {
        return 0;
    }

    int amount = cost * perUnit;
    if (amount > battery)
    {
        amount = battery;
    }
    battery -= amount;
    return amount;
}

int Robot::drainWork(int perStep)
{
    if (perStep <= 0)
    {
        return 0;
    }

    int amount = perStep;
    if (amount > battery)
    {
        amount = battery;
    }
    battery -= amount;
    return amount;
}

int Robot::charge(int amount)
{
    if (amount <= 0)
    {
        return 0;
    }

    int room = maxBattery - battery;
    if (amount > room)
    {
        amount = room;
    }
    battery += amount;
    return amount;
}

bool Robot::needsCharge() const
{
    return battery * 100 < maxBattery * LOW_BATTERY_PERCENT;
}

bool Robot::canServe(int category) const
{
    return ::canServe(skill, category);
}

bool Robot::isFree() const
{
    return state == AVAILABLE && requestId == INVALID;
}

bool Robot::goingToCharger() const
{
    return state == TRAVELLING && requestId == INVALID;
}

void Robot::setPlace(int p)
{
    place = p;
}

void Robot::setRequest(int reqId)
{
    requestId = reqId;
}

void Robot::clearRequest()
{
    requestId = INVALID;
}

void Robot::setCostLeft(int c)
{
    if (c < 0)
    {
        costLeft = 0;
    }
    else
    {
        costLeft = c;
    }
}

void Robot::setNextPlace(int p)
{
    nextPlace = p;
}

void Robot::addBusyStep()
{
    busySteps++;
}
