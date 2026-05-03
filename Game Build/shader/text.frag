#version 430 core

out vec4 FragColor;

uniform vec4 TextColor;

void main()
{
    FragColor = TextColor;
}