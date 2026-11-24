#include "rescue_request.h"

RescueRequest::RescueRequest()
{
    id = INVALID;
    reportTime = 0;
    place = INVALID;
    category = INVALID;
    urgency = URGENCY_MIN;
    duration = 1;
    state = PENDING;
    reported = false;
    robotId = INVALID;
    workDone = 0;
    version = 0;
    startStep = -1;
    finishStep = -1;
}

void RescueRequest::init(int newId, int newTime, int newPlace, int newCategory, int newUrgency, int newDuration)
{
    id = newId;
    reportTime = newTime;
    place = newPlace;
    category = newCategory;
    urgency = newUrgency;
    duration = newDuration;
    state = PENDING;
    reported = false;
    robotId = INVALID;
    workDone = 0;
    version = 0;
    startStep = -1;
    finishStep = -1;
}

int RescueRequest::getId() const
{
    return id;
}

int RescueRequest::getReportTime() const
{
    return reportTime;
}

int RescueRequest::getPlace() const
{
    return place;
}

int RescueRequest::getCategory() const
{
    return category;
}

int RescueRequest::getUrgency() const
{
    return urgency;
}

int RescueRequest::getDuration() const
{
    return duration;
}

int RescueRequest::getState() const
{
    return state;
}

bool RescueRequest::isReported() const
{
    return reported;
}

int RescueRequest::getRobotId() const
{
    return robotId;
}

int RescueRequest::getWorkDone() const
{
    return workDone;
}

int RescueRequest::getVersion() const
{
    return version;
}

int RescueRequest::getStartStep() const
{
    return startStep;
}

int RescueRequest::getFinishStep() const
{
    return finishStep;
}

int RescueRequest::responseTime() const
{
    if (state != COMPLETED)
    {
        return -1;
    }
    return finishStep - reportTime;
}

bool RescueRequest::changeState(int newState)
{
    bool ok = false;
    if (state == PENDING)
    {
        ok = (newState == ASSIGNED || newState == FAILED);
    }
    else if (state == ASSIGNED)
    {
        ok = (newState == IN_PROGRESS || newState == PENDING);
    }
    else if (state == IN_PROGRESS)
    {
        ok = (newState == COMPLETED || newState == PENDING);
    }

    if (ok)
    {
        state = newState;
    }
    return ok;
}

bool RescueRequest::report()
{
    if (reported)
    {
        return false;
    }
    reported = true;
    return true;
}

bool RescueRequest::assign(int robot)
{
    if (!reported || robot == INVALID || robotId != INVALID)
    {
        return false;
    }
    if (!changeState(ASSIGNED))
    {
        return false;
    }
    robotId = robot;
    return true;
}

bool RescueRequest::startWork(int step)
{
    if (!changeState(IN_PROGRESS))
    {
        return false;
    }
    startStep = step;
    workDone = 0;
    return true;
}

bool RescueRequest::doWork()
{
    if (state != IN_PROGRESS)
    {
        return false;
    }
    if (workDone < duration)
    {
        workDone++;
    }
    return workDone >= duration;
}

bool RescueRequest::complete(int step)
{
    if (workDone < duration)
    {
        return false;
    }
    if (!changeState(COMPLETED))
    {
        return false;
    }
    finishStep = step;
    return true;
}

bool RescueRequest::fail(int step)
{
    if (!changeState(FAILED))
    {
        return false;
    }
    finishStep = step;
    return true;
}

bool RescueRequest::release()
{
    if (state != ASSIGNED && state != IN_PROGRESS)
    {
        return false;
    }
    changeState(PENDING);
    robotId = INVALID;
    workDone = 0;
    startStep = -1;
    version++;
    return true;
}

bool RescueRequest::setUrgency(int u)
{
    if (u < URGENCY_MIN || u > URGENCY_MAX)
    {
        return false;
    }
    if (isFinal())
    {
        return false;
    }
    urgency = u;
    version++;
    return true;
}

bool RescueRequest::isFinal() const
{
    return state == COMPLETED || state == FAILED;
}