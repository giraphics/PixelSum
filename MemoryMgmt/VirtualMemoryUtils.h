#pragma once

#define GFX_MEM_1KB             1024
#define GFX_MEM_KB(capacity)    (capacity                                     * GFX_MEM_1KB)
#define GFX_MEM_MB(capacity)    (GFX_MEM_KB(capacity)                         * GFX_MEM_1KB)
#define GFX_MEM_GB(capacity)    ((GFX_MEM_MB(static_cast<uint64_t>(capacity)) * GFX_MEM_1KB))

#define GFX_CONVERT_TO_POINTER(value)                       ((void*)(value))
#define GFX_ADVANCE_POINTER_BY_OFFSET(pointer, offset)      ((void*)((uintptr_t)GFX_CONVERT_TO_POINTER(pointer) + offset))
#define GFX_POINTER_TO_UINT(pointer)                        ((uintptr_t)(pointer))
#define GFX_ALIGN_POINTER(pointer, base)                    GFX_CONVERT_TO_POINTER(((GFX_POINTER_TO_UINT(pointer))+((base)-1L)) & ~((base)-1L))
#define GFX_IS_ALIGNED(pointer, alignment)                  (((uintptr_t)pointer & (uintptr_t)(alignment - 1L)) == 0)
#define GFX_IS_VALID_RANGE(value, min_value, max_value)     (value >= min_value && value <= max_value)
#define GFX_NEXT_POWER_OF_2(v)                              { (v)--; (v) |= (v) >> 1; (v) |= (v) >> 2; (v) |= (v) >> 4; (v) |= (v) >> 8; (v) |= (v) >> 16; (v)++; }

#if (defined(UINTPTR_MAX) && UINTPTR_MAX == UINT32_MAX)
    #define GFX_SYSTEM_DEFAULT_ALIGNMENT 4
#elif defined(UINTPTR_MAX) && UINTPTR_MAX == UINT64_MAX
    #define GFX_SYSTEM_DEFAULT_ALIGNMENT 8
#else
#error "Invalid Darwin platform architecture"
#endif
