#include"MinHeap.h"
MinHeap::MinHeap(int cap){
    arr=new HeapItem [cap];
    size=0;
    capacity=cap;
}
MinHeap::~MinHeap(){
    delete []arr;
}
void MinHeap::swap(int i, int j){
    HeapItem temp=arr[i];
    arr[i]=arr[j];
    arr[j]=temp;
}
void MinHeap::heapifyUp(int i){
    while(i>0){
        int p=parent(i);
        if(arr[i].dist<arr[p].dist){
            swap(i,p);
            i=p;
        }
        else{
            break;
        }
    }
}
void MinHeap::heapifyDown(int i){
    int l=left(i);
    int r=right(i);
    int smallest=i;
    if(l<size){
        if(arr[l].dist<arr[smallest].dist){
            smallest=l;
        }
    }
    if(r<size){
        if(arr[r].dist<arr[smallest].dist){
            smallest=r;
        }
    }
    if(smallest!=i){
        swap(i,smallest);
        heapifyDown(smallest);
    }
}
void MinHeap::resize(){
    if (size>= capacity){
        HeapItem * temp=new HeapItem[capacity*2];
        
        for (int i=0;i<capacity;i++){
            temp[i]=arr[i];
        }
        delete [] arr;
        arr=temp;
        capacity*=2;
    }
}
void MinHeap:: insert(int vertex,int dist){
    if(size==capacity){
        resize();
    }
    HeapItem  newnode;
    newnode.vertex=vertex;
    newnode.dist=dist;
    arr[size]=newnode;
    heapifyUp(size);
    size++;
}
HeapItem MinHeap::extractMin(){
    if(size==0){
        HeapItem empty;
        empty.vertex=-1;
        empty.dist=-1;
        return empty;
    }
    HeapItem min= arr[0];
    arr[0]=arr[size-1];
    size--;
    heapifyDown(0);
    return min;
}
HeapItem MinHeap::peek(){
    if(size==0){
        HeapItem empty;
        empty.vertex=-1;
        empty.dist=-1;
        return empty;
    }
    return arr[0];
}
bool MinHeap::isEmpty(){
    return (size==0);
}
int MinHeap::getSize(){
    return size;
}