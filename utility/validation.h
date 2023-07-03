#pragma once 

#include <iostream>

#define CHECK_VKCMD(result)                                                         \
    if (result < 0) {                                                               \
        std::cerr << "[Vulkan Renderer] Error:" << result << std::endl;             \
        throw;}                                                                   

#define ASSERT(result)                                                              \
    if ((result) != true) {                                                           \
        std::cerr << "[Vulkan Renderer] Evaluated to false.";                       \
        throw;}                                                                   