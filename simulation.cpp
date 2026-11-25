#include"simulation.h"
#include<ctime>
Simulation::Simulation(Parser* p){
    parser=p;
    env=nullptr;
    scheduler=nullptr;
    robots=nullptr;
    robotCount=0;
    requests=nullptr;
    requestCount=0;
    events=nullptr;
    eventCount=0;
    step=0;
    started=false;
    finished=false;
    runMs=0;
}
Simulation::~Simulation(){
    delete scheduler;
    delete env;
    logger.close();
}
bool Simulation::buildEnvironment(){
    LocationRec* locs=parser->getLocations();
    ConnectionRec* conns=parser->getConnections();
    int nLoc=parser->getLocationCount();
    int nConn=parser->getConnectionCount();
    int maxLoc=0;
    for(int i=0;i<nLoc;i++){
        if(locs[i].id>maxLoc){
            maxLoc=locs[i].id;
        }
    }
    int maxConn=0;
    for(int i=0;i<nConn;i++){
        if(conns[i].id>maxConn){
            maxConn=conns[i].id;
        }
    }
    if(maxLoc==0){
        return false;
    }
    env=new Environment(maxLoc,maxConn);
    for(int i=0;i<nLoc;i++){
        env->setLocation(locs[i].id,locs[i].name,locs[i].type);
    }
    for(int i=0;i<nConn;i++){
        env->addConnection(conns[i].id,conns[i].a,conns[i].b,conns[i].cost,conns[i].status);
    }
    return true;
}
bool Simulation::init(bool echo){
    if(parser==nullptr||!parser->isLoaded()){
        return false;
    }
    delete scheduler;
    scheduler=nullptr;
    delete env;
    env=nullptr;
    params=parser->getParams();
    if(params.speed<1){
        params.speed=1;
    }
    robots=parser->getRobots();
    robotCount=parser->getRobotCount();
    requests=parser->getRequests();
    requestCount=parser->getRequestCount();
    events=parser->getEvents();
    eventCount=parser->getEventCount();
    eventQueue=*parser->getEventList();
    requestQueue=*parser->getRequestList();
    if(!buildEnvironment()){
        return false;
    }
    logger.reset();
    logger.open(parser->getScenarioName());
    logger.setEcho(echo);
    scheduler=new Scheduler(env,robots,robotCount,requests,requestCount,&logger,params.batteryPerCost,params.batteryPerWork);
    step=0;
    runMs=0;
    started=true;
    finished=false;
    logger.write(0,"START","scenario "+parser->getScenarioName()+", "+to_string(parser->getLocationCount())+" locations, "+to_string(parser->getConnectionCount())+" connections, "+to_string(robotCount)+" robots, "+to_string(requestCount)+" requests, "+to_string(eventCount)+" events, max_steps "+to_string(params.maxSteps));
    return true;
}
int Simulation::requestOf(int robotIdx){
    int id=robots[robotIdx].getRequestId();
    if(id==INVALID){
        return -1;
    }
    return parser->requestIndex(id);
}
void Simulation::processEvents(){
    while(!eventQueue.isEmpty()&&eventQueue.peekKey()<=step){
        int idx=eventQueue.popFront();
        EventRec e=events[idx];
        if(e.type==EV_BLOCK){
            env->blockConnection(e.target);
            logger.write(step,"EVENT","road "+to_string(e.target)+" blocked");
        }
        else if(e.type==EV_UNBLOCK){
            env->unblockConnection(e.target);
            logger.write(step,"EVENT","road "+to_string(e.target)+" reopened");
        }
        else if(e.type==EV_STATION_DOWN){
            env->setStationUp(e.target,false);
            logger.write(step,"EVENT","charging station "+to_string(e.target)+" down");
            for(int i=0;i<robotCount;i++){
                if(robots[i].getState()==RECHARGING&&robots[i].getPlace()==e.target){
                    robots[i].changeState(AVAILABLE);
                    logger.write(step,"ROBOT","robot "+to_string(robots[i].getId())+" stopped charging, station "+to_string(e.target)+" down, battery "+to_string(robots[i].getBattery()));
                }
            }
        }
        else if(e.type==EV_STATION_UP){
            env->setStationUp(e.target,true);
            logger.write(step,"EVENT","charging station "+to_string(e.target)+" up");
        }
        else if(e.type==EV_SET_URGENCY){
            int r=parser->requestIndex(e.target);
            if(r!=INVALID){
                int old=requests[r].getUrgency();
                if(requests[r].setUrgency(e.value)){
                    logger.write(step,"EVENT","request "+to_string(e.target)+" urgency "+to_string(old)+" -> "+to_string(e.value));
                    scheduler->addRequest(r);
                }
            }
        }
        else if(e.type==EV_ROBOT_OFFLINE){
            int b=parser->robotIndex(e.target);
            if(b!=INVALID){
                releaseRequest(b,"robot offline");
                robots[b].getRoute().clear();
                robots[b].setCostLeft(0);
                robots[b].changeState(UNAVAILABLE);
                logger.write(step,"EVENT","robot "+to_string(e.target)+" offline at location "+to_string(robots[b].getPlace()));
            }
        }
        else if(e.type==EV_ROBOT_ONLINE){
            int b=parser->robotIndex(e.target);
            if(b!=INVALID&&robots[b].getState()==UNAVAILABLE){
                robots[b].changeState(AVAILABLE);
                logger.write(step,"EVENT","robot "+to_string(e.target)+" back online at location "+to_string(robots[b].getPlace()));
            }
        }
    }
}
void Simulation::reportRequests(){
    while(!requestQueue.isEmpty()&&requestQueue.peekKey()<=step){
        int idx=requestQueue.popFront();
        if(requests[idx].report()){
            logger.write(step,"REQUEST","request "+to_string(requests[idx].getId())+" reported at location "+to_string(requests[idx].getPlace())+", "+categoryName(requests[idx].getCategory())+", urgency "+to_string(requests[idx].getUrgency()));
            scheduler->addRequest(idx);
        }
    }
}
void Simulation::releaseRequest(int robotIdx,const string& reason){
    int r=requestOf(robotIdx);
    if(r==-1){
        return;
    }
    if(requests[r].release()){
        logger.addRelease();
        logger.write(step,"RELEASE","request "+to_string(requests[r].getId())+" released by robot "+to_string(robots[robotIdx].getId())+" ("+reason+"), back to pending");
        scheduler->addRequest(r);
    }
    robots[robotIdx].clearRequest();
}
void Simulation::sendToCharger(int robotIdx){
    int cost=env->nearestCharger(robots[robotIdx].getPlace(),robots[robotIdx].getRoute());
    if(cost==-1){
        return;
    }
    robots[robotIdx].setCostLeft(0);
    if(cost==0){
        robots[robotIdx].changeState(RECHARGING);
        logger.write(step,"CHARGE","robot "+to_string(robots[robotIdx].getId())+" starts charging at location "+to_string(robots[robotIdx].getPlace())+", battery "+to_string(robots[robotIdx].getBattery()));
        return;
    }
    robots[robotIdx].changeState(TRAVELLING);
    logger.write(step,"CHARGE","robot "+to_string(robots[robotIdx].getId())+" battery low ("+to_string(robots[robotIdx].getBattery())+"), heading to charger, cost "+to_string(cost));
}
void Simulation::handleBattery(){
    for(int i=0;i<robotCount;i++){
        if(robots[i].getState()==RECHARGING){
            if(!env->isStationUp(robots[i].getPlace())){
                robots[i].changeState(AVAILABLE);
                continue;
            }
            robots[i].charge(params.rechargePerStep);
            if(robots[i].getBattery()>=robots[i].getMaxBattery()){
                robots[i].changeState(AVAILABLE);
                logger.write(step,"CHARGE","robot "+to_string(robots[i].getId())+" fully charged ("+to_string(robots[i].getBattery())+")");
            }
        }
    }
    for(int i=0;i<robotCount;i++){
        if(robots[i].isFree()&&(robots[i].needsCharge()||robots[i].getBattery()*2<robots[i].getMaxBattery())){
            sendToCharger(i);
        }
    }
}
void Simulation::reroute(int robotIdx){
    int r=requestOf(robotIdx);
    int cost;
    logger.addReroute();
    if(r!=-1){
        cost=env->shortestPath(robots[robotIdx].getPlace(),requests[r].getPlace(),robots[robotIdx].getRoute());
    }
    else{
        cost=env->nearestCharger(robots[robotIdx].getPlace(),robots[robotIdx].getRoute());
    }
    if(cost==-1){
        logger.write(step,"REROUTE","robot "+to_string(robots[robotIdx].getId())+" has no route from location "+to_string(robots[robotIdx].getPlace()));
        releaseRequest(robotIdx,"no route");
        robots[robotIdx].getRoute().clear();
        robots[robotIdx].changeState(AVAILABLE);
        return;
    }
    logger.write(step,"REROUTE","robot "+to_string(robots[robotIdx].getId())+" new route from location "+to_string(robots[robotIdx].getPlace())+", cost "+to_string(cost));
}
bool Simulation::startLeg(int robotIdx){
    int next=robots[robotIdx].getRoute().front();
    int c=env->roadCost(robots[robotIdx].getPlace(),next);
    if(c==INVALID){
        reroute(robotIdx);
        if(robots[robotIdx].getState()!=TRAVELLING||robots[robotIdx].getRoute().isEmpty()){
            return false;
        }
        next=robots[robotIdx].getRoute().front();
        c=env->roadCost(robots[robotIdx].getPlace(),next);
        if(c==INVALID){
            return false;
        }
    }
    robots[robotIdx].setNextPlace(next);
    robots[robotIdx].setCostLeft(c);
    return true;
}
void Simulation::arrive(int robotIdx){
    int r=requestOf(robotIdx);
    robots[robotIdx].setCostLeft(0);
    if(r!=-1){
        robots[robotIdx].changeState(PERFORMING);
        requests[r].startWork(step);
        logger.write(step,"ARRIVE","robot "+to_string(robots[robotIdx].getId())+" reached request "+to_string(requests[r].getId())+" at location "+to_string(robots[robotIdx].getPlace())+", starting work");
        return;
    }
    if(env->isStationUp(robots[robotIdx].getPlace())){
        robots[robotIdx].changeState(RECHARGING);
        logger.write(step,"CHARGE","robot "+to_string(robots[robotIdx].getId())+" reached charger "+to_string(robots[robotIdx].getPlace())+", battery "+to_string(robots[robotIdx].getBattery()));
    }
    else{
        robots[robotIdx].changeState(AVAILABLE);
    }
}
void Simulation::moveRobots(){
    for(int i=0;i<robotCount;i++){
        if(robots[i].getState()!=TRAVELLING){
            continue;
        }
        int budget=params.speed;
        while(budget>0&&robots[i].getState()==TRAVELLING){
            if(robots[i].getCostLeft()==0){
                if(robots[i].getRoute().isEmpty()){
                    arrive(i);
                    break;
                }
                if(!startLeg(i)){
                    break;
                }
            }
            if(robots[i].getBattery()<=0&&params.batteryPerCost>0){
                logger.write(step,"BATTERY","robot "+to_string(robots[i].getId())+" ran out of battery at location "+to_string(robots[i].getPlace()));
                releaseRequest(i,"out of battery");
                robots[i].getRoute().clear();
                robots[i].setCostLeft(0);
                robots[i].changeState(UNAVAILABLE);
                break;
            }
            int m=robots[i].getCostLeft();
            if(m>budget){
                m=budget;
            }
            int used=robots[i].drainMove(m,params.batteryPerCost);
            logger.addCost(m);
            logger.addBattery(used);
            robots[i].setCostLeft(robots[i].getCostLeft()-m);
            budget-=m;
            if(robots[i].getCostLeft()==0){
                robots[i].setPlace(robots[i].getNextPlace());
                robots[i].getRoute().dequeue();
                logger.write(step,"MOVE","robot "+to_string(robots[i].getId())+" at location "+to_string(robots[i].getPlace())+", battery "+to_string(robots[i].getBattery()));
                if(robots[i].getRoute().isEmpty()){
                    arrive(i);
                    break;
                }
            }
        }
    }
}
void Simulation::doWork(){
    for(int i=0;i<robotCount;i++){
        int r=requestOf(i);
        if(r!=-1&&(robots[i].getState()==TRAVELLING||robots[i].getState()==PERFORMING)){
            robots[i].addBusyStep();
        }
        if(robots[i].getState()!=PERFORMING||r==-1){
            continue;
        }
        int used=robots[i].drainWork(params.batteryPerWork);
        logger.addBattery(used);
        if(requests[r].doWork()){
            requests[r].complete(step);
            logger.addCompleted(requests[r].responseTime());
            logger.write(step,"COMPLETE","request "+to_string(requests[r].getId())+" completed by robot "+to_string(robots[i].getId())+", response time "+to_string(requests[r].responseTime()));
            robots[i].clearRequest();
            robots[i].changeState(AVAILABLE);
        }
    }
}
bool Simulation::allDone(){
    if(!requestQueue.isEmpty()){
        return false;
    }
    for(int i=0;i<requestCount;i++){
        if(!requests[i].isFinal()){
            return false;
        }
    }
    return true;
}
bool Simulation::stepOnce(){
    if(!started||finished){
        return false;
    }
    clock_t t0=clock();
    processEvents();
    reportRequests();
    handleBattery();
    scheduler->assign(step);
    moveRobots();
    doWork();
    runMs+=(double)(clock()-t0)*1000.0/CLOCKS_PER_SEC;
    step++;
    if(allDone()||step>params.maxSteps){
        finished=true;
        if(allDone()){
            logger.write(step,"END","all requests finished");
        }
        else{
            logger.write(step,"END","reached max_steps "+to_string(params.maxSteps));
        }
    }
    return true;
}
void Simulation::runAll(){
    while(stepOnce()){
    }
}
int Simulation::getStep() const{
    return step;
}
bool Simulation::isFinished() const{
    return finished;
}
Environment* Simulation::getEnvironment(){
    return env;
}
void Simulation::printRobots(ostream& out){
    out<<"Robots at step "<<step<<":\n";
    for(int i=0;i<robotCount;i++){
        out<<"  robot "<<robots[i].getId()<<"  "<<categoryName(robots[i].getSkill())<<"  at "<<robots[i].getPlace()<<"  battery "<<robots[i].getBattery()<<"/"<<robots[i].getMaxBattery()<<"  "<<robotStateName(robots[i].getState());
        if(robots[i].getRequestId()!=INVALID){
            out<<"  request "<<robots[i].getRequestId();
        }
        out<<"\n";
    }
}
void Simulation::printRequests(ostream& out){
    const int states[5]={PENDING,ASSIGNED,IN_PROGRESS,COMPLETED,FAILED};
    out<<"Requests at step "<<step<<":\n";
    for(int s=0;s<5;s++){
        out<<"  "<<requestStateName(states[s])<<":";
        int count=0;
        for(int i=0;i<requestCount;i++){
            if(requests[i].getState()!=states[s]){
                continue;
            }
            if(states[s]==PENDING&&!requests[i].isReported()){
                continue;
            }
            out<<" "<<requests[i].getId();
            count++;
        }
        if(count==0){
            out<<" none";
        }
        out<<"\n";
    }
    int notYet=0;
    for(int i=0;i<requestCount;i++){
        if(!requests[i].isReported()){
            notYet++;
        }
    }
    out<<"  not reported yet: "<<notYet<<"\n";
}
void Simulation::printEnvironment(ostream& out){
    out<<"Environment at step "<<step<<":\n";
    LocationRec* locs=parser->getLocations();
    for(int i=0;i<parser->getLocationCount();i++){
        int id=locs[i].id;
        out<<"  location "<<id<<" "<<env->getLocationName(id)<<" "<<Parser::locationTypeName(env->getLocationType(id));
        if(env->getLocationType(id)==CHARGING){
            out<<(env->isStationUp(id)?" (up)":" (down)");
        }
        out<<"\n";
    }
    ConnectionRec* conns=parser->getConnections();
    for(int i=0;i<parser->getConnectionCount();i++){
        int id=conns[i].id;
        out<<"  road "<<id<<": "<<env->getConnectionA(id)<<" - "<<env->getConnectionB(id)<<"  cost "<<env->getConnectionCost(id)<<"  "<<(env->getConnectionStatus(id)==OPEN?"OPEN":"BLOCKED")<<"\n";
    }
}
void Simulation::printStats(ostream& out){
    logger.printStats(out,robots,robotCount,requestCount,step,runMs);
}
bool Simulation::saveStats(string& pathOut){
    return logger.writeStatsFile(robots,robotCount,requestCount,step,runMs,pathOut);
}