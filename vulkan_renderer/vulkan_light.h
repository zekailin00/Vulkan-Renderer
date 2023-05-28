#pragma once

#include "light.h"

#include "vk_primitives/vulkan_uniform.h"
#include "pipeline_inputs.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace renderer
{

struct DirLight
{
    glm::vec4 direction{-1, -1, -1, 0};
    glm::vec4 color;
};
struct PointLight
{// TODO: not used yet
    glm::vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class VulkanLight: public Light
{
public:
    /**
     * Return a VulkanLight.
     * Note that even though it is a shared pointer.
     * It must only be added to one node.
     * There is no check for it, but can cause undefined error
     * if a light is added to multiple nodes.
    */
    static std::shared_ptr<VulkanLight> BuildLight(LightProperties&);
    static std::shared_ptr<VulkanLight> GetDefaultLight();

    void SetTransform(glm::mat4& transform); // Used by VulkanNode

    const LightProperties& GetLightProperties() override;
    void SetLightProperties(LightProperties&) override;

    VulkanLight() = default;
    ~VulkanLight() override = default; // no need to call Destroy.

    VulkanLight(const VulkanLight&) = delete;
    VulkanLight& operator=(const VulkanLight&) = delete;

    void Destroy();

public:
    DirLight dirLight{}; //FIXME:

private:
    LightProperties properties{};

    static std::shared_ptr<VulkanLight> defaultLight;
};

} // namespace renderer