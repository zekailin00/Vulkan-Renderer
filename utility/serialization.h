#pragma once

#include <json/json.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define JSON_TYPE "FileType"

enum class JsonType
{
    Scene,
    Entity,
    GltfModel,
    ObjModel,
    Material,
    Texture,
    Mesh
};

void SerializeMat4(const glm::mat4& mat4, Json::Value& json);
void DeserializeMat4(glm::mat4& mat4, Json::Value& json);

void SerializeVec2(const glm::vec2& vec2, Json::Value& json);
void DeserializeVec2(glm::vec2& vec2, Json::Value& json);

void SerializeVec3(const glm::vec3& vec3, Json::Value& json);
void DeserializeVec3(glm::vec3& vec3, Json::Value& json);

void SerializeVec4(const glm::vec4& vec4, Json::Value& json);
void DeserializeVec4(glm::vec4& vec4, Json::Value& json);