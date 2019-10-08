#include "MemoryAllocator.h"

//#include "MemoryMgmt/VirtualMemoryPool.h"
#include "VirtualMemoryPool.h"

using namespace VM;

std::unique_ptr<MemoryAllocator> MemoryAllocator::m_Instance;
std::once_flag MemoryAllocator::m_OnceFlag;
std::unique_ptr<VM::VirtualMemoryPool> MemoryAllocator::m_MemoryPool;

MemoryAllocator& MemoryAllocator::GetInstance()
{
    // Demo configuration table, please build of your own as per your renderer need and configure it to UiGfxVirtualMemoryPool
    std::call_once(m_OnceFlag, [] {
        m_Instance.reset(new MemoryAllocator());
    });

    return *m_Instance.get();
}

MemoryAllocator::~MemoryAllocator()
{
}

void* MemoryAllocator::Allocate(size_t p_Size)
{
    return m_MemoryPool->AllocateVirtualMemory(p_Size);
}

void MemoryAllocator::Free(void* p_Pointer)
{
    m_MemoryPool->FreeVirtualMemory(p_Pointer);
}

bool MemoryAllocator::ConfigureMemory(std::vector<UserMemoryRequirementConfig> p_UserMemoryRequirement)
{
    if (m_IsMemoryConfigured) return true;

    const size_t userMemoryRequirementSize = p_UserMemoryRequirement.size();
    if (userMemoryRequirementSize == 0) return false;

    m_VirtualMemoryTable.resize(userMemoryRequirementSize);
    for (size_t i = 0; i < userMemoryRequirementSize; i++)
    {
        size_t poolSize = p_UserMemoryRequirement.at(i).memorySize;
        VM_NEXT_POWER_OF_2(poolSize);
        m_VirtualMemoryTable[i].poolSize = poolSize;
        m_VirtualMemoryTable[i].poolCapacity = poolSize * p_UserMemoryRequirement.at(i).memoryInstances;
    }

    m_MemoryPool = std::unique_ptr<VM::VirtualMemoryPool>(new VM::VirtualMemoryPool(m_VirtualMemoryTable));

    m_IsMemoryConfigured = true;

    return true;
}

size_t MemoryAllocator::InUsedMemory()
{
    return m_MemoryPool->InUsedMemory();
}

size_t MemoryAllocator::TotalMemoryCapacity()
{
    return m_MemoryPool->TotalMemoryCapacity();
}
