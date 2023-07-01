#pragma once

#include "material.h"
#include "mesh.h"
#include "texture.h"

#include <memory>
#include <string>


namespace renderer
{
    class IRendererAssetManager
    {
    public:
        virtual std::shared_ptr<Material> GetMaterial(std::string path) = 0;
        virtual std::shared_ptr<Mesh> GetMesh(std::string path) = 0;
        virtual std::shared_ptr<Texture> GetTexture(std::string path) = 0;

        virtual void GetAvailableMeshes(std::vector<const char*>& meshPaths) = 0;
        virtual void GetAvailableMaterials(std::vector<const char*>& materialPaths) = 0;
        virtual void GetAvailableTextures(std::vector<const char*>& texturePaths) = 0;
    };
} // namespace renderer
