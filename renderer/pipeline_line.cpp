#include "pipeline_line.h"

#include "vk_primitives/vulkan_pipeline.h"
#include "vk_primitives/vulkan_pipeline_layout.h"
#include "vk_primitives/vulkan_device.h"
#include "validation.h"
#include "input.h"

#include <memory>
#include <array>
#include <vector>
#include <tracy/Tracy.hpp>

namespace renderer
{

PipelineLine::PipelineLine(VulkanDevice& vulkanDevice,
    VkDescriptorPool vkDescriptorPool, VkRenderPass renderpass)
{
    this->vulkanDevice = &vulkanDevice;
    this->vkDescriptorPool = vkDescriptorPool;

    linePipeline = std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
    PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
    std::unique_ptr<VulkanPipelineLayout> wireLayout;

    linePipeline->LoadShader("resources/vulkan_shaders/line/vert.spv",
                             "resources/vulkan_shaders/line/frag.spv");

    layoutBuilder.PushDescriptorSetLayout("transform",
    {
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
    });

    layoutBuilder.PushDescriptorSetLayout("camera",
    {
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
    });

    layoutBuilder.PushDescriptorSetLayout("lineProperties",
    {
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
    });

    wireLayout = layoutBuilder.BuildPipelineLayout(
        vkDescriptorPool
    );

    linePipeline->rasterState.cullMode = VK_CULL_MODE_NONE;

    std::vector<VkVertexInputBindingDescription> bindingDesc =
    {
        {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX},
        {1, sizeof(LineData), VK_VERTEX_INPUT_RATE_INSTANCE},
    };

    std::vector<VkVertexInputAttributeDescription> attributeDesc =
    {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position)},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal)},
        {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoords)},
        {3, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LineData, beginPoint)},
        {4, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LineData, endPoint)},
    };

    VkPipelineVertexInputStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = bindingDesc.size();
    info.pVertexBindingDescriptions = bindingDesc.data();
    info.vertexAttributeDescriptionCount = attributeDesc.size();
    info.pVertexAttributeDescriptions = attributeDesc.data();

    linePipeline->BuildPipeline(
        &info,
        std::move(wireLayout),
        renderpass
    );
}

PipelineLine::~PipelineLine()
{
    vkDeviceWaitIdle(vulkanDevice->vkDevice);
    linePipeline = nullptr;
}

void PipelineLine::Render(
    std::vector<std::shared_ptr<LineRenderer>>& lineList,
    VkDescriptorSet* cameraDescSet, glm::vec2 extent,
    VkCommandBuffer commandbuffer)
{
    vkCmdBindPipeline(commandbuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        linePipeline->pipeline
    );

    vkCmdBindDescriptorSets(
        commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
        linePipeline->pipelineLayout->layout,
        2, 1, cameraDescSet, 0, nullptr
    );

    for(const std::shared_ptr<LineRenderer> e: lineList)
    {
        // e->GetLineData()->
    }

}


} // namespace renderer
