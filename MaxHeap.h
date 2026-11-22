#pragma once
struct ReqItem{
    int id;
    int urgency;
    int reportTime;
};
class MaxHeap{
private:
    ReqItem* arr;
    int size;
    int capacity;
    int parent(int i){
        return (i-1)/2;
    }
    int left(int i){
        return 2*i+1;
    }
    int right(int i){
        return 2*i+2;
    }
    bool higher(ReqItem a,ReqItem b);
    void swap(int i,int j);
    void heapifyUp(int i);
    void heapifyDown(int i);
    void resize();
public:
    MaxHeap(int cap=16);
    MaxHeap(const MaxHeap& other);
    MaxHeap& operator=(const MaxHeap& other);
    ~MaxHeap();
    void insert(ReqItem item);
    void insert(int id,int urgency,int reportTime);
    ReqItem extractMax();
    ReqItem peek();
    bool isEmpty();
    int getSize();
};
