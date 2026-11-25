#include"schedular.h"
Scheduler::Scheduler(Environment* e,Robot* r,int rCount,RescueRequest* req,int reqCount,Logger* log,int perCost,int perWork){
    env=e;
    robots=r;
    robotCount=rCount;
    requests=req;
    requestCount=reqCount;
    logger=log;
    batteryPerCost=perCost;
    batteryPerWork=perWork;
    int n=env->getNumLocations()+1;
    dist=new int[n];
    prev=new int[n];
    retry=new int[requestCount+1];
}
Scheduler::~Scheduler(){
    delete []dist;
    delete []prev;
    delete []retry;
}
Scheduler::Scheduler(const Scheduler& other):pending(other.pending){
    env=other.env;
    robots=other.robots;
    robotCount=other.robotCount;
    requests=other.requests;
    requestCount=other.requestCount;
    logger=other.logger;
    batteryPerCost=other.batteryPerCost;
    batteryPerWork=other.batteryPerWork;
    int n=env->getNumLocations()+1;
    dist=new int[n];
    prev=new int[n];
    retry=new int[requestCount+1];
}
Scheduler& Scheduler::operator=(const Scheduler& other){
    if(this!=&other){
        delete []dist;
        delete []prev;
        delete []retry;
        env=other.env;
        robots=other.robots;
        robotCount=other.robotCount;
        requests=other.requests;
        requestCount=other.requestCount;
        logger=other.logger;
        batteryPerCost=other.batteryPerCost;
        batteryPerWork=other.batteryPerWork;
        pending=other.pending;
        int n=env->getNumLocations()+1;
        dist=new int[n];
        prev=new int[n];
        retry=new int[requestCount+1];
    }
    return *this;
}
void Scheduler::addRequest(int reqIdx){
    if(reqIdx<0 || reqIdx>requestCount-1){
        return;
    }
    if(requests[reqIdx].getState()!=PENDING){
        return;
    }
    if(!requests[reqIdx].isReported()){
        return;
    }
    ReqItem item;
    item.id=requests[reqIdx].getId();
    item.index=reqIdx;
    item.reportTime=requests[reqIdx].getReportTime();
    item.urgency=requests[reqIdx].getUrgency();
    item.version=requests[reqIdx].getVersion();
    pending.insert(item);
}
bool Scheduler::isStale(ReqItem item){
    if(item.index<0||item.index>=requestCount){
        return true;
    }
    if(requests[item.index].getState()!=PENDING){
        return true;
    }
    if(item.version!=requests[item.index].getVersion()){
        return true;
    }
    return false;
}
bool Scheduler::enoughBattery(int robotIdx,int travelCost,int duration){
    int needed=travelCost*batteryPerCost+duration*batteryPerWork;
    return robots[robotIdx].getBattery()>=needed;
}
int Scheduler::pickRobot(int reqIdx,int& travelCost,int& candidates){
    env->dijkstra(requests[reqIdx].getPlace(),dist,prev);
    int best=-1;
    travelCost=-1;
    candidates=0;
    for(int i=0;i<robotCount;i++){
        if(!robots[i].isFree()){
            continue;
        }
        if(!robots[i].canServe(requests[reqIdx].getCategory())){
            continue;
        }
        if(!env->isValidLocation(robots[i].getPlace())){
            continue;
        }
        int d=dist[robots[i].getPlace()];
        if(d==-1){
            continue;
        }
        if(!enoughBattery(i,d,requests[reqIdx].getDuration())){
            continue;
        }
        candidates++;
        if(best==-1||d<travelCost||(d==travelCost&&robots[i].getId()<robots[best].getId())){
            best=i;
            travelCost=d;
        }
    }
    return best;
}
int Scheduler::assign(int step){
    int assigned=0;
    int retryCount=0;
    while(!pending.isEmpty()){
        ReqItem item=pending.extractMax();
        if(isStale(item)){
            continue;
        }
        int travelCost=0;
        int candidates=0;
        int best=pickRobot(item.index,travelCost,candidates);
        if(best==-1){
            retry[retryCount]=item.index;
            retryCount++;
            continue;
        }
        int cost=env->shortestPath(robots[best].getPlace(),requests[item.index].getPlace(),robots[best].getRoute());
        if(cost==-1){
            retry[retryCount]=item.index;
            retryCount++;
            continue;
        }
        if(candidates>1&&logger!=nullptr){
            logger->addConflict();
        }
        requests[item.index].assign(robots[best].getId());
        robots[best].setRequest(requests[item.index].getId());
        robots[best].changeState(TRAVELLING);
        if(logger!=nullptr){
            logger->write(step,"ASSIGN","request "+to_string(requests[item.index].getId())+" (urgency "+to_string(requests[item.index].getUrgency())+") -> robot "+to_string(robots[best].getId())+", travel cost "+to_string(cost)+", "+to_string(candidates)+" candidate(s)");
        }
        assigned++;
    }
    for(int k=0;k<retryCount;k++){
        addRequest(retry[k]);
    }
    return assigned;
}
bool Scheduler::hasPending(){
    return !pending.isEmpty();
}
int Scheduler::pendingCount(){
    return pending.getSize();
}