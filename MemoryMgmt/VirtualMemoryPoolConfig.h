#pragma once

#include <vector>

#include "VirtualMemoryUtils.h"

namespace VM
{

struct PageAllocation
{
    void* baseAddress = nullptr;
    size_t size = 0;
};

struct FreeStack
{
    PageAllocation pageAlloc;

    void** addressPtr   = nullptr;

    size_t capacity     = 0;
    size_t count        = 0;
} ;

struct MemoryPage
{
    FreeStack freeStack;
    PageAllocation pageAlloc;

    void* baseAddress    = nullptr;
    void* currentAddress = nullptr;

    size_t elementSize   = 0;
    size_t usedByteSize  = 0;
};

struct MemoryPool
{
    MemoryPool(uint8_t p_MemoryPoolCount)
    {
        pools.resize(p_MemoryPoolCount);
    }

    std::vector<MemoryPage> pools;
};


struct VMPoolConfig
{
    size_t   poolSize;
    uint32_t poolCapacity;
};

struct MemRequirementSortObject
{
    bool operator() (VMPoolConfig i,VMPoolConfig j)
    {
        return (i.poolSize < j.poolSize);
    }
};

}

