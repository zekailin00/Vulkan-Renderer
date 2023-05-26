#include "vulkan_ui.h"

#include "vulkan_renderer.h"
#include "validation.h"

#include <memory>

namespace renderer
{

std::shared_ptr<VulkanUI> VulkanUI::BuildUI(UIBuildInfo& info)
{
    std::shared_ptr<VulkanUI> ui = std::make_shared<VulkanUI>();
    ui->Initialize(info.extent, info.imgui);

    return ui;
}

VulkanUI::~VulkanUI()
{
    Destroy();
}

void VulkanUI::Destroy()
{
    vkDeviceWaitIdle(vulkanDevice->vkDevice);

    if (vertexBuffer)
    {
        vkDestroyBuffer(vulkanDevice->vkDevice, vertexBuffer, nullptr);
        vertexBuffer = VK_NULL_HANDLE;
    }
    if (vertexBufferMemory)
    {
        vkFreeMemory(vulkanDevice->vkDevice, vertexBufferMemory, nullptr);
        vertexBufferMemory = VK_NULL_HANDLE;
    }
    if (indexBuffer)
    {
        vkDestroyBuffer(vulkanDevice->vkDevice, indexBuffer, nullptr);
        indexBuffer = VK_NULL_HANDLE;
    }
    if (indexBufferMemory)
    {
        vkFreeMemory(vulkanDevice->vkDevice, indexBufferMemory, nullptr);
        indexBufferMemory = VK_NULL_HANDLE;
    }

    vertexBufferSize = 0;
    indexBufferSize = 0;

    this->colorImage = nullptr;
    vkDestroyFramebuffer(vulkanDevice->vkDevice,framebuffer, nullptr);

    this->vulkanDevice = nullptr;
    this->renderUI = nullptr;
}

void VulkanUI::RenderUI()
{
    ImGui::NewFrame();
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::SetNextWindowSize({extent.x, extent.y});
    ImGui::Begin("window", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse
    );
    this->renderUI();
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    this->drawData = ImGui::GetDrawData();
    MapData();
}

void VulkanUI::SetExtent(glm::vec2 extent)
{
    void (*renderUI)(void) = this->renderUI;
    Destroy();
    Initialize(extent, renderUI);
}

std::shared_ptr<Texture> VulkanUI::GetTexture()
{
    return colorImage;
}

void VulkanUI::Initialize(glm::vec2& extent, void (*renderUI)(void))
{
    this->extent = extent;
    this->renderUI = renderUI;

    VulkanRenderer& vkr = VulkanRenderer::GetInstance();
    IVulkanSwapchain *swapchain = vkr.GetSwapchain();
    this->vulkanDevice = &vkr.vulkanDevice;

    // Create color image
    this->colorImage = std::make_shared<VulkanTexture>();
    this->colorImage->CreateImage({
            static_cast<unsigned int>(this->extent.x),
            static_cast<unsigned int>(this->extent.y)},
        swapchain->GetImageFormat(),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    this->colorImage->CreateSampler();

    std::vector<VkImageView> attachments = {this->colorImage->GetImageView()};
    VkFramebufferCreateInfo vkFramebufferCreateInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    vkFramebufferCreateInfo.renderPass = vkr.vkRenderPass.imgui;
    vkFramebufferCreateInfo.attachmentCount = attachments.size();
    vkFramebufferCreateInfo.pAttachments = attachments.data();
    vkFramebufferCreateInfo.width = this->extent.x;
    vkFramebufferCreateInfo.height = this->extent.y;
    vkFramebufferCreateInfo.layers = 1;

    CHECK_VKCMD(vkCreateFramebuffer(
        this->vulkanDevice->vkDevice, 
        &vkFramebufferCreateInfo, nullptr,
        &this->framebuffer));
}

void VulkanUI::MapData()
{
    // Allocate array to store enough vertex/index buffers
    if (drawData->TotalVtxCount > 0)
    {
        // Create or resize the vertex/index buffers
        size_t vertex_size = drawData->TotalVtxCount * sizeof(ImDrawVert);
        size_t index_size = drawData->TotalIdxCount * sizeof(ImDrawIdx);
        if (vertexBuffer == VK_NULL_HANDLE ||
            vertexBufferSize < vertex_size)
            CreateOrResizeBuffer(vertexBuffer, vertexBufferMemory,
                vertexBufferSize, vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        if (indexBuffer == VK_NULL_HANDLE ||
            indexBufferSize < index_size)
            CreateOrResizeBuffer(indexBuffer, indexBufferMemory,
                indexBufferSize, index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        // Upload vertex/index data into a single contiguous GPU buffer
        ImDrawVert* vtx_dst = nullptr;
        ImDrawIdx* idx_dst = nullptr;
        VkResult err = vkMapMemory(vulkanDevice->vkDevice, vertexBufferMemory, 0,
            vertexBufferSize, 0, (void**)(&vtx_dst));
        CHECK_VKCMD(err);
        err = vkMapMemory(vulkanDevice->vkDevice, indexBufferMemory, 0,
            indexBufferSize, 0, (void**)(&idx_dst));
        CHECK_VKCMD(err);
        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = drawData->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
        VkMappedMemoryRange range[2] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = vertexBufferMemory;
        range[0].size = VK_WHOLE_SIZE;
        range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[1].memory = indexBufferMemory;
        range[1].size = VK_WHOLE_SIZE;
        err = vkFlushMappedMemoryRanges(vulkanDevice->vkDevice, 2, range);
        CHECK_VKCMD(err);
        vkUnmapMemory(vulkanDevice->vkDevice, vertexBufferMemory);
        vkUnmapMemory(vulkanDevice->vkDevice, indexBufferMemory);
    }
}

void VulkanUI::CreateOrResizeBuffer(
    VkBuffer& buffer, VkDeviceMemory& buffer_memory,
    VkDeviceSize& p_buffer_size, size_t new_size, VkBufferUsageFlagBits usage)
{
    VkResult err;
    if (buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(vulkanDevice->vkDevice, buffer, nullptr);
    if (buffer_memory != VK_NULL_HANDLE)
        vkFreeMemory(vulkanDevice->vkDevice, buffer_memory, nullptr);

    VkDeviceSize vertex_buffer_size_aligned = new_size;
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = vertex_buffer_size_aligned;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vkCreateBuffer(vulkanDevice->vkDevice, &buffer_info, nullptr, &buffer);
    CHECK_VKCMD(err);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(vulkanDevice->vkDevice, buffer, &req);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(req.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    err = vkAllocateMemory(vulkanDevice->vkDevice, &alloc_info, nullptr, &buffer_memory);
    CHECK_VKCMD(err);

    err = vkBindBufferMemory(vulkanDevice->vkDevice, buffer, buffer_memory, 0);
    CHECK_VKCMD(err);
    p_buffer_size = req.size;
}

} // namespace renderer
