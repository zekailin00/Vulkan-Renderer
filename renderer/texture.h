#pragma once

#include <memory>
#include <string>


namespace renderer
{

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

class Texture
{

public:
    static std::shared_ptr<Texture> BuildTexture(TextureBuildInfo*);

    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    const Texture& operator=(const Texture&) = delete;

private:

};
    
} // namespace renderer