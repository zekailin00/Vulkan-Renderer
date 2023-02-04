#include "logger.h"

#include <iostream>
#include <cstdlib>


namespace Log {
Log::Level currentSeverity{Log::Level::Info};

void SetLevel(Level minSeverity) {currentSeverity = minSeverity;}

void Write(Level severity, const std::string& msg) 
{
    if (severity < currentSeverity) {
        return;
    }

    std::cout << msg << std::endl;
    if (severity == Log::Level::Error)
        exit(1);
    return;
}
}  // namespace Log
