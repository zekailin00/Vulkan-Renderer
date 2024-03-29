#include "gltfloader.h"

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "stb/stb_image.h"
#include "logger.h"
#include "base64.h"

#include "vulkan_node.h"
#include <tracy/Tracy.hpp>

namespace renderer
{

GltfModel GltfModel::LoadModel(std::string path)
{
    ZoneScopedN("GltfModel::LoadModel");

    Logger::Write(
        "Loading model: " + path,
        Logger::Level::Info,
        Logger::MsgType::Loader
    );

    std::ifstream jsonIn;
    jsonIn.open(path);
    jsonIn >> gltf;
    jsonIn.close();

    LoadBuffers();
    LoadTextures();
    LoadMaterials();

    int scene = gltf["scene"].isNull()? 0: gltf["scene"].asInt();

    Logger::Write(
        "Load scene: " + std::to_string(scene),
        Logger::Level::Info,
        Logger::MsgType::Loader
    );

    node = std::make_unique<VulkanNode>();

    Json::Value& rootNodes = gltf["scenes"][scene]["nodes"];
    for(int i = 0; i < rootNodes.size(); i++)
    {
        ProcessNode(*node, rootNodes[i].asInt());
    }
    
    GltfModel model{};
    return model;
}

std::unique_ptr<Node> GltfModel::GetNode()
{
    ZoneScopedN("GltfModel::GetNode");

    if (this->node == nullptr)
        throw;
    return std::move(this->node);
}

void GltfModel::LoadBuffers()
{
    ZoneScopedN("GltfModel::LoadBuffers");

    Json::Value& gltfBuffers = gltf["buffers"];
    const std::string header = "data:application/octet-stream;base64,";

    for (int i = 0; i < gltfBuffers.size(); i++)
    {
        Json::Value& gltfBuffer = gltfBuffers[i];

        if (gltfBuffer["uri"].asString().find(header) == 0)
        { // embedded
            std::string data = gltfBuffer["uri"].asString().substr(header.size());
            std::string decode = base64::base64_decode(data);
            assert(decode.size() == gltfBuffer["byteLength"].asUInt());

            std::vector<unsigned char> bin;
            bin.resize(decode.size());
            std::copy(decode.begin(), decode.end(), bin.begin());
            bufferList.push_back(bin);
        }
        else
        { // from a file
            throw; //TODO:
        }
    }
}

void GltfModel::LoadTextures()
{
    ZoneScopedN("GltfModel::LoadTextures");

    Json::Value& gltfTextures = gltf["textures"];
    Json::Value& gltfImages = gltf["images"];

    for(int i = 0; i < gltfTextures.size(); i++)
    {
        int samplerIndex = gltfTextures[i]["sampler"].asInt();
        int sourceIndex = gltfTextures[i]["source"].asInt();

        Logger::Write(
            "Loading texture: " + gltfImages[sourceIndex]["name"].asString(),
            Logger::Level::Verbose,
            Logger::MsgType::Loader
        );

        Json::Value& sampler = gltf["samplers"][samplerIndex];
        int magFilter = sampler["magFilter"].asInt();
        int minFilter = sampler["minFilter"].asInt();

        Json::Value& gltfImage = gltfImages[sourceIndex];

        if (gltfImage["uri"].isNull())
        {
            assert(gltfImage["bufferView"] && gltfImage["mimeType"]);
            int viewIndex = gltfImage["bufferView"].asInt();
            Json::Value& gltfBufferView = gltf["bufferViews"][viewIndex];

            assert(gltfBufferView["byteStride"].isNull()); // image data has no stride
            int bufIndex = gltfBufferView["buffer"].asInt();
            int bufLength = gltfBufferView["byteLength"].asInt();
            int bufOffset = gltfBufferView["byteOffset"].asInt();

            std::vector<unsigned char> &buf = bufferList[bufIndex];

            int width, height, channels;
            stbi_uc* pixels = stbi_load_from_memory(&buf[bufOffset], bufLength,
                &width, &height, &channels, STBI_rgb_alpha);

            TextureBuildInfo info{};
            info.maxFilter = (magFilter == SamplerType::NEAREST)? FILTER_NEAREST: FILTER_LINEAR;
            info.minFilter = (minFilter == SamplerType::NEAREST)? FILTER_NEAREST: FILTER_LINEAR;

            std::shared_ptr<Texture> texture = 
                VulkanTexture::BuildTextureFromBuffer(pixels, width, height, &info);

            stbi_image_free(pixels);

            textureList.push_back(std::dynamic_pointer_cast<VulkanTexture>(texture));
        }
        else // load image file
        {
            // TODO:
            throw;
        }
    }
}

void GltfModel::LoadMaterials()
{
    ZoneScopedN("GltfModel::LoadMaterials");
    
    Json::Value& gltfMaterials = gltf["materials"];

            // "name" : "Rusk_Body.003",
            // "pbrMetallicRoughness" : {
            //     "baseColorTexture" : {
            //         "index" : 2
            //     },
            //     "roughnessFactor" : 0.7763931751251221
            // }

    for (int i = 0; i < gltfMaterials.size(); i++)
    {
        Json::Value& gltfMaterial = gltfMaterials[i];

            Logger::Write(
                "Loading material: " + gltfMaterial["name"].asString(),
                Logger::Level::Verbose,
                Logger::MsgType::Loader
            );

        if(gltfMaterial["pbrMetallicRoughness"].isNull())
        {
            Logger::Write(
                "pbrMetallicRoughness not present. Use default material.",
                Logger::Level::Info,
                Logger::MsgType::Loader
            );

            materialList.push_back(
                std::dynamic_pointer_cast<VulkanMaterial>(
                VulkanMaterial::GetDefaultMaterial()));
        }
        else
        {
            Json::Value& gltfPbr = gltfMaterial["pbrMetallicRoughness"];

            MaterialProperties prop{};

            if (!gltfPbr["baseColorTexture"].isNull())
            {
                assert(gltfPbr["baseColorTexture"]["texCoord"].isNull() ||
                    gltfPbr["baseColorTexture"]["texCoord"].asInt() == 0);
                prop.albedoTexture = textureList[gltfPbr["baseColorTexture"]["index"].asInt()];
            }
            else
            {
                if(!gltfPbr["baseColorFactor"].isNull())
                {
                    float r = gltfPbr["baseColorFactor"][0].asFloat();
                    float g = gltfPbr["baseColorFactor"][1].asFloat();
                    float b = gltfPbr["baseColorFactor"][2].asFloat();

                    prop.albedo = glm::vec3(r, g, b);
                }
            }

            if (!gltfPbr["metallicRoughnessTexture"].isNull())
            {
                int texIndex = gltfPbr["metallicRoughnessTexture"]["index"].asInt();

                Json::Value& gltfTextures = gltf["textures"];
                Json::Value& gltfImages = gltf["images"];

                int samplerIndex = gltfTextures[texIndex]["sampler"].asInt();
                int sourceIndex = gltfTextures[texIndex]["source"].asInt();


                Json::Value& sampler = gltf["samplers"][samplerIndex];
                int magFilter = sampler["magFilter"].asInt();
                int minFilter = sampler["minFilter"].asInt();

                Json::Value& gltfImage = gltfImages[sourceIndex];

                if (gltfImage["uri"].isNull())
                {
                    assert(gltfImage["bufferView"] && gltfImage["mimeType"]);
                    int viewIndex = gltfImage["bufferView"].asInt();
                    Json::Value& gltfBufferView = gltf["bufferViews"][viewIndex];

                    assert(gltfBufferView["byteStride"].isNull()); // image data has no stride
                    int bufIndex = gltfBufferView["buffer"].asInt();
                    int bufLength = gltfBufferView["byteLength"].asInt();
                    int bufOffset = gltfBufferView["byteOffset"].asInt();

                    std::vector<unsigned char> &buf = bufferList[bufIndex];

                    { // green channel contains roughness values
                        int width, height, channels;
                        stbi_uc* pixels = stbi_load_from_memory(&buf[bufOffset], bufLength,
                            &width, &height, &channels, STBI_rgb_alpha);

                        for (int p = 0; p < width * height; p++)
                        {
                            stbi_uc *pixel = &pixels[4*p];
                            pixel[0] = pixel[1];
                            pixel[1] = pixel[1];
                            pixel[2] = pixel[1];
                            pixel[3] = pixel[1];
                        }

                        TextureBuildInfo info{};
                        info.maxFilter = (magFilter == SamplerType::NEAREST)? FILTER_NEAREST: FILTER_LINEAR;
                        info.minFilter = (minFilter == SamplerType::NEAREST)? FILTER_NEAREST: FILTER_LINEAR;

                        std::shared_ptr<Texture> texture = 
                            VulkanTexture::BuildTextureFromBuffer(pixels, width, height, &info);

                        stbi_image_free(pixels);

                        roughTexList.push_back(std::dynamic_pointer_cast<VulkanTexture>(texture));
                        prop.roughnessTexture = texture;
                    }

                    { // blue channel contains metalness values
                        int width, height, channels;
                        stbi_uc* pixels = stbi_load_from_memory(&buf[bufOffset], bufLength,
                            &width, &height, &channels, STBI_rgb_alpha);

                        for (int p = 0; p < width * height; p++)
                        {
                            stbi_uc *pixel = &pixels[4*p];
                            pixel[0] = pixel[2];
                            pixel[1] = pixel[2];
                            pixel[2] = pixel[2];
                            pixel[3] = pixel[2];
                        }

                        TextureBuildInfo info{};
                        info.maxFilter = (magFilter == SamplerType::NEAREST)? FILTER_NEAREST: FILTER_LINEAR;
                        info.minFilter = (minFilter == SamplerType::NEAREST)? FILTER_NEAREST: FILTER_LINEAR;

                        std::shared_ptr<Texture> texture = 
                            VulkanTexture::BuildTextureFromBuffer(pixels, width, height, &info);
                        
                        stbi_image_free(pixels);

                        metalTexList.push_back(std::dynamic_pointer_cast<VulkanTexture>(texture));
                        prop.metallicTexture = texture;
                    }
                }
                else // load image file
                {
                    // TODO:
                    throw;
                }

            }
            else
            {
                if (!gltfPbr["metallicFactor"].isNull())
                    prop.metallic = gltfPbr["metallicFactor"].asFloat();
                if (!gltfPbr["roughnessFactor"].isNull())
                    prop.roughness = glm::min(0.0f, glm::max(1.0f, gltfPbr["roughnessFactor"].asFloat()));
            }

            std::shared_ptr<Material> material = VulkanMaterial::BuildMaterial(&prop);
            materialList.push_back(std::dynamic_pointer_cast<VulkanMaterial>(material));
        }
    }
}

void GltfModel::ProcessNode(Node& parentNode, int nodeIndex)
{
    ZoneScopedN("GltfModel::ProcessNode");

    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();
    Node* nodePtr = parentNode.AddChildNode(std::move(node));

    Json::Value& gltfNode = gltf["nodes"][nodeIndex];

    Logger::Write(
        "ProcessNode: " + gltfNode["name"].asString(),
        Logger::Level::Verbose,
        Logger::MsgType::Loader
    );

    if (!gltfNode["matrix"].isNull())
    {
        glm::mat4 transform{};
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                transform[i][j] = gltfNode["matrix"][i*4 + j].asFloat();
        
        nodePtr->SetTransform(transform);
    }
    else
    {
        glm::mat4 transform = glm::mat4(1.0f);

        if (!gltfNode["translation"].isNull())
        {
            transform = glm::translate(glm::mat4(1.0f), glm::vec3(
                gltfNode["translation"][0].asFloat(),
                gltfNode["translation"][1].asFloat(),
                gltfNode["translation"][2].asFloat()
            ));
        }

        if (!gltfNode["rotation"].isNull())
        { // XYZW rotation quaternion
            glm::mat4 rotation = glm::toMat4(glm::quat(
                gltfNode["rotation"][3].asFloat(),
                gltfNode["rotation"][0].asFloat(),
                gltfNode["rotation"][1].asFloat(),
                gltfNode["rotation"][2].asFloat()
            ));

            transform = transform * rotation;
        }

        if (!gltfNode["scale"].isNull())
        {
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(
                gltfNode["scale"][0].asFloat(),
                gltfNode["scale"][1].asFloat(),
                gltfNode["scale"][2].asFloat()
            ));
            transform = transform * scale;
        }

        nodePtr->SetTransform(transform);
    }

    if (!gltfNode["mesh"].isNull())
    {
        LoadMeshes(*nodePtr, gltfNode["mesh"].asInt());
    }

    if (!gltfNode["children"].isNull())
    {
        for (int i = 0; i < gltfNode["children"].size(); i++)
            ProcessNode(*nodePtr, gltfNode["children"][i].asInt());
    }
}

void GltfModel::LoadMeshes(Node& parentNode, int meshIndex)
{
    ZoneScopedN("GltfModel::LoadMeshes");

    Json::Value& primList = gltf["meshes"][meshIndex]["primitives"];
    for(int i = 0; i < primList.size(); i++)
    {
        std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();
        Node* nodePtr = parentNode.AddChildNode(std::move(node));

        // "attributes" : {
        //     "POSITION" : 889,
        //     "TEXCOORD_0" : 890,
        //     "NORMAL" : 891,
        //     "JOINTS_0" : 892,
        //     "WEIGHTS_0" : 893
        // },
        // "indices" : 894,
        // "material" : 2,

        assert(!primList[i]["attributes"]["POSITION"].isNull());
        assert(!primList[i]["attributes"]["TEXCOORD_0"].isNull());
        assert(!primList[i]["attributes"]["NORMAL"].isNull());
        assert(!primList[i]["indices"].isNull());

        BuildMeshInfo info{};
        std::vector<Vertex>& vertexList = info.vertices;
        std::vector<unsigned int>& indexList = info.indices;

        { // vertex position
            int posIndex = primList[i]["attributes"]["POSITION"].asInt();
            Json::Value& gltfAccessor = gltf["accessors"][posIndex];

            assert(gltfAccessor["type"].asString() == "VEC3");
            assert(!gltfAccessor["bufferView"].isNull());

            int accIndex = gltfAccessor["bufferView"].asInt();
            Json::Value& gltfBufferView = gltf["bufferViews"][accIndex];

            int accOffset = gltfAccessor["byteOffset"].asInt();
            int accCount = gltfAccessor["count"].asInt();
            int bufIndex = gltfBufferView["buffer"].asInt();
            // int bufLength = gltfBufferView["byteLength"].asInt();
            int bufOffset = gltfBufferView["byteOffset"].asInt();
            int bufStride = gltfBufferView["byteStride"].asInt();

            vertexList.resize(accCount);

            std::vector<unsigned char> &buf = bufferList[bufIndex];

            if (bufStride == 0)
            {
                for (int i = 0; i < accCount; i++)
                {
                    glm::vec3* point = reinterpret_cast<glm::vec3*>(
                        &buf[bufOffset + accOffset + sizeof(glm::vec3)*i]);
                    vertexList[i].Position = *point;
                }
            }
            else
            {
                for (int i = 0; i < accCount; i++)
                {
                    glm::vec3* point = reinterpret_cast<glm::vec3*>(
                        &buf[bufOffset + accOffset + bufStride*i]);
                    vertexList[i].Position = *point;
                }   
            }
        }

        { // vertex uv
            int uvIndex = primList[i]["attributes"]["TEXCOORD_0"].asInt();
            Json::Value& gltfAccessor = gltf["accessors"][uvIndex];

            assert(gltfAccessor["type"].asString() == "VEC2");
            assert(!gltfAccessor["bufferView"].isNull());

            int accIndex = gltfAccessor["bufferView"].asInt();
            Json::Value& gltfBufferView = gltf["bufferViews"][accIndex];

            int accOffset = gltfAccessor["byteOffset"].asInt();
            int accCount = gltfAccessor["count"].asInt();
            int bufIndex = gltfBufferView["buffer"].asInt();
            // int bufLength = gltfBufferView["byteLength"].asInt();
            int bufOffset = gltfBufferView["byteOffset"].asInt();
            int bufStride = gltfBufferView["byteStride"].asInt();

            assert(vertexList.size() == accCount);

            std::vector<unsigned char> &buf = bufferList[bufIndex];

            if (bufStride == 0)
            {
                for (int i = 0; i < accCount; i++)
                {
                    glm::vec2* uv = reinterpret_cast<glm::vec2*>(
                        &buf[bufOffset + accOffset + sizeof(glm::vec2)*i]);
                    vertexList[i].TexCoords = *uv;
                }
            }
            else
            {
                for (int i = 0; i < accCount; i++)
                {
                    glm::vec2* uv = reinterpret_cast<glm::vec2*>(
                        &buf[bufOffset + accOffset + bufStride*i]);
                    vertexList[i].TexCoords = *uv;
                }   
            }
        }

        { // vertex normal
            int normIndex = primList[i]["attributes"]["NORMAL"].asInt();
            Json::Value& gltfAccessor = gltf["accessors"][normIndex];

            assert(gltfAccessor["type"].asString() == "VEC3");
            assert(!gltfAccessor["bufferView"].isNull());

            int accIndex = gltfAccessor["bufferView"].asInt();
            Json::Value& gltfBufferView = gltf["bufferViews"][accIndex];

            int accOffset = gltfAccessor["byteOffset"].asInt();
            int accCount = gltfAccessor["count"].asInt();
            int bufIndex = gltfBufferView["buffer"].asInt();
            // int bufLength = gltfBufferView["byteLength"].asInt();
            int bufOffset = gltfBufferView["byteOffset"].asInt();
            int bufStride = gltfBufferView["byteStride"].asInt();

            vertexList.resize(accCount);

            std::vector<unsigned char> &buf = bufferList[bufIndex];

            if (bufStride == 0)
            {
                for (int i = 0; i < accCount; i++)
                {
                    glm::vec3* norm = reinterpret_cast<glm::vec3*>(
                        &buf[bufOffset + accOffset + sizeof(glm::vec3)*i]);
                    vertexList[i].Normal = *norm;
                }
            }
            else
            {
                for (int i = 0; i < accCount; i++)
                {
                    glm::vec3* norm = reinterpret_cast<glm::vec3*>(
                        &buf[bufOffset + accOffset + bufStride*i]);
                    vertexList[i].Normal = *norm;
                }   
            }
        }

        { // vertex indices FIXME: type check
            int vertIndex = primList[i]["indices"].asInt();
            Json::Value& gltfAccessor = gltf["accessors"][vertIndex];

            assert(gltfAccessor["type"].asString() == "SCALAR");
            assert(!gltfAccessor["bufferView"].isNull());

            int accIndex = gltfAccessor["bufferView"].asInt();
            Json::Value& gltfBufferView = gltf["bufferViews"][accIndex];

            int accOffset = gltfAccessor["byteOffset"].asInt();
            int accCount = gltfAccessor["count"].asInt();
            int bufIndex = gltfBufferView["buffer"].asInt();
            // int bufLength = gltfBufferView["byteLength"].asInt();
            int bufOffset = gltfBufferView["byteOffset"].asInt();
            int bufStride = gltfBufferView["byteStride"].asInt();

            indexList.resize(accCount);

            std::vector<unsigned char> &buf = bufferList[bufIndex];

            if (bufStride == 0)
            {
                for (int i = 0; i < accCount; i++)
                {
                    unsigned short* norm = reinterpret_cast<unsigned short*>(
                        &buf[bufOffset + accOffset + sizeof(unsigned short)*i]);
                    indexList[i] = static_cast<unsigned int>(*norm);
                }
            }
            else
            {
                for (int i = 0; i < accCount; i++)
                {
                    unsigned int* norm = reinterpret_cast<unsigned int*>(
                        &buf[bufOffset + accOffset + bufStride*i]);
                    indexList[i] = *norm;
                }   
            }
        }

        std::shared_ptr<Mesh> mesh = VulkanMesh::BuildMesh(info);


        if (!primList[i]["material"].isNull())
        {
            mesh->AddMaterial(materialList[primList[i]["material"].asInt()]);
        }

        nodePtr->SetMesh(mesh);
        meshList.push_back(std::dynamic_pointer_cast<VulkanMesh>(mesh));
    }

}

} // namespace renderer