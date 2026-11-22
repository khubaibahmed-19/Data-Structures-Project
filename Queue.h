#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_EMPTY -1

class Queue 
{
private:
    struct Node 
    {
        int value;
        Node* next;
    };

    Node* head;   
    Node* tail;   
    int count;    
    void copyFrom(const Queue& other);   

public:
    Queue();
    Queue(const Queue& other);              
    Queue& operator=(const Queue& other);   
    ~Queue();                               

    void enqueue(int value);   
    void pushFront(int value); 
    int  dequeue();            
    int  front() const;        
    bool isEmpty() const;      
    int  size() const;         
    void clear();              
};

#endif