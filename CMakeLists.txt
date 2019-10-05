cmake_minimum_required (VERSION 3.0)

set (CMAKE_CXX_STANDARD 11)

project(PixelSum)

set(${PROJECT_NAME}_EXEC_SOURCE
    MemoryMgmt/VirtualMemory.cpp
    MemoryMgmt/VirtualMemoryPool.cpp
    MemoryMgmt/MemoryAllocator.cpp

    PixelSum/PixelSum.cpp
    PixelSum/PixelSumTheirs.cpp
    PixelSum/PixelSumNaive.cpp

	main.cpp
)

set(${PROJECT_NAME}_EXEC_HEADERS
    MemoryMgmt/VirtualMemory.h
    MemoryMgmt/VirtualMemoryPool.h
    MemoryMgmt/VirtualMemoryPoolConfig.h
    MemoryMgmt/VirtualMemoryUtils.h
    MemoryMgmt/MemoryAllocator.h

    PixelSum/HelperClasses/ScopedTimer.h
    PixelSum/HelperClasses/PixelBuffer.h
    PixelSum/HelperClasses/LogMacros.h

    PixelSum/PixelSum.h
    PixelSum/PixelSumTheirs.h
    PixelSum/PixelSumNaive.h

    TestCases/PixelSumTestCases.h
    TestCases/TestCaseHelper.h
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_EXEC_SOURCE} ${${PROJECT_NAME}_EXEC_HEADERS})

target_include_directories(${PROJECT_NAME}
    PUBLIC 
    MemoryMgmt
    PixelSum
    PixelSum/HelperClasses
)
