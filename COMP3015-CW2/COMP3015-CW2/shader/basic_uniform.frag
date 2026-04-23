#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform vec3 LightPos;
uniform vec3 ViewPos;
uniform vec3 ObjectColor;
uniform sampler2D ShadowMap;

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.005;
    float shadowAmount = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadowAmount;
}

void main()
{
    vec3 ambient = 0.35 * ObjectColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * ObjectColor;

    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.45) * spec;

    float shadowAmount = ShadowCalculation(FragPosLightSpace);

    vec3 result = ambient + (1.0 - shadowAmount) * (diffuse + specular);
    FragColor = vec4(result, 1.0);
}