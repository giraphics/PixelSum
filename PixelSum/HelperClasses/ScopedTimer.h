#pragma once

#include <chrono>

struct ScopedTimer
{
    ScopedTimer(std::function<void(int)> callback)
        : m_TimePoint0(std::chrono::high_resolution_clock::now())
        , m_CallBack(callback)
    {
    }

    ~ScopedTimer(void)
    {
        auto  timePoint1 = std::chrono::high_resolution_clock::now();

        m_CallBack(std::chrono::duration_cast<std::chrono::microseconds>(timePoint1 - m_TimePoint0).count());
    }

private:
    std::chrono::high_resolution_clock::time_point m_TimePoint0;
    std::function<void(long)> m_CallBack;
};

// Default class for measuring performance, you can customise or add new class as per your need
struct DefaultResults
{
    void operator()(long p_MicroSeconds)
    {
        std::cout << "Performance:      : " << p_MicroSeconds << " µs"<< std::endl;
    }
};
