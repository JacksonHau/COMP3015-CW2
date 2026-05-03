#version 430 core

layout(location = 0) in vec2 VertexPosition;

uniform mat4 ProjectionMatrix;

void main()
{
    gl_Position = ProjectionMatrix * vec4(VertexPosition, 0.0, 1.0);
}