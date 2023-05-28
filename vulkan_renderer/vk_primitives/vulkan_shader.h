#pragma once

#include "vulkan/vulkan.h"

#include "logger.h"
#include "validation.h"

#include <string>
#include <fstream>
#include <iostream>
#include <tracy/Tracy.hpp>

/**
 * Stateless class that loads glsl shaders
*/
class VulkanShader
{
public:
    static VkPipelineShaderStageCreateInfo LoadFromFile(VkDevice vkDevice, std::string filePath, VkShaderStageFlagBits stage)
    {
        ZoneScopedN("VulkanShader::LoadFromFile");
        
        VkPipelineShaderStageCreateInfo shaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        shaderStageInfo.stage = stage;
        shaderStageInfo.module = CreateModule(filePath, vkDevice);
        shaderStageInfo.pName = "main";
        return shaderStageInfo;
    };

    static VkPipelineShaderStageCreateInfo LoadAndCompileFromFile(std::string filePath){return VkPipelineShaderStageCreateInfo();}

private:
    static VkShaderModule CreateModule(std::string filePath, VkDevice vkDevice)
    {
        ZoneScopedN("VulkanShader::CreateModule");

        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            std::cout << "[Vulkan Shader] Error: failed to load shader from filesystem with path:" << std::endl
                      << filePath << std::endl;
            throw;
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        Log::Write(Log::Level::Info, "Load bytes: " + std::to_string(fileSize));

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        VkShaderModule shaderModule;
        VkShaderModuleCreateInfo moduleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        moduleCreateInfo.codeSize = fileSize;
        moduleCreateInfo.pCode = reinterpret_cast<uint32_t*>(buffer.data());

        CHECK_VKCMD(vkCreateShaderModule(vkDevice, &moduleCreateInfo, NULL, &shaderModule));

        return shaderModule;
    }
};