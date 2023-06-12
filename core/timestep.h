#pragma once

#include <chrono>

typedef float Timestep; /* Unit is second. Precision is nanosecond */
#define TO_SECOND(nanosec) (0.000000001f * nanosec)
#define TO_NANOSECOND(sec) (static_cast<int>(1000000000 * sec))

class Timer {
public:
    Timer() {previousTime = std::chrono::high_resolution_clock::now();}
    Timer(int maxFrameRate):maxFrameRate(maxFrameRate) {
        previousTime = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Get the minimal Timestep object. If max frame rate is not defined, it returns 0.0f. Unit is second.
     * 
     * @return Timestep 
     */
    Timestep GetMinimalTimestep() {
        if (maxFrameRate == -1)  return 0.0f;
        else return (1.0f / static_cast<float>(maxFrameRate));
    }

    /**
     * @brief Get the Timestep object. Unit is second. Precision is nanosecond. 
     * 
     * @return Timestep 
     */
    Timestep GetTimestep() {
        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
        long long timestep = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - previousTime).count();

        if (maxFrameRate == -1) {
            previousTime = currentTime;
            return TO_SECOND(timestep);
        }

        while (timestep < TO_NANOSECOND(GetMinimalTimestep())){
            currentTime = std::chrono::high_resolution_clock::now();
            timestep = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - previousTime).count();
        }

        previousTime = currentTime;
        return TO_SECOND(timestep);
    } 

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> previousTime;
    const int maxFrameRate = -1;
};