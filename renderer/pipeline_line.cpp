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
    std::unique_ptr<VulkanPipeline> wirePipeline = 
            std::make_unique<VulkanPipeline>(vulkanDevice.vkDevice);
    PipelineLayoutBuilder layoutBuilder(&vulkanDevice);
    std::unique_ptr<VulkanPipelineLayout> wireLayout;

    wirePipeline->LoadShader("resources/vulkan_shaders/wire/vert.spv",
                             "resources/vulkan_shaders/wire/frag.spv");
    
    layoutBuilder.PushDescriptorSetLayout("camera",
    {
        /*
        layout (set = 0, binding = 0) uniform ViewProjection 
        {
            mat4 view;
            mat4 projection;
        } vp;
        */
        layoutBuilder.descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
    });

    VkPushConstantRange range = {};
    range.offset = 0;
    range.size = sizeof(WirePushConst);
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    wireLayout = layoutBuilder.BuildPipelineLayout(
        vkDescriptorPool,
        &range
    );

    wirePipeline->rasterState.cullMode = VK_CULL_MODE_NONE;

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
        {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LineData, beginPoint)},
        {1, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LineData, endPoint)},
    };


    VkPipelineVertexInputStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = bindingDesc.size();
    info.pVertexBindingDescriptions = bindingDesc.data();
    info.vertexAttributeDescriptionCount = attributeDesc.size();
    info.pVertexAttributeDescriptions = attributeDesc.data();

    wirePipeline->BuildPipeline(
        &info,
        std::move(wireLayout),
        renderpass
    );

    pipelines["wire"] = std::move(wirePipeline);
}

PipelineLine::~PipelineLine()
{

}

void PipelineLine::Render(std::vector<std::shared_ptr<LineRenderer>> lineList,
    VkBuffer vertexBuffer, VkBuffer indexBuffer, VkCommandBuffer commandbuffer)
{
    
}

} // namespace renderer
