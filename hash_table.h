#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define HASH_MISSING -1

class HashTable 
{
private:
    struct Node 
    {
        int key;
        int value;
        Node* next;
    };

    Node** slots;
    int tableSize;
    int count;

    int indexOf(int key) const;
    void copyFrom(const HashTable& other);
    void freeAll();

public:
    explicit HashTable(int expectedCount);
    HashTable(const HashTable& other);
    HashTable& operator=(const HashTable& other);
    ~HashTable();

    bool put(int key, int value);
    int get(int key) const;
    bool contains(int key) const;
    int size() const;
    int capacity() const;
};

#endif