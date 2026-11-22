#include"MaxHeap.h"
MaxHeap::MaxHeap(int cap){
    arr=new ReqItem [cap];
    size=0;
    capacity=cap;
}
MaxHeap::MaxHeap(const MaxHeap& other){
    capacity=other.capacity;
    size=other.size;
    arr=new ReqItem [capacity];
    for(int i=0;i<size;i++){
        arr[i]=other.arr[i];
    }
}
MaxHeap& MaxHeap::operator=(const MaxHeap& other){
    if(this!=&other){
        delete []arr;
        capacity=other.capacity;
        size=other.size;
        arr=new ReqItem [capacity];
        for(int i=0;i<size;i++){
            arr[i]=other.arr[i];
        }
    }
    return *this;
}
MaxHeap::~MaxHeap(){
    delete []arr;
}
bool MaxHeap::higher(ReqItem a,ReqItem b){
    if(a.urgency!=b.urgency){
        return a.urgency>b.urgency;
    }
    if(a.reportTime!=b.reportTime){
        return a.reportTime<b.reportTime;
    }
    return a.id<b.id;
}
void MaxHeap::swap(int i,int j){
    ReqItem temp=arr[i];
    arr[i]=arr[j];
    arr[j]=temp;
}
void MaxHeap::heapifyUp(int i){
    while(i>0){
        int p=parent(i);
        if(higher(arr[i],arr[p])){
            swap(i,p);
            i=p;
        }
        else{
            break;
        }
    }
}
void MaxHeap::heapifyDown(int i){
    int l=left(i);
    int r=right(i);
    int largest=i;
    if(l<size){
        if(higher(arr[l],arr[largest])){
            largest=l;
        }
    }
    if(r<size){
        if(higher(arr[r],arr[largest])){
            largest=r;
        }
    }
    if(largest!=i){
        swap(i,largest);
        heapifyDown(largest);
    }
}
void MaxHeap::resize(){
    ReqItem* temp=new ReqItem [capacity*2];
    for(int i=0;i<size;i++){
        temp[i]=arr[i];
    }
    delete []arr;
    arr=temp;
    capacity*=2;
}
void MaxHeap::insert(ReqItem item){
    if(size==capacity){
        resize();
    }
    arr[size]=item;
    heapifyUp(size);
    size++;
}
void MaxHeap::insert(int id,int urgency,int reportTime){
    ReqItem item;
    item.id=id;
    item.urgency=urgency;
    item.reportTime=reportTime;
    insert(item);
}
ReqItem MaxHeap::extractMax(){
    if(size==0){
        ReqItem empty;
        empty.id=-1;
        empty.urgency=-1;
        empty.reportTime=-1;
        return empty;
    }
    ReqItem max=arr[0];
    arr[0]=arr[size-1];
    size--;
    heapifyDown(0);
    return max;
}
ReqItem MaxHeap::peek(){
    if(size==0){
        ReqItem empty;
        empty.id=-1;
        empty.urgency=-1;
        empty.reportTime=-1;
        return empty;
    }
    return arr[0];
}
bool MaxHeap::isEmpty(){
    return (size==0);
}
int MaxHeap::getSize(){
    return size;
}
