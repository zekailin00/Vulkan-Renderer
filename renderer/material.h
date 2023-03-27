#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "texture.h"


namespace renderer
{

// Material properties with default values defined.
struct MaterialProperties
{
    glm::vec3 albedo = {1, 1, 1}; //  // [0, 1] for RGB
    std::shared_ptr<Texture> albedoTexture = nullptr;

    float metallic = 0.1f;
    std::shared_ptr<Texture> metallicTexture = nullptr;

    float smoothness = 0.1f;
    std::shared_ptr<Texture> smoothnessTexture = nullptr;

    std::shared_ptr<Texture> normalTexture = nullptr;
};

class Material
{

public:
    virtual MaterialProperties* GetProperties() = 0;
    virtual void ResetProperties() = 0;

    virtual void AddAlbedoTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddMetallicTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddSmoothnessTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddNormalTexture(std::shared_ptr<Texture> texture) = 0;

    virtual void ResetAlbedoTexture() = 0;
    virtual void ResetMetallicTexture() = 0;
    virtual void ResetSmoothnessTexture() = 0;
    virtual void ResetNormalTexture() = 0;

    Material() = default;
    virtual ~Material() = default;

    Material(const Material&) = delete;
    const Material& operator=(const Material&) = delete;

protected:
    MaterialProperties properties;
};

} // namespace renderer