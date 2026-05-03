#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
in vec2 TexCoord;

uniform vec3 LightPos;
uniform vec3 ViewPos;
uniform vec3 ObjectColor;

uniform sampler2D ShadowMap;
uniform sampler2D DiffuseTexture;
uniform int UseTexture;

uniform sampler2D NoiseTexture;
uniform int UseNoise;
uniform float NoiseScale;
uniform float NoiseStrength;

uniform vec3 FogColor;
uniform float FogStart;
uniform float FogEnd;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.006;
    float shadowAmount = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadowAmount;
}

void main()
{
    vec3 baseColor = ObjectColor;

    if (UseTexture == 1) {
        vec3 textureColor = texture(DiffuseTexture, TexCoord).rgb;
        baseColor *= textureColor;
    }
    else if (UseTexture == 2) {
        vec2 projectedUV = FragPos.xz * 0.45;
        vec3 textureColor = texture(DiffuseTexture, projectedUV).rgb;
        baseColor *= textureColor;
    }

    if (UseNoise == 1) {
        vec2 noiseUV = FragPos.xz * NoiseScale;
        vec4 noiseSample = texture(NoiseTexture, noiseUV);

        float noiseValue = dot(noiseSample.rgb, vec3(0.3333));
        float variation = mix(1.0 - NoiseStrength, 1.0 + NoiseStrength, noiseValue);

        baseColor *= variation;
    }

    vec3 ambient = 0.35 * baseColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColor;

    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.35) * spec;

    float shadowAmount = ShadowCalculation(FragPosLightSpace);

    vec3 result = ambient + (1.0 - shadowAmount) * (diffuse + specular);

    // Fog
    float dist = length(ViewPos - FragPos);
    float fogFactor = clamp((FogEnd - dist) / (FogEnd - FogStart), 0.0, 1.0);
    result = mix(FogColor, result, fogFactor);

    FragColor = vec4(result, 1.0);

    // Bloom extraction
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}