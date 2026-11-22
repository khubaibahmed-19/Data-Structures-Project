#include "linklist.h"

LinkedList::LinkedList() 
{
    head = nullptr;
    tail = nullptr;
    count = 0;
}

LinkedList::LinkedList(const LinkedList& other) 
{
    head = nullptr;
    tail = nullptr;
    count = 0;
    copyFrom(other);
}

LinkedList& LinkedList::operator=(const LinkedList& other) 
{
    if (this != &other) 
    {
        clear();
        copyFrom(other);
    }
    return *this;
}

LinkedList::~LinkedList() 
{
    clear();
}

void LinkedList::copyFrom(const LinkedList& other) 
{
    Node* cur = other.head;
    while (cur != nullptr) 
    {
        insertSorted(cur->key, cur->value);
        cur = cur->next;
    }
}

void LinkedList::insertSorted(int key, int value) 
{
    Node* n = new Node;
    n->key = key;
    n->value = value;
    n->next = nullptr;

    if (head == nullptr || key >= tail->key) 
    {
        if (head == nullptr) 
        {
            head = n;
        } 
        else 
        {
            tail->next = n;
        }
        tail = n;
        count++;
        return;
    }

    if (key < head->key) 
    {
        n->next = head;
        head = n;
        count++;
        return;
    }

    Node* cur = head;
    while (cur->next != nullptr && cur->next->key <= key) 
    {
        cur = cur->next;
    }
    n->next = cur->next;
    cur->next = n;
    count++;
}

int LinkedList::peekKey() const 
{
    if (head == nullptr) 
    {
        return LIST_EMPTY;
    }
    return head->key;
}

int LinkedList::peekValue() const 
{
    if (head == nullptr) 
    {
        return LIST_EMPTY;
    }
    return head->value;
}

int LinkedList::popFront() 
{
    if (head == nullptr) 
    {
        return LIST_EMPTY;
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

bool LinkedList::isEmpty() const 
{
    return count == 0;
}

int LinkedList::size() const 
{
    return count;
}

void LinkedList::clear() 
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