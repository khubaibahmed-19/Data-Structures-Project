#pragma once
struct HeapItem {
    int vertex;
    int dist;
};

class MinHeap {
private:
    HeapItem* arr;
    int size;
    int capacity;

    int parent(int i) { return (i - 1) / 2; }
    int left(int i)   { return 2 * i + 1; }
    int right(int i)  { return 2 * i + 2; }
    void swap(int i, int j);
    void heapifyUp(int i);
    void heapifyDown(int i);
    void resize();

public:
    MinHeap(int cap = 16);
    ~MinHeap();

    void insert(int vertex, int dist);
    HeapItem extractMin();
    HeapItem peek();
    bool isEmpty();
    int getSize();
};

