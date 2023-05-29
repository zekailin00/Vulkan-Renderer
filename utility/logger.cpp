#include "logger.h"

#include <iostream>
#include <cstdlib>
#include <tracy/Tracy.hpp>


Logger::Level Logger::currentLevel{Logger::Level::Info};

void Logger::SetLevel(Level minLevel)
{
    currentLevel = minLevel;
}

void Logger::Write(const std::string& msg,
    Level level, MsgType type)
{
    if (level < currentLevel)
        return;

    std::cout << msg << std::endl;

    if (level == Logger::Level::Error)
        throw;

    return;
}
