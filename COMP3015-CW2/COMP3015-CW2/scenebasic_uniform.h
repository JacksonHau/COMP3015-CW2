#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/glslprogram.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle;
    GLuint vboHandle;
    GLuint shadowFBO;
    GLuint shadowTex;
    GLuint hdrFBO = 0;
    GLuint colorBuffers[2] = { 0, 0 };
    GLuint rboDepth = 0;

    GLuint pingpongFBO[2] = { 0, 0 };
    GLuint pingpongColorbuffers[2] = { 0, 0 };

    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    GLuint terrainVAO = 0;
    GLuint terrainVBO = 0;
    GLuint terrainEBO = 0;
    GLsizei terrainIndexCount = 0;

    GLuint grassTexture = 0;

    GLuint loadTexture2D(const char* path);

    GLSLProgram blurProg;
    GLSLProgram finalProg;
    GLSLProgram prog;
    GLSLProgram depthProg;

    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 lightPos;

    float yaw;
    float pitch;
    float lastTime;
    float corePulse;

    float lastX;
    float lastY;
    bool firstMouse;
    float mouseSensitivity;

    bool bloomEnabled = true;
    bool bloomKeyPressed = false;
    float exposure = 1.0f;
    float getTerrainHeight(float x, float z);
    glm::vec3 getTerrainNormal(float x, float z);

    bool reactorActivated = false;
    bool interactKeyPressed = false;
    float interactionRadius = 2.2f;

    glm::vec3 reactorPos = glm::vec3(0.0f, 0.85f, 0.0f);

    float bloomStrength = 1.0f;
    float targetExposure = 1.0f;

    bool isPlayerNearReactor() const;

    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;

    void initBloom();
    void renderQuad();
    void compile();
    void processInput(float dt);
    void drawBox(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color);
    void renderScene(GLSLProgram& shader, bool depthPass = false);
    void initShadowMap();
    void buildTerrain();
    void drawTerrain(GLSLProgram& shader, bool depthPass = false);

public:
    SceneBasic_Uniform();

    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int w, int h) override;
};

#endif