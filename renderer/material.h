#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "texture.h"

namespace renderer
{

// Material properties with default values defined.
struct MaterialProperties
{
    glm::vec3 albedo = {255, 255, 255};
    std::shared_ptr<Texture> albedoTexture = nullptr;

    float metallic = 0.0f;
    std::shared_ptr<Texture> metalicTexture = nullptr;

    float smoothness = 0.5f;
    std::shared_ptr<Texture> smoothnessTexture = nullptr;

    std::shared_ptr<Texture> normalTexture = nullptr;
};

class Material
{

public:

    MaterialProperties* GetProperties();
    MaterialProperties* ResetProperties();

    virtual void AddAlbedoTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddMetallicTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddSmoothnessTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddNormalTexture(std::shared_ptr<Texture> texture) = 0;


    Material() = default;
    ~Material() = default;

    Material(const Material&) = delete;
    const Material& operator=(const Material&) = delete;

protected:
    MaterialProperties properties;

    std::shared_ptr<Texture> albedoTex;
    std::shared_ptr<Texture> metallicTex;
    std::shared_ptr<Texture> smoothnessTex;
    std::shared_ptr<Texture> normalTex;
};

} // namespace renderer