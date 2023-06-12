#include "serialization.h"


void SerializeMat4(const glm::mat4& mat4, Json::Value& json)
{
    json[0]  = mat4[0][0];
    json[1]  = mat4[0][1];
    json[2]  = mat4[0][2];
    json[3]  = mat4[0][3];
    json[4]  = mat4[1][0];
    json[5]  = mat4[1][1];
    json[6]  = mat4[1][2];
    json[7]  = mat4[1][3];
    json[8]  = mat4[2][0];
    json[9]  = mat4[2][1];
    json[10] = mat4[2][2];
    json[11] = mat4[2][3];
    json[12] = mat4[3][0];
    json[13] = mat4[3][1];
    json[14] = mat4[3][2];
    json[15] = mat4[3][3];
}

void DeserializeMat4(glm::mat4& mat4, Json::Value& json)
{
    mat4[0][0] = json[0].asFloat();
    mat4[0][1] = json[1].asFloat();
    mat4[0][2] = json[2].asFloat();
    mat4[0][3] = json[3].asFloat();

    mat4[1][0] = json[4].asFloat();
    mat4[1][1] = json[5].asFloat();
    mat4[1][2] = json[6].asFloat();
    mat4[1][3] = json[7].asFloat();

    mat4[2][0] = json[8].asFloat();
    mat4[2][1] = json[9].asFloat();
    mat4[2][2] = json[10].asFloat();
    mat4[2][3] = json[11].asFloat();

    mat4[3][0] = json[12].asFloat();
    mat4[3][1] = json[13].asFloat();
    mat4[3][2] = json[14].asFloat();
    mat4[3][3] = json[15].asFloat();
}

void SerializeVec2(const glm::vec2& vec2, Json::Value& json)
{
    json[0] = vec2[0];
    json[1] = vec2[1];
}

void DeserializeVec2(glm::vec2& vec2, Json::Value& json)
{
    vec2[0] = json[0].asFloat();
    vec2[1] = json[1].asFloat();
}

void SerializeVec3(const glm::vec3& vec3, Json::Value& json)
{
    json[0] = vec3[0];
    json[1] = vec3[1];
    json[2] = vec3[2];
}

void DeserializeVec3(glm::vec3& vec3, Json::Value& json)
{
    vec3[0] = json[0].asFloat();
    vec3[1] = json[1].asFloat();
    vec3[2] = json[2].asFloat();
}

void SerializeVec4(const glm::vec4& vec4, Json::Value& json)
{
    json[0] = vec4[0];
    json[1] = vec4[1];
    json[2] = vec4[2];
    json[3] = vec4[3];
}

void DeserializeVec4(glm::vec4& vec4, Json::Value& json)
{
    vec4[0] = json[0].asFloat();
    vec4[1] = json[1].asFloat();
    vec4[2] = json[2].asFloat();
    vec4[3] = json[3].asFloat();
}