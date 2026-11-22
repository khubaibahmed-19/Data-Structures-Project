#include"Minheap.h"
MinHeap::MinHeap(int cap=16){
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
    arr[j]=arr[i];
}
void MinHeap::heapifyUp(int i){
    
}