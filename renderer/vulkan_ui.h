#pragma once

#include "ui.h"

#include "vk_primitives/vulkan_device.h"
#include "vulkan_texture.h"

#include "event_queue.h"

#include <glm/vec2.hpp>
#include <imgui.h>
#include <memory>

namespace renderer
{

class RenderTechnique;
class WindowEventHandler;

class VulkanUI: public UI
{

public:
    static std::shared_ptr<VulkanUI> BuildUI(UIBuildInfo& info);

    VulkanUI() = default;
    ~VulkanUI() override;

    VulkanUI(const VulkanUI&) = delete;
    VulkanUI& operator=(const VulkanUI&) = delete;

    void SetExtent(glm::vec2 extent) override;
    
    std::shared_ptr<Texture> GetTexture() override;

    void Destroy();

    void RenderUI();

    friend RenderTechnique;

private:
    glm::vec2 extent;
    std::function<void()> renderUI;

    bool editorUI;
    WindowEventHandler* windowEventHandler;
    int subscriberHandle;

    ImDrawData* drawData;

    VkDeviceMemory      vertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize        vertexBufferSize = 0;
    VkBuffer            vertexBuffer = VK_NULL_HANDLE;

    VkDeviceMemory      indexBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize        indexBufferSize = 0;
    VkBuffer            indexBuffer = VK_NULL_HANDLE;

    VulkanDevice* vulkanDevice;
    std::shared_ptr<VulkanTexture> colorImage;
    VkFramebuffer framebuffer;

private:
    void Initialize(glm::vec2& extent, std::function<void()> renderUI);
    
    void MapData();

    void CreateOrResizeBuffer(
        VkBuffer& buffer, VkDeviceMemory& buffer_memory,
        VkDeviceSize& p_buffer_size, size_t new_size, VkBufferUsageFlagBits usage);
};

class WindowEventHandler
{
public:
    class Subscriber
    {
    public:
        Subscriber(WindowEventHandler* handler)
        {
            this->hanlder = handler;
        }

        void operator()(Event* event);

    private:
        WindowEventHandler* hanlder;
    };

    Subscriber GetSubscriber()
    {
        return Subscriber(this);
    };

    void ProcessInputs();

private:
    std::list<Event*> queue;
};

} // namespace renderer
