#include "queue.h"

Queue::Queue() 
{
    head = nullptr;
    tail = nullptr;
    count = 0;
}

Queue::Queue(const Queue& other) 
{
    head = nullptr;
    tail = nullptr;
    count = 0;
    copyFrom(other);
}

Queue& Queue::operator=(const Queue& other) 
{
    if (this != &other) 
    {
        clear();
        copyFrom(other);
    }
    return *this;
}

Queue::~Queue() 
{
    clear();
}

void Queue::copyFrom(const Queue& other) 
{
    Node* cur = other.head;
    while (cur != nullptr) 
    {
        enqueue(cur->value);
        cur = cur->next;
    }
}

void Queue::enqueue(int value) 
{
    Node* n = new Node;
    n->value = value;
    n->next = nullptr;

    if (tail == nullptr) 
    {
        head = n;
        tail = n;
    } 
    else 
    {
        tail->next = n;
        tail = n;
    }
    count++;
}

void Queue::pushFront(int value) 
{
    Node* n = new Node;
    n->value = value;
    n->next = head;
    head = n;

    if (tail == nullptr) 
    {
        tail = n;
    }
    count++;
}

int Queue::dequeue() 
{
    if (head == nullptr) 
    {
        return -1;
    }

    Node* old = head;
    int value = old->value;
    head = head->next;

    if (head == nullptr) 
    {
        tail = nullptr;
    }

    delete old;
    count--;
    return value;
}

int Queue::front() const 
{
    if (head == nullptr) 
    {
        return -1;
    }
    return head->value;
}

bool Queue::isEmpty() const 
{
    return count == 0;
}

int Queue::size() const 
{
    return count;
}

void Queue::clear() 
{
    while (head != nullptr) 
    {
        Node* old = head;
        head = head->next;
        delete old;
    }
    tail = nullptr;
    count = 0;
}