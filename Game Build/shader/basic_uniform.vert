#version 430 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexTexCoord;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat3 NormalMatrix;
uniform mat4 LightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;
out vec2 TexCoord;

void main()
{
    vec4 worldPos = ModelMatrix * vec4(VertexPosition, 1.0);

    FragPos = worldPos.xyz;
    Normal = normalize(NormalMatrix * VertexNormal);
    FragPosLightSpace = LightSpaceMatrix * worldPos;
    TexCoord = VertexTexCoord;

    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}