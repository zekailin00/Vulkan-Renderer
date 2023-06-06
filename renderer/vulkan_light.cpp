#include "vulkan_light.h"

#include "vulkan_renderer.h"

#include <memory>
#include <tracy/Tracy.hpp>

namespace renderer
{

std::shared_ptr<VulkanLight> VulkanLight::defaultLight;

std::shared_ptr<VulkanLight> VulkanLight::BuildLight(LightProperties& prop)
{

    ZoneScopedN("VulkanLight::BuildLight");

    std::shared_ptr<VulkanLight> light = std::make_shared<VulkanLight>();

    light->properties = prop;

    // For now it only supports direction light
    if (light->properties.type != DIRECTIONAL_LIGHT)
        throw;

    glm::vec3 direction = light->dirLight.direction;
    light->dirLight.direction = 
        glm::vec4(direction / glm::length(direction), 0);
    light->dirLight.color = glm::vec4(prop.color, 0);

    return light;
}

std::shared_ptr<VulkanLight> VulkanLight::GetDefaultLight()
{
    ZoneScopedN("VulkanLight::GetDefaultLight");

    if (defaultLight)
        return defaultLight;

    LightProperties prop{};

    VulkanLight::defaultLight = BuildLight(prop);
    return defaultLight;
}

void VulkanLight::SetTransform(glm::mat4& transform)
{
    ZoneScopedN("VulkanLight::SetTransform");

    glm::vec4 up{0, 1, 0, 0};
    glm::vec3 direction = transform * up; //FIXME: needs to check direction
    this->dirLight.direction = 
        glm::vec4(direction / glm::length(direction), 0);
}

const LightProperties& VulkanLight::GetLightProperties()
{
    ZoneScopedN("VulkanLight::GetLightProperties");

    return this->properties;
}

void VulkanLight::SetLightProperties(LightProperties& prop)
{
    ZoneScopedN("VulkanLight::SetLightProperties");

    this->properties = prop;
}

void VulkanLight::Destroy()
{
    ZoneScopedN("VulkanLight::Destroy");

    /* Vulkan resources are not handled here. Do nothing. */
}

} // namespace renderer