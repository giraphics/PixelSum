#include "VirtualMemory.h"

#include <assert.h>
#include <inttypes.h>
#include <mach/mach.h>

using namespace VM;

VirtualMemory::VirtualMemory()
{
}

VirtualMemory::~VirtualMemory()
{
}

bool VirtualMemory::VmFree(const VM::PageAllocation* allocationInfo)
{
    return (vm_deallocate(mach_task_self(),
                          (vm_address_t)allocationInfo->baseAddress,
                          (vm_size_t)allocationInfo->size) == KERN_SUCCESS);
}

size_t VirtualMemory::VmSize()
{
    vm_size_t size = 0;
    host_page_size(mach_task_self(), &size);
    return (size_t)size;
}

bool VirtualMemory::VmAllocate(size_t size, VM::PageAllocation* allocationInfo)
{
    vm_address_t address = 0;
    vm_size_t pageSize = round_page(size);
    if (vm_allocate(mach_task_self(), &address, pageSize, VM_FLAGS_ANYWHERE) != KERN_SUCCESS)
    {
        return false;
    }

    allocationInfo->baseAddress = reinterpret_cast<void*>(address);
    allocationInfo->size = pageSize;

    return true;
}
