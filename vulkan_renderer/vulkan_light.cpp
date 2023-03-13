#include "vulkan_light.h"

#include "vulkan_renderer.h"

#include <memory>

namespace renderer
{

std::shared_ptr<VulkanLight> VulkanLight::defaultLight;

std::shared_ptr<VulkanLight> VulkanLight::BuildLight(LightProperties& prop)
{
    std::shared_ptr<VulkanLight> light = std::make_shared<VulkanLight>();

    light->properties = prop;

    // For now it only supports direction light
    if (light->properties.type != DIRECTIONAL_LIGHT)
        throw;

    glm::vec3 direction = light->dirLight.direction;
    light->dirLight.direction = direction / glm::length(direction);

    return light;
}

std::shared_ptr<VulkanLight> VulkanLight::GetDefaultLight()
{
    if (defaultLight)
        return defaultLight;

    LightProperties prop{};
    prop.type = LightType::DIRECTIONAL_LIGHT;
    prop.color = {255,255,255};

    VulkanLight::defaultLight = BuildLight(prop);
    return defaultLight;
}

void VulkanLight::SetTransform(glm::mat4& transform)
{
    glm::vec4 up{0, 1, 0, 0};
    glm::vec3 direction = transform * up; //FIXME: needs to check direction
    this->dirLight.direction = direction / glm::length(direction);
}

const LightProperties& VulkanLight::GetLightProperties()
{
    return this->properties;
}

void VulkanLight::SetLightProperties(LightProperties& prop)
{
    this->properties = prop;
}

void VulkanLight::Destroy()
{
    /* Vulkan resources are not handled here. Do nothing. */
}

} // namespace renderer