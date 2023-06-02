#pragma once

#include <chrono>
#include <cstdint>

using namespace std::chrono;

class Clock {
  public:
    void Restart() noexcept { _start = high_resolution_clock::now(); }

    uint64_t GetElapsedNanoseconds() noexcept {
        return duration_cast<nanoseconds>(high_resolution_clock::now() - _start)
            .count();
    }

  private:
    high_resolution_clock::time_point _start = high_resolution_clock::now();
};