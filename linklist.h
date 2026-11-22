#ifndef LINKLIST_H
#define LINKLIST_H

#define LIST_EMPTY -1

class LinkedList 
{
private:
    struct Node 
    {
        int key;
        int value;
        Node* next;
    };

    Node* head;
    Node* tail;
    int count;

    void copyFrom(const LinkedList& other);

public:
    LinkedList();
    LinkedList(const LinkedList& other);
    LinkedList& operator=(const LinkedList& other);
    ~LinkedList();

    void insertSorted(int key, int value);
    int peekKey() const;
    int peekValue() const;
    int popFront();
    bool isEmpty() const;
    int size() const;
    void clear();
};

#endif