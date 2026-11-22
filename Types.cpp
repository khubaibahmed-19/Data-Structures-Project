#include"Types.h"
using namespace std;
int parseLocationType(string s){
    if(s=="JUNCTION"){
        return JUNCTION;
    }
    if(s=="BUILDING"){
        return BUILDING;
    }
    if(s=="HOSPITAL"){
        return HOSPITAL;
    }
    if(s=="DEPOT"){
        return DEPOT;
    }
    if(s=="CHARGING"){
        return CHARGING;
    }
    if(s=="COMMAND"){
        return COMMAND;
    }
    return INVALID;
}
int parseCapability(string s){
    if(s=="MEDICAL"){
        return MEDICAL;
    }
    if(s=="FIRE"){
        return FIRE;
    }
    if(s=="SUPPLY"){
        return SUPPLY;
    }
    if(s=="SEARCH"){
        return SEARCH;
    }
    if(s=="GENERAL"){
        return GENERAL;
    }
    return INVALID;
}
int parseCategory(string s){
    int c=parseCapability(s);
    if(c==GENERAL){
        return INVALID;
    }
    return c;
}
int parseStatus(string s){
    if(s=="OPEN"){
        return OPEN;
    }
    if(s=="BLOCKED"){
        return BLOCKED;
    }
    return INVALID;
}
int parseEvent(string s){
    if(s=="BLOCK"){
        return EV_BLOCK;
    }
    if(s=="UNBLOCK"){
        return EV_UNBLOCK;
    }
    if(s=="STATION_DOWN"){
        return EV_STATION_DOWN;
    }
    if(s=="STATION_UP"){
        return EV_STATION_UP;
    }
    if(s=="SET_URGENCY"){
        return EV_SET_URGENCY;
    }
    if(s=="ROBOT_OFFLINE"){
        return EV_ROBOT_OFFLINE;
    }
    if(s=="ROBOT_ONLINE"){
        return EV_ROBOT_ONLINE;
    }
    return INVALID;
}
bool canServe(int capability,int category){
    if(capability==GENERAL){
        return true;
    }
    return capability==category;
}
string robotStateName(int s){
    if(s==AVAILABLE){
        return "AVAILABLE";
    }
    if(s==TRAVELLING){
        return "TRAVELLING";
    }
    if(s==PERFORMING){
        return "PERFORMING";
    }
    if(s==RECHARGING){
        return "RECHARGING";
    }
    if(s==UNAVAILABLE){
        return "UNAVAILABLE";
    }
    return "INVALID";
}
string requestStateName(int s){
    if(s==PENDING){
        return "PENDING";
    }
    if(s==ASSIGNED){
        return "ASSIGNED";
    }
    if(s==IN_PROGRESS){
        return "IN_PROGRESS";
    }
    if(s==COMPLETED){
        return "COMPLETED";
    }
    if(s==FAILED){
        return "FAILED";
    }
    return "INVALID";
}
string categoryName(int c){
    if(c==MEDICAL){
        return "MEDICAL";
    }
    if(c==FIRE){
        return "FIRE";
    }
    if(c==SUPPLY){
        return "SUPPLY";
    }
    if(c==SEARCH){
        return "SEARCH";
    }
    if(c==GENERAL){
        return "GENERAL";
    }
    return "INVALID";
}
