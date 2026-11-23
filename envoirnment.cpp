#include"envoirnment.h"
Environment::Environment(int nLoc,int nConn){
    numLocations=nLoc;
    numConnections=nConn;
    locName=new string[numLocations+1];
    locType=new int[numLocations+1];
    stationUp=new bool[numLocations+1];
    adj= new Edge*[numLocations+1];
    connA=new int [numConnections+1];
    connB=new int [numConnections+1];
    connCost=new int[numConnections+1];
    connStatus=new int[numConnections+1];
}
void Environment::addEdge(int from,int to, int connId,int cost){
    Edge *e = new Edge;
    e->to=to;
    e->connId=connId;
    e->cost=cost;
    e->next=adj[from];
    adj[from]=e;
}
Environment::~Environment(){
        for(int i=0;i<=numLocations;i++){
        Edge* current=adj[i];
        while(current!=nullptr){
            Edge* next=current->next;
            delete current;
            current=next;
        }
    }

    delete[] adj;
    delete[] locName;
    delete[] locType;
    delete[] stationUp;
    delete[] connA;
    delete[] connB;
    delete[] connCost;
    delete[] connStatus;
}
void Environment::setLocation(int id,string name,int type){
    locName[id]=name;
    locType[id]=type;
}
void Environment::addConnection(int id,int a,int b,int cost,int status){
    if(id<1||id>numConnections||!isValidLocation(a)||!isValidLocation(b)){
        return;
    }
    addEdge(a,b,id,cost);
    addEdge(b,a,id,cost);
    connA[id]=a;
    connB[id]=b;
    connCost[id]=cost;
    connStatus[id]=status;
}
void Environment::blockConnection(int connId){
    connStatus[connId]=BLOCKED;
}
void Environment::unblockConnection(int connId){
    connStatus[connId]=OPEN;
}
void Environment::setStationUp(int LocId,bool up){
    locType[LocId]=CHARGING;
    stationUp[LocId]=up;
}
bool Environment::isOpen(int from,int to){
    Edge* e=adj[from];
    while(e!=nullptr){
        if(e->to==to&&connStatus[e->connId]==OPEN){
            return true;
        }
        e=e->next;
    }
    return false;
}
bool Environment::isValidLocation(int id){
    return id>=1&&id<=numLocations;
}
int Environment::getLocationType(int id){
    return locType[id];
}
bool Environment::isStationUp(int id){
    return stationUp[id]&&locType[id]==CHARGING;
}
int Environment::getNumLocations(){
    return numLocations;
}
