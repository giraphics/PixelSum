#pragma once

#include <map>

#include "VirtualMemory.h"
#include "VirtualMemoryPoolConfig.h"

// Virtual memory namespace
namespace VM
{

class VirtualMemoryPool : public VirtualMemory
{
public:
    explicit VirtualMemoryPool(std::vector<VMPoolConfig> p_VmPoolConfig);
    virtual ~VirtualMemoryPool();

    void* AllocateVirtualMemory(size_t p_Size);
    void FreeVirtualMemory(void* p_Pointer);

    void PrintStats();

    size_t InUsedMemory();
    size_t TotalMemoryCapacity();

private:
    std::map<size_t, size_t> m_PoolSizeLookupTableIdx;
    VMPoolConfig*            m_PoolConfig;
    MemoryPool*              m_VMPool;
    uint8_t                  m_PoolCount = 0;

    MemRequirementSortObject m_VmPoolConfigSortObject;
};

} //namespace VM (Virtual memory namespace)
