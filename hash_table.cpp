#include "hash_table.h"

static bool isPrime(int n) 
{
    if (n < 2) 
    {
        return false;
    }
    for (int d = 2; d * d <= n; d++) 
    {
        if (n % d == 0) 
        {
            return false;
        }
    }
    return true;
}

static int nextPrimeAbove(int n) 
{
    int p;
    if (n < 10) 
    {
        p = 11;
    } 
    else 
    {
        p = n + 1;
    }

    while (!isPrime(p)) 
    {
        p++;
    }
    return p;
}

HashTable::HashTable(int expectedCount) 
{
    slots = nullptr;
    tableSize = 0;
    count = 0;

    if (expectedCount < 0) 
    {
        expectedCount = 0;
    }

    tableSize = nextPrimeAbove(expectedCount);
    slots = new Node*[tableSize];
    for (int i = 0; i < tableSize; i++) 
    {
        slots[i] = nullptr;
    }
}

HashTable::HashTable(const HashTable& other) 
{
    slots = nullptr;
    tableSize = 0;
    count = 0;
    copyFrom(other);
}

HashTable& HashTable::operator=(const HashTable& other) 
{
    if (this != &other) 
    {
        freeAll();
        copyFrom(other);
    }
    return *this;
}

HashTable::~HashTable() 
{
    freeAll();
}

int HashTable::indexOf(int key) const 
{
    int r = key % tableSize;
    if (r < 0) 
    {
        r += tableSize;
    }
    return r;
}

void HashTable::copyFrom(const HashTable& other) 
{
    tableSize = other.tableSize;
    count = 0;
    slots = new Node*[tableSize];

    for (int i = 0; i < tableSize; i++) 
    {
        slots[i] = nullptr;
    }

    for (int i = 0; i < other.tableSize; i++) 
    {
        Node* cur = other.slots[i];
        while (cur != nullptr) 
        {
            put(cur->key, cur->value);
            cur = cur->next;
        }
    }
}

void HashTable::freeAll() 
{
    if (slots != nullptr) 
    {
        for (int i = 0; i < tableSize; i++) 
        {
            Node* cur = slots[i];
            while (cur != nullptr) 
            {
                Node* old = cur;
                cur = cur->next;
                delete old;
            }
        }
        delete[] slots;
    }
    slots = nullptr;
    tableSize = 0;
    count = 0;
}

bool HashTable::put(int key, int value) 
{
    int i = indexOf(key);
    Node* cur = slots[i];
    while (cur != nullptr) 
    {
        if (cur->key == key) 
        {
            return false;
        }
        cur = cur->next;
    }

    Node* n = new Node;
    n->key = key;
    n->value = value;
    n->next = slots[i];
    slots[i] = n;
    count++;
    return true;
}

int HashTable::get(int key) const 
{
    int i = indexOf(key);
    Node* cur = slots[i];
    while (cur != nullptr) 
    {
        if (cur->key == key) 
        {
            return cur->value;
        }
        cur = cur->next;
    }
    return HASH_MISSING;
}

bool HashTable::contains(int key) const 
{
    int i = indexOf(key);
    Node* cur = slots[i];
    while (cur != nullptr) 
    {
        if (cur->key == key) 
        {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

int HashTable::size() const 
{
    return count;
}

int HashTable::capacity() const 
{
    return tableSize;
}