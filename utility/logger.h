#pragma once

#include <string>


inline std::string Fmt(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    int size = std::vsnprintf(nullptr, 0, fmt, vl);
    va_end(vl);

    if (size != -1) {
        std::unique_ptr<char[]> buffer(new char[size + 1]);

        va_start(vl, fmt);
        size = std::vsnprintf(buffer.get(), size + 1, fmt, vl);
        va_end(vl);
        if (size != -1) {
            return std::string(buffer.get(), size);
        }
    }

    throw std::runtime_error("Unexpected vsnprintf failure");
}


namespace Log 
{

enum class Level { Verbose, Info, Warning, Error};

void SetLevel(Level minSeverity);

/**
 * @brief Output the message with severity info.
 * If severity is error. The program exits.
 * 
 * @param severity 
 * @param msg 
 */
void Write(Level severity, const std::string& msg);

}  // namespace Log
