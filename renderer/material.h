#pragma once

#include <glm/glm.hpp>

#include <memory>

namespace renderer
{

// Material properties with default values defined.
struct MaterialProp
{
    glm::vec3 albedo = {255, 255, 255};
    void* albedoTexture = nullptr;

    float metallic = 0.0f;
    void* metalicTexture = nullptr;

    float smoothness = 0.5f;
    void* smoothnessTexture = nullptr;

    void* normalMap = nullptr;
};

class Material
{

public:
    static std::shared_ptr<Material> BuildMaterial();

    MaterialProp* GetProperties();
    MaterialProp* ResetProperties();

    Material() = default;
    ~Material() = default;

    Material(const Material&) = delete;
    const Material& operator=(const Material&) = delete;

private:
    MaterialProp properties;
};

} // namespace renderer