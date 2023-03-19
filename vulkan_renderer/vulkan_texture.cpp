#include "vulkan_texture.h"

#include "vulkan_renderer.h"

#include "validation.h"
#include "logger.h"
#include "stb/stb_image.h"

#include <memory>


namespace renderer
{

std::shared_ptr<VulkanTexture> VulkanTexture::defaultTexture;
std::shared_ptr<VulkanTextureCube> VulkanTextureCube::defaultTexture;

void VulkanTexture::CreateImage(
    VkExtent2D imageExtent, VkFormat colorFormat, VkImageUsageFlags usage)
{
    vulkanDevice = &VulkanRenderer::GetInstance().vulkanDevice;
    VkDevice vkDevice = vulkanDevice->vkDevice;
    this->imageExtent = imageExtent;

    // Create image
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = imageExtent.width;
    imageInfo.extent.height = imageExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = colorFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CHECK_VKCMD(vkCreateImage(vkDevice, &imageInfo, nullptr, &vkImage));

    // Allocate image memory
    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(vkDevice, vkImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkDeviceMemory));
    CHECK_VKCMD(vkBindImageMemory(vkDevice, vkImage, vkDeviceMemory, 0));

    // Allocate image view 
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = colorFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    CHECK_VKCMD(vkCreateImageView(vkDevice, &viewInfo, nullptr, &vkImageView));
}

void VulkanTexture::LoadImageFromFile(std::string filePath)
{
    VkDevice vkDevice = vulkanDevice->vkDevice;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        Log::Write(Log::Level::Error,
            "[Vulkan Texture] Error, failed to load texture image.");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // Transfer data to buffer
    {
        VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = imageSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        CHECK_VKCMD(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &stagingBuffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vkDevice, stagingBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(
            memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &stagingBufferMemory));

        vkBindBufferMemory(vkDevice, stagingBuffer, stagingBufferMemory, 0);

        void* data;
        vkMapMemory(vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(vkDevice, stagingBufferMemory);
    }

    stbi_image_free(pixels);
    CreateImage(
        {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)},
        VK_FORMAT_R8G8B8A8_SRGB);

    // Transfer data from buffer to device local memory
    {
        VulkanSingleCmd cmd;
        cmd.Initialize(vulkanDevice);
        VkCommandBuffer vkCommandBuffer = cmd.BeginCommand();

        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vkImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        vkCmdPipelineBarrier(
            vkCommandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight), 1
        };

        vkCmdCopyBufferToImage(
            vkCommandBuffer, stagingBuffer, vkImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier(
            vkCommandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        cmd.EndCommand();
    }

    vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
}

void VulkanTexture::CreateSampler(
    VkFilter minFilter, VkFilter magFilter,
    VkSamplerAddressMode addressMode)
{
    VkDevice vkDevice = vulkanDevice->vkDevice;

    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter = magFilter;
    samplerInfo.minFilter = minFilter;
    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    CHECK_VKCMD(vkCreateSampler(vkDevice, &samplerInfo, nullptr, &vkSampler));
}

VkDescriptorImageInfo* VulkanTexture::GetDescriptor(VkImageLayout vkImageLayout)
{
    vkDecriptorInfo.imageLayout = vkImageLayout;
    vkDecriptorInfo.imageView = vkImageView;
    vkDecriptorInfo.sampler = vkSampler;

    if (vkDecriptorInfo.sampler == VK_NULL_HANDLE)
        Log::Write(Log::Level::Error,
            "[Vulkan Texture] Error, Sampler is null when creating image view.");
    return &vkDecriptorInfo;
}

void VulkanTexture::Destroy()
{
    VkDevice& vkDevice = vulkanDevice->vkDevice;
    vkDeviceWaitIdle(vkDevice); 
    // FIXME: wait until this resource has been used by GPU
    // Not the best way to do this.
    vkDestroySampler(vkDevice, vkSampler, nullptr);
    vkDestroyImageView(vkDevice, vkImageView, nullptr);
    vkDestroyImage(vkDevice, vkImage, nullptr);
    vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);
    vulkanDevice = nullptr;
}

std::shared_ptr<Texture> VulkanTexture::BuildTexture(TextureBuildInfo* buildInfo)
{
    std::shared_ptr<VulkanTexture> texture = std::make_shared<VulkanTexture>();
    texture->vulkanDevice = &(VulkanRenderer::GetInstance().vulkanDevice);
    texture->textureType = TextureType::TEX_DEFAULT;

    texture->LoadImageFromFile(buildInfo->path);

    {
        VkFilter minFilter = VK_FILTER_LINEAR;
        switch (buildInfo->minFilter)
        {
        case FILTER_NEAREST:
            minFilter = VK_FILTER_NEAREST;
            break;
        case FILTER_LINEAR:
            minFilter = VK_FILTER_LINEAR;
            break;
        }

        VkFilter maxFilter = VK_FILTER_LINEAR;
        switch (buildInfo->maxFilter)
        {
        case FILTER_NEAREST:
            maxFilter = VK_FILTER_NEAREST;
            break;
        case FILTER_LINEAR:
            maxFilter = VK_FILTER_LINEAR;
            break;
        default:
            break;
        }

        VkSamplerAddressMode addressMode;
        switch (buildInfo->addressMode)
        {
        case REPEAT:
            addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            break;
        case MIRRORED_REPEAT:
            addressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            break;
        case CLAMP_TO_EDGE:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case CLAMP_TO_BORDER:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;
        default:
            break;
        }

        texture->CreateSampler(minFilter, maxFilter, addressMode);
    }
    
    texture->info = *buildInfo;
    return texture;
}

std::shared_ptr<VulkanTexture> VulkanTexture::GetDefaultTexture()
{
    if (defaultTexture == nullptr)
    {
        struct TextureBuildInfo info{};
        defaultTexture = std::dynamic_pointer_cast<VulkanTexture>(BuildTexture(&info));
    }

    return defaultTexture;
}

VulkanTexture::~VulkanTexture()
{
    Destroy();
    Log::Write(Log::Level::Verbose, "VulkanTexture Destoryed.\n");
}


std::shared_ptr<TextureCube> VulkanTextureCube::BuildTexture(TextureCubeBuildInfo& buildInfo)
{
    std::shared_ptr<VulkanTextureCube> texture = std::make_shared<VulkanTextureCube>();
    texture->vulkanDevice = &(VulkanRenderer::GetInstance().vulkanDevice);
    texture->textureType = TextureType::TEX_DEFAULT;

    texture->LoadImagesFromFile(buildInfo);

    {
        VkFilter minFilter = VK_FILTER_LINEAR;
        switch (buildInfo.minFilter)
        {
        case FILTER_NEAREST:
            minFilter = VK_FILTER_NEAREST;
            break;
        case FILTER_LINEAR:
            minFilter = VK_FILTER_LINEAR;
            break;
        }

        VkFilter maxFilter = VK_FILTER_LINEAR;
        switch (buildInfo.maxFilter)
        {
        case FILTER_NEAREST:
            maxFilter = VK_FILTER_NEAREST;
            break;
        case FILTER_LINEAR:
            maxFilter = VK_FILTER_LINEAR;
            break;
        default:
            break;
        }

        VkSamplerAddressMode addressMode;
        switch (buildInfo.addressMode)
        {
        case REPEAT:
            addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            break;
        case MIRRORED_REPEAT:
            addressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            break;
        case CLAMP_TO_EDGE:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case CLAMP_TO_BORDER:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;
        default:
            break;
        }

        VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerInfo.magFilter = maxFilter;
        samplerInfo.minFilter = minFilter;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        CHECK_VKCMD(vkCreateSampler(texture->vulkanDevice->vkDevice,
            &samplerInfo, nullptr, &texture->vkSampler));
    }
    
    texture->info = buildInfo;
    return texture;
}

void VulkanTextureCube::LoadImagesFromFile(TextureCubeBuildInfo& buildInfo)
{
    VkDevice vkDevice = vulkanDevice->vkDevice;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        buildInfo.z_pos_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        Log::Write(Log::Level::Error,
            "[Vulkan Texture] Error, failed to load texture image.");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // Transfer data to buffer
    {
        VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = imageSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        CHECK_VKCMD(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &stagingBuffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vkDevice, stagingBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(
            memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &stagingBufferMemory));

        vkBindBufferMemory(vkDevice, stagingBuffer, stagingBufferMemory, 0);
    }

    stbi_image_free(pixels);

    this->imageExtent = {
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight)
    };

    // Create image
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = imageExtent.width;
    imageInfo.extent.height = imageExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CHECK_VKCMD(vkCreateImage(vkDevice, &imageInfo, nullptr, &vkImage));

    // Allocate image memory
    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(vkDevice, vkImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkDeviceMemory));
    CHECK_VKCMD(vkBindImageMemory(vkDevice, vkImage, vkDeviceMemory, 0));

    // Allocate image view 
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;

    CHECK_VKCMD(vkCreateImageView(vkDevice, &viewInfo, nullptr, &vkImageView));

    std::string files[6] = {
        buildInfo.z_pos_path, buildInfo.z_neg_path,
        buildInfo.y_pos_path, buildInfo.y_neg_path,
        buildInfo.x_pos_path, buildInfo.z_neg_path
    };

    for (int i = 0; i < 6; i++)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(
            files[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            Log::Write(Log::Level::Error,
                "[Vulkan Texture] Error, failed to load texture image.");
        }

        void* data;
        vkMapMemory(vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(vkDevice, stagingBufferMemory);

        stbi_image_free(pixels);

        // Transfer data from buffer to device local memory

        VulkanSingleCmd cmd;
        cmd.Initialize(vulkanDevice);
        VkCommandBuffer vkCommandBuffer = cmd.BeginCommand();

        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vkImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = i;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        vkCmdPipelineBarrier(
            vkCommandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = i;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight), 1
        };

        vkCmdCopyBufferToImage(
            vkCommandBuffer, stagingBuffer, vkImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier(
            vkCommandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        cmd.EndCommand();
    }

    vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
}

std::shared_ptr<VulkanTextureCube> VulkanTextureCube::GetDefaultTexture()
{
    if (defaultTexture == nullptr)
    {
        struct TextureCubeBuildInfo info{};
        defaultTexture = std::dynamic_pointer_cast<VulkanTextureCube>(BuildTexture(info));
    }

    return defaultTexture;
}

VkDescriptorImageInfo* VulkanTextureCube::GetDescriptor(VkImageLayout vkImageLayout)
{
    vkDecriptorInfo.imageLayout = vkImageLayout;
    vkDecriptorInfo.imageView = vkImageView;
    vkDecriptorInfo.sampler = vkSampler;

    if (vkDecriptorInfo.sampler == VK_NULL_HANDLE)
        Log::Write(Log::Level::Error,
            "[Vulkan Texture] Error, Sampler is null when creating image view.");
    return &vkDecriptorInfo;
}

VulkanTextureCube::~VulkanTextureCube()
{
    Destroy();
    Log::Write(Log::Level::Verbose, "VulkanTextureCube Destoryed.\n");
}

void VulkanTextureCube::Destroy()
{
    VkDevice& vkDevice = vulkanDevice->vkDevice;
    vkDeviceWaitIdle(vkDevice); 

    vkDestroySampler(vkDevice, vkSampler, nullptr);
    vkDestroyImageView(vkDevice, vkImageView, nullptr);
    vkDestroyImage(vkDevice, vkImage, nullptr);
    vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);
    vulkanDevice = nullptr;
}

} // namespace renderer