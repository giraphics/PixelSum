#pragma once

#include <mutex>
#include <vector>

#include "VirtualMemoryPool.h"

namespace VM
{

class VirtualMemoryPool;

struct UserMemoryRequirementConfig
{
    size_t   memorySize;
    uint32_t memoryInstances;
};

class MemoryAllocator
{
public:
    // Singleton
    static MemoryAllocator& GetInstance();

    ~MemoryAllocator();

    void* Allocate(size_t p_Size);
    void Free(void* p_Pointer);

    bool ConfigureMemory(std::vector<UserMemoryRequirementConfig> p_UserMemoryRequirement);

    size_t InUsedMemory();
    size_t TotalMemoryCapacity();

    // Private Ctor
private:
    MemoryAllocator() = default;

    // Private mem-vars
private:
    static std::unique_ptr<MemoryAllocator> m_Instance;
    static std::once_flag m_OnceFlag;
    bool m_IsMemoryConfigured = false;

    static std::unique_ptr<VM::VirtualMemoryPool> m_MemoryPool;
    std::vector<VM::VMPoolConfig> m_VirtualMemoryTable;
};

}//namespace VM (Virtual memory namespace)
