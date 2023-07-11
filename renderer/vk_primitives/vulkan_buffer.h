#pragma once

#include <vulkan/vulkan.h>

#include "validation.h"

namespace renderer
{

template<typename T>
class VulkanBuffer
{
public:
    void Initialize(VkBufferUsageFlags usageFlags, unsigned int capacity)
    {
        if (initialized)
            Destroy();

        T* data = AllocateVulkanBuffer(
            usageFlags, capacity,
            this->vkBuffer, this->vkMemory
        );

        this->capacity = capacity;
        this->size = 0;
        this->data = data;

        this->initialized = true;
    }

    void Destroy()
    {
        if (!initialized)
            return;
        
        VkDevice& vkDevice = vulkanDevice->vkDevice;
        vkDeviceWaitIdle(vulkanDevice->vkDevice);

        vkUnmapMemory(vulkanDevice->vkDevice, vkMemory);
        vkDestroyBuffer(vkDevice, vkBuffer, nullptr);
        vkFreeMemory(vkDevice, vkMemory, nullptr);

        vkBuffer = VK_NULL_HANDLE;
        vkMemory = VK_NULL_HANDLE;

        data = nullptr;
        capacity = 0;
        size = 0;
        initialized = false;
    }

    T* Data()
    {
        return data;
    }

    /**
     * @brief Get the element at index.
     * 
     * @param index The index muxt be less than the length of the array.
     * @return A reference to the object.
     */
    T& operator[](unsigned int index)
    {
        ASSERT(index < currentLength)
        return data[index];
    }
    
    void PushBack(const T& element)
    {
        //TODO: resizable
        ASSERT(size < capacity);
        data[size++] = element;
    }

    void Insert(unsigned int index, const T& element)
    {
        //TODO: resizable
        ASSERT(index <= size);
        ASSERT(size < capacity);

        int nextIndex = size - 1;
        while (nextIndex != index)
        {
            data[nextIndex] = data[nextIndex - 1];
            nextIndex--;
        }

        data[index] = element;
    }

    void Erase(unsigned int index, unsigned int nElements = 1)
    {
        ASSERT((index + nElements - 1) < size);

        unsigned int newSize = size - nElements;
        unsigned int nextIndex = index + nElements;
        while(nextIndex < size)
        {
            data[index] = data[nextIndex];
            index++;
            nextIndex++;
        }

        size = newSize;
    }

    void Clear()
    {
        size = 0;
    }

    unsigned int Size()
    {
        return size;
    }

    unsigned int Capacity()
    {
        return capacity;
    }

    VulkanBuffer(VulkanDevice* vulkanDevice, bool resizable):
        vulkanDevice(vulkanDevice), resizable(resizable) {}

private:
    T* AllocateVulkanBuffer(
        VkBufferUsageFlags usageFlags, unsigned int capacity,
        VkBuffer& vkBuffer, VkDeviceMemory& vkMemory
    )
    {
        VkDevice vkDevice = vulkanDevice->vkDevice;

        VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = sizeof(T) * capacity;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        CHECK_VKCMD(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkBuffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(
            memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkMemory));
        CHECK_VKCMD(vkBindBufferMemory(vkDevice, vkBuffer, vkMemory, 0));

        void* data;
        vkMapMemory(vkDevice, vkMemory, 0, bufferInfo.size, 0, &data);
        return static_cast<T*>(data);
    }

    void operator=(const VulkanBuffer<T>&) = delete;
    VulkanBuffer(const VulkanBuffer<T>&) = delete;

private:
    VulkanDevice *const vulkanDevice;
    const bool resizable;

    bool initialized = false;
    VkBuffer vkBuffer{VK_NULL_HANDLE};
    VkDeviceMemory vkMemory{VK_NULL_HANDLE};

    T* data = nullptr;
    unsigned int capacity = 0;
    unsigned int size = 0;
};

} // namespace renderer
