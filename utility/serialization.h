#pragma once

#include <json/json.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

void SerializeMat4(const glm::mat4& mat4, Json::Value& json);
void DeserializeMat4(glm::mat4& mat4, Json::Value& json);