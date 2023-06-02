#pragma once

#include <chrono>
#include <cstdint>
#include <thread>
#include <x86intrin.h>

class Clock {
  public:
    void Restart() noexcept { _start = GetTime(); }

    int64_t GetElapsed() const noexcept { return GetTime() - _start; }

    int64_t GetElapsedNanoseconds() noexcept {
        return std::chrono::nanoseconds(int64_t(GetElapsed() / _frequency))
            .count();
    }

    static uint64_t GetTime() noexcept {
        uint32_t cpuId;

        return __rdtscp(&cpuId);
    }

  private:
    uint64_t _start = GetTime();
    static double _frequency;
};

double GetClockRate() {
    using namespace std::chrono;

    const auto start = high_resolution_clock::now();
    Clock clock;
    std::this_thread::sleep_for(100ms);
    int64_t cycleElapsed = clock.GetElapsed();
    double timeElapsed =
        duration_cast<nanoseconds>(high_resolution_clock::now() - start)
            .count();

    return cycleElapsed / timeElapsed;
}

double Clock::_frequency = GetClockRate();