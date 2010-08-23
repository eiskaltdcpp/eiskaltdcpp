/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef POOLALLOC_H
#define POOLALLOC_H

/*
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <algorithm>
#include <exception>
#include <string>

using namespace std;

class MemoryManager;

class MemoryManager {
    class MemoryException: public exception{
    public:
        MemoryException(const string &err): _err(err){}
        virtual ~MemoryException(){}

        const char *what() const {return _err.c_str(); }

    private:
        string _err;
    };

public:

    MemoryManager() {
    }

    virtual ~MemoryManager() {
        freeHash(free_blocks);
        freeHash(alloc_blocks);
    }

    void *get(const unsigned getSize){
        vector< void* > *free_vec  = getPool(free_blocks,  getSize);
        vector< void* > *alloc_vec = getPool(alloc_blocks, getSize);

        if (free_vec->empty())
            grow(*free_vec, getSize);

        void *chunk = free_vec->front();

        free_vec->erase(free_vec->begin());

        alloc_vec->push_back(chunk);

        return chunk;
    }

    void put(const unsigned size, void *chunk){
        if (!(size && chunk))
            throw MemoryManager::MemoryException("Cannot put invalid data");

        vector< void* > *alloc_vec = getPool(alloc_blocks, size);
        vector< void* >::iterator it = find(alloc_vec->begin(), alloc_vec->end(), chunk);

        if (it == alloc_vec->end())
            throw MemoryManager::MemoryException("Cannot put non-existing data");

        vector< void* > *free_vec  = getPool(free_blocks,  size);

        free_vec->push_back(chunk);
        alloc_vec->erase(it);

        if (alloc_vec->empty()){
            map<int, vector< void* > * >::iterator it = alloc_blocks.find(size);

            alloc_blocks.erase(it);

            delete alloc_vec;
        }
    }

    void printStat(){
        map<int, vector< void* > * >::iterator it = alloc_blocks.begin();

        printf("Allocated:\n\n"
               "\tChunk size\t\tPool size\n\n");

        for (; it != alloc_blocks.end(); ++it){
            int size = (*it).first;
            vector< void* > *vec = (*it).second;

            printf("\t%i\t\t\t%i\n", size, vec->size());
        }

        it = free_blocks.begin();

        printf("Free:\n\n"
               "\tChunk size\t\tPool size\n\n");

        for (; it != free_blocks.end(); ++it){
            int size = (*it).first;
            vector< void* > *vec = (*it).second;

            printf("\t%i\t\t\t%i\n", size, vec->size());
        }
    }

private:

    void grow(vector< void* > &vec, unsigned _size){
        if (_size == 0)
            throw MemoryManager::MemoryException("Cannot allocate pool with 0 bytes");

        for (int i = 0; i < 512; i++){
            void *chunk = malloc(_size);

            if (!chunk)
                throw MemoryManager::MemoryException("Not enough memory");

            vec.push_back(chunk);
        }
    }

    void freePool(vector< void* > &pool){
        vector< void* >::iterator it = pool.begin();

        for (; it != pool.end(); ++it)
            free(*it);

        pool.clear();
    }

    void freeHash(map<int, vector< void* > * > &blocks){
        map<int, vector< void* > * >::iterator it = blocks.begin();

        for (; it != blocks.end(); ++it){
            vector< void* > *vec = (*it).second;

            freePool(*vec);

            delete vec;
        }

        blocks.clear();
    }

    vector< void* > *getPool(map<int, vector< void* > * > &hash, unsigned size){
        if (size == 0)
            throw MemoryManager::MemoryException("Cannot allocate pool with 0 bytes");

        map<int, vector< void* > * >::iterator it = hash.find(size);

        if (it != hash.end()){
            vector< void* > *vec = (*it).second;

            return vec;
        }
        else {
            vector< void* > *vec = new vector< void* >();

            hash[size] = vec;

            return vec;
        }
    }

    map<int, vector< void* > * > free_blocks;
    map<int, vector< void* > * > alloc_blocks;
};
*/

#endif // POOLALLOC_H
