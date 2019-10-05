#pragma once

#include <stddef.h>
#include "VirtualMemoryPoolConfig.h"

namespace VM
{

class VirtualMemory
{
public:
    VirtualMemory();
    virtual ~VirtualMemory();

    bool VmAllocate(size_t size, VM::PageAllocation* allocationInfo);
    bool VmFree(const VM::PageAllocation* allocationInfo);
    size_t VmSize();
};

}
