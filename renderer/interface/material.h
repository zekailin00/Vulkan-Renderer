#pragma once

#include <glm/glm.hpp>
#include <json/json.h>
#include <memory>
#include <string>

#include "texture.h"

#define DEFAULT_MATERIAL_PATH "@default"

namespace renderer
{

// Material properties with default values defined.
struct MaterialProperties
{
    std::string resourcePath = DEFAULT_MATERIAL_PATH;

    glm::vec3 albedo = {1, 1, 1}; //  // [0, 1] for RGB
    std::shared_ptr<Texture> albedoTexture = nullptr;

    float metallic = 0.0f;
    std::shared_ptr<Texture> metallicTexture = nullptr;

    float roughness = 1.0f;
    std::shared_ptr<Texture> roughnessTexture = nullptr;

    std::shared_ptr<Texture> normalTexture = nullptr;
};

class Material
{

public:
    virtual MaterialProperties* GetProperties() = 0;
    virtual void ResetProperties() = 0;

    virtual void SetAlbedo(glm::vec3 albedo) = 0;
    virtual void SetMetallic(float metallic) = 0;
    virtual void SetRoughness(float roughness) = 0;

    virtual void AddAlbedoTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddMetallicTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddRoughnessTexture(std::shared_ptr<Texture> texture) = 0;
    virtual void AddNormalTexture(std::shared_ptr<Texture> texture) = 0;

    virtual void ResetAlbedoTexture() = 0;
    virtual void ResetMetallicTexture() = 0;
    virtual void ResetRoughnessTexture() = 0;
    virtual void ResetNormalTexture() = 0;

    virtual void Serialize(Json::Value& json) = 0;

    Material() = default;
    virtual ~Material() = default;

    Material(const Material&) = delete;
    const Material& operator=(const Material&) = delete;
};

} // namespace renderer