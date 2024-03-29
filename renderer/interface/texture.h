#pragma once

#include <memory>
#include <string>

#include <json/json.h>
#include <glm/vec2.hpp>

#define DEFAULT_TEXTURE_PATH "@default"


namespace renderer
{

enum TextureType
{
    TEX_DEFAULT,
    TEX_CUBEMAP
};

enum TextureAddressMode
{
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
};

enum TextureFilter
{
    FILTER_NEAREST,
    FILTER_LINEAR
};

struct TextureBuildInfo
{
    std::string imagePath = "resources/textures/defaultTexture.png";
    std::string resourcePath = DEFAULT_TEXTURE_PATH;
    TextureAddressMode addressMode = REPEAT;
    TextureFilter minFilter = FILTER_LINEAR;
    TextureFilter maxFilter = FILTER_LINEAR;
};

struct TextureCubeBuildInfo
{
    std::string x_pos_path = "resources/textures/skybox/right.jpg";
    std::string x_neg_path = "resources/textures/skybox/left.jpg";
    std::string y_pos_path = "resources/textures/skybox/top.jpg";
    std::string y_neg_path = "resources/textures/skybox/bottom.jpg";
    std::string z_pos_path = "resources/textures/skybox/back.jpg";
    std::string z_neg_path = "resources/textures/skybox/front.jpg";
    TextureAddressMode addressMode = REPEAT;
    TextureFilter minFilter = FILTER_LINEAR;
    TextureFilter maxFilter = FILTER_LINEAR;
};

class Texture
{

public:
    Texture() = default;
    virtual ~Texture() = default;

    virtual const TextureBuildInfo& GetBuildInfo() = 0;
    virtual const TextureType& GetTextureType() = 0;
    virtual glm::vec2 GetExtent() = 0;
    virtual void Serialize(Json::Value& json) = 0;

    Texture(const Texture&) = delete;
    const Texture& operator=(const Texture&) = delete;
};

class TextureCube
{

public:
    TextureCube() = default;
    virtual ~TextureCube() = default;

    TextureCube(const TextureCube&) = delete;
    const TextureCube& operator=(const TextureCube&) = delete;

public:
    TextureCubeBuildInfo info;
    TextureType textureType;
};
    
} // namespace renderer