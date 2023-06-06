#pragma once

#include <memory>
#include <string>


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
    std::string path = "resources/textures/defaultTexture.png";
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

    Texture(const Texture&) = delete;
    const Texture& operator=(const Texture&) = delete;

public:
    TextureBuildInfo info;
    TextureType textureType;
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