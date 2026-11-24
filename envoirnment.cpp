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
void Environment::dijkstra(int src,int* dist,int* prev){
    for(int i=1;i<=numLocations;i++){
        dist[i]=-1;
        prev[i]=-1;
    }
    MinHeap heap;
    heap.insert(src,0);
    while(!heap.isEmpty()){
        HeapItem cur=heap.extractMin();
        if(cur.dist>dist[cur.vertex]){
            continue;
        }
        Edge *now=adj[cur.vertex];
        while(now!=nullptr){
            if(connStatus[now->connId]==OPEN){
                int newDist=now->cost+dist[cur.vertex];
                if(dist[now->to]==-1||dist[now->to]>newDist){
                    dist[now->to]=newDist;
                    prev[now->to]=cur.vertex;
                }
            }
            now=now->next;
        }
    }
}
int Environment::shortestPath(int from,int to,Queue& path){
    int *dist=new int[numLocations+1];
    int *prev=new int[numLocations+1];
    dijkstra(from,dist,prev);
    path.clear();
    if(dist[to]==-1){
        delete []dist;
        delete []prev;
        return -1;
    }
    if(from==to){
        delete []dist;
        delete []prev;
        return 0;
    }
    int* temp=new int [numLocations];
    int trav=prev[to];
    int i=1;
    temp[0]=to;
    temp[i]=trav;
    while(trav!=from){
        trav=prev[temp[i]];
        i++;
        temp[i]=trav;
    }
    for(int k=i-1;k>=0;k--){
        path.enqueue(temp[k]);
    }
    int d=dist[to];
    delete []dist;
    delete []prev;
    delete []temp;
    return d;
}
int Environment::nearestCharger(int from,Queue& path){
    int *dist=new int[numLocations+1];
    int *prev=new int[numLocations+1];
    dijkstra(from,dist,prev);
    int best=-1;
    for(int x=1;x<=numLocations;x++){
        if(locType[x]!=CHARGING){
            continue;
        }
        if(!stationUp[x]){
            continue;
        }
        if(dist[x]==-1){
            continue;
        }
        if(best==-1||dist[x]<dist[best]){
            best=x;
        }
    }
    delete []dist;
    delete []prev;
    if(best==-1){
        path.clear();
        return -1;
    }
    return shortestPath(from,best,path);
}