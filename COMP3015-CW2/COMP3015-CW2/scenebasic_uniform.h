#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/glslprogram.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>

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

    GLuint rockVAO = 0;
    GLuint rockVBO = 0;
    GLsizei rockVertexCount = 0;
    GLuint rockTexture = 0;

    GLSLProgram textProg;
    GLuint textVAO = 0;
    GLuint textVBO = 0;

    void initText();
    void renderText(float x, float y, const std::string& text, float scale, const glm::vec4& color);
    void renderUI();
    std::string getObjectiveText();

    struct RockInstance
    {
        glm::vec3 position;
        glm::vec3 scale;
        float rotationY;
    };

    std::vector<RockInstance> rockInstances;

    bool loadOBJMesh(const char* path, GLuint& vao, GLuint& vbo, GLsizei& vertexCount);
    void drawRock(GLSLProgram& shader, bool depthPass, const glm::vec3& position, const glm::vec3& scale, float rotationY);

    void generateRockField();
    bool isRockSpawnBlocked(float x, float z) const;

    GLuint reactorVAO = 0;
    GLuint reactorVBO = 0;
    GLsizei reactorVertexCount = 0;
    GLuint reactorTexture = 0;
    GLuint woodTexture = 0;

    void drawReactor(GLSLProgram& shader, bool depthPass, const glm::vec3& position, const glm::vec3& scale, float rotationY);

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

    float walkSpeed = 3.0f;
    float runSpeed = 6.0f;

    float playerEyeHeight = 1.8f;
    float verticalVelocity = 0.0f;
    float gravity = -14.0f;
    float jumpStrength = 6.0f;

    bool isGrounded = false;
    bool jumpKeyPressed = false;

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

    glm::vec3 hutPos = glm::vec3(-8.5f, 0.0f, 10.5f);

    bool doorOpen = false;
    bool doorKeyPressed = false;
    float doorAngle = 0.0f;
    float targetDoorAngle = 0.0f;
    float doorInteractRadius = 2.2f;

    float playerRadius = 0.35f;

    float getHutBaseY();
    bool isPlayerOnHutPlatform() const;
    float getPlayerGroundHeight();
    float getPathGroundHeight();

    bool isPlayerNearHutDoor() const;

    void resolveHutCollisions();
    void resolveRockCollisions();
    void resolveAABBCollision(const glm::vec3& boxCenter, const glm::vec3& boxHalfSize);
    void resolveReactorCollisions();

    float getReactorPlatformGroundHeight();

    GLuint energyCellVAO = 0;
    GLuint energyCellVBO = 0;
    GLsizei energyCellVertexCount = 0;

    bool energyCellCollected = false;
    float energyCellSpin = 0.0f;
    float energyCellInteractRadius = 1.8f;

    glm::vec3 getEnergyCellWorldPos();
    bool isPlayerNearEnergyCell();
    void drawEnergyCell(GLSLProgram& shader, bool depthPass, const glm::vec3& position, const glm::vec3& scale, float rotationY);

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