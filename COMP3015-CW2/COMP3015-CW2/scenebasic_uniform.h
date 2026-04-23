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

    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;

    void compile();
    void processInput(float dt);
    void drawBox(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color);
    void renderScene(GLSLProgram& shader, bool depthPass = false);
    void initShadowMap();

public:
    SceneBasic_Uniform();

    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int w, int h) override;
};

#endif