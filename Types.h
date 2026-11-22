#ifndef TYPES_H
#define TYPES_H
#include<string>

// Invalid
const int INVALID=-1;

// Location types
const int JUNCTION=0;
const int BUILDING=1;
const int HOSPITAL=2;
const int DEPOT=3;
const int CHARGING=4;
const int COMMAND=5;

// Capabilities and categories
const int MEDICAL=0;
const int FIRE=1;
const int SUPPLY=2;
const int SEARCH=3;
const int GENERAL=4;

// Connection status
const int OPEN=0;
const int BLOCKED=1;

// Robot states
const int AVAILABLE=0;
const int TRAVELLING=1;
const int PERFORMING=2;
const int RECHARGING=3;
const int UNAVAILABLE=4;

// Request states
const int PENDING=0;
const int ASSIGNED=1;
const int IN_PROGRESS=2;
const int COMPLETED=3;
const int FAILED=4;

// Events
const int EV_BLOCK=0;
const int EV_UNBLOCK=1;
const int EV_STATION_DOWN=2;
const int EV_STATION_UP=3;
const int EV_SET_URGENCY=4;
const int EV_ROBOT_OFFLINE=5;
const int EV_ROBOT_ONLINE=6;

// Parse functions
int parseLocationType(std::string s);
int parseCapability(std::string s);
int parseCategory(std::string s);
int parseStatus(std::string s);
int parseEvent(std::string s);

// Capability check
bool canServe(int capability,int category);

// Name functions
std::string robotStateName(int s);
std::string requestStateName(int s);
std::string categoryName(int c);

#endif
