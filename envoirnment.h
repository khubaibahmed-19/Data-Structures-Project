#pragma once
#include<string>
#include"Types.h"
#include"MinHeap.h"
#include"linklist.h"

using namespace std;

struct Edge{
    int connId;
    int to;
    int cost;
    Edge* next;
};

class Environment{
private:
    int numLocations;
    int numConnections;

    string* locName;
    int* locType;
    bool* stationUp;
    Edge** adj;

    int* connA;
    int* connB;
    int* connCost;
    int* connStatus;

    void addEdge(int from,int to,int connId,int cost);

public:
    Environment(int nLoc,int nConn);
    ~Environment();

    void setLocation(int id,string name,int type);
    void addConnection(int id,int a,int b,int cost,int status);

    void blockConnection(int connId);
    void unblockConnection(int connId);
    void setStationUp(int locId,bool up);

    bool isOpen(int from,int to);
    bool isValidLocation(int id);
    int getLocationType(int id);
    bool isStationUp(int id);
    int getNumLocations();

    void dijkstra(int src,int* dist,int* prev);
    int shortestPath(int from,int to,LinkedList& path);
    int nearestCharger(int from,LinkedList& path);
};