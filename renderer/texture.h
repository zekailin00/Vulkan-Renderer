#pragma once

#include <memory>
#include <string>


namespace renderer
{


class Texture
{

public:
    static std::shared_ptr<Texture> BuildTexture();

    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    const Texture& operator=(const Texture&) = delete;

private:

};
    
} // namespace renderer