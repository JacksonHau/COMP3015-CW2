#include "scenebasic_uniform.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <GLFW/glfw3.h>
#include "helper/glutils.h"
#include "helper/noisetex.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "helper/stb/stb_image.h"

#define STB_EASY_FONT_IMPLEMENTATION
#include "helper/stb/stb_easy_font.h"

using glm::vec3;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform()
    : vaoHandle(0),
    vboHandle(0),
    shadowFBO(0),
    shadowTex(0),
    cameraPos(0.0f, 2.0f, 18.0f),
    cameraFront(0.0f, 0.0f, -1.0f),
    cameraUp(0.0f, 1.0f, 0.0f),
    lightPos(0.8f, 4.5f, 1.2f),
    yaw(-90.0f),
    pitch(0.0f),
    lastTime(0.0f),
    corePulse(0.0f),
    lastX(0.0f),
    lastY(0.0f),
    firstMouse(true),
    mouseSensitivity(0.1f)
{
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();

        depthProg.compileShader("shader/shadow_depth.vert");
        depthProg.compileShader("shader/shadow_depth.frag");
        depthProg.link();

        blurProg.compileShader("shader/quad.vert");
        blurProg.compileShader("shader/bloom_blur.frag");
        blurProg.link();

        finalProg.compileShader("shader/quad.vert");
        finalProg.compileShader("shader/bloom_final.frag");
        finalProg.link();

        textProg.compileShader("shader/text.vert");
        textProg.compileShader("shader/text.frag");
        textProg.link();

        prog.use();
        prog.setUniform("ShadowMap", 0);
        prog.setUniform("DiffuseTexture", 1);
        prog.setUniform("NoiseTexture", 2);
        prog.setUniform("UseTexture", 0);
        prog.setUniform("UseNoise", 0);
        prog.setUniform("NoiseScale", 0.08f);
        prog.setUniform("NoiseStrength", 0.25f);

        blurProg.use();
        blurProg.setUniform("image", 0);

        finalProg.use();
        finalProg.setUniform("scene", 0);
        finalProg.setUniform("bloomBlur", 1);
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::initText()
{
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec2),
        (void*)0
    );

    glBindVertexArray(0);
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.18f, 0.22f, 0.28f, 1.0f);

    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        lastX = static_cast<float>(x);
        lastY = static_cast<float>(y);
    }

    // Cube vertex data: position + normal
    float cubeVertices[] = {
        // back face
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,

        // front face
        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,

        // left face
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        // right face
         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,

         // bottom face
         -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
          0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
          0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
          0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
         -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
         -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,

         // top face
         -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
          0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
          0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
          0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
         -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
         -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glGenBuffers(1, &vboHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    initShadowMap();
    initBloom();
    initText();

    buildTerrain();
    grassTexture = loadTexture2D("assets/textures/grass.png");
    noiseTexture = NoiseTex::generatePeriodic2DTex(8.0f, 0.55f, 256, 256);

    woodTexture = loadTexture2D("assets/textures/wood.jpg");

    rockTexture = loadTexture2D("assets/textures/rock.png");
    loadOBJMesh("assets/models/rock.obj", rockVAO, rockVBO, rockVertexCount);
    generateRockField();

    reactorTexture = loadTexture2D("assets/textures/reactor_basecolor.png");
    loadOBJMesh("assets/models/reactor.obj", reactorVAO, reactorVBO, reactorVertexCount);

    loadOBJMesh("assets/models/energy_cell.obj", energyCellVAO, energyCellVBO, energyCellVertexCount);

    prog.use();
    prog.setUniform("LightPos", lightPos);
    prog.setUniform("ShadowMap", 0);
}

std::string SceneBasic_Uniform::getObjectiveText()
{
    if (reactorActivated) {
        return "Objective Complete: Reactor Activated";
    }

    if (!energyCellCollected && isPlayerNearEnergyCell()) {
        return "Press E to Collect Energy Cell";
    }

    if (!energyCellCollected && isPlayerNearHutDoor()) {
        return "Press E to Open Hut Door";
    }

    if (!energyCellCollected) {
        return "Objective: Find the Energy Cell in the Hut";
    }

    if (isPlayerNearReactor()) {
        return "Press E to Activate Reactor";
    }

    return "Objective: Return to the Reactor";
}

void SceneBasic_Uniform::renderText(float x, float y, const std::string& text, float scale, const glm::vec4& color)
{
    if (text.empty() || textVAO == 0 || textVBO == 0) {
        return;
    }

    struct StbVertex
    {
        float x;
        float y;
        float z;
        unsigned char color[4];
    };

    char buffer[99999];

    int quadCount = stb_easy_font_print(
        0.0f,
        0.0f,
        const_cast<char*>(text.c_str()),
        nullptr,
        buffer,
        sizeof(buffer)
    );

    if (quadCount <= 0) {
        return;
    }

    StbVertex* stbVertices = reinterpret_cast<StbVertex*>(buffer);

    std::vector<glm::vec2> triangleVertices;
    triangleVertices.reserve(quadCount * 6);

    auto addVertex = [&](const StbVertex& v)
        {
            triangleVertices.push_back(glm::vec2(
                x + v.x * scale,
                y + v.y * scale
            ));
        };

    for (int i = 0; i < quadCount; i++) {
        StbVertex& v0 = stbVertices[i * 4 + 0];
        StbVertex& v1 = stbVertices[i * 4 + 1];
        StbVertex& v2 = stbVertices[i * 4 + 2];
        StbVertex& v3 = stbVertices[i * 4 + 3];

        addVertex(v0);
        addVertex(v1);
        addVertex(v2);

        addVertex(v0);
        addVertex(v2);
        addVertex(v3);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textProg.use();

    glm::mat4 textProjection = glm::ortho(
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height),
        0.0f
    );

    textProg.setUniform("ProjectionMatrix", textProjection);
    textProg.setUniform("TextColor", color);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        triangleVertices.size() * sizeof(glm::vec2),
        triangleVertices.data(),
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(triangleVertices.size()));

    glBindVertexArray(0);

    glDisable(GL_BLEND);
}

void SceneBasic_Uniform::renderUI()
{
    std::string objectiveText = getObjectiveText();

    // Black shadow/background text
    renderText(
        24.0f,
        28.0f,
        objectiveText,
        2.0f,
        glm::vec4(0.0f, 0.0f, 0.0f, 0.85f)
    );

    // Main white text
    renderText(
        22.0f,
        26.0f,
        objectiveText,
        2.0f,
        glm::vec4(1.0f, 1.0f, 0.85f, 1.0f)
    );

    std::string controls = "WASD Move | Shift Run | Space Jump | E Interact | B Bloom";

    renderText(
        22.0f,
        static_cast<float>(height) - 42.0f,
        controls,
        1.35f,
        glm::vec4(0.0f, 0.0f, 0.0f, 0.85f)
    );

    renderText(
        20.0f,
        static_cast<float>(height) - 44.0f,
        controls,
        1.35f,
        glm::vec4(0.85f, 0.9f, 1.0f, 1.0f)
    );
}

void SceneBasic_Uniform::processInput(float dt)
{
    if (!window) return;

    bool running =
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    float currentSpeed = running ? runSpeed : walkSpeed;
    float moveSpeed = currentSpeed * dt;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    glm::vec3 moveFront = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 right = glm::normalize(glm::cross(moveFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += moveFront * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= moveFront * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= right * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += right * moveSpeed;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !jumpKeyPressed) {
        if (isGrounded) {
            verticalVelocity = jumpStrength;
            isGrounded = false;
        }

        jumpKeyPressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        jumpKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bloomKeyPressed) {
        bloomEnabled = !bloomEnabled;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !interactKeyPressed) {

        // Collect Energy Cell
        if (!energyCellCollected && isPlayerNearEnergyCell()) {
            energyCellCollected = true;
            targetExposure = 1.15f;
        }

        // Open/close hut door
        else if (isPlayerNearHutDoor()) {
            doorOpen = !doorOpen;
            targetDoorAngle = doorOpen ? -75.0f : 0.0f;
        }

        // Activate reactor only after collecting Energy Cell
        else if (isPlayerNearReactor() && !reactorActivated) {
            if (energyCellCollected) {
                reactorActivated = true;
                targetExposure = 1.4f;
            }
        }

        interactKeyPressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
        interactKeyPressed = false;
    }

    resolveHutCollisions();
    resolveRockCollisions();
    resolveReactorCollisions();

    // Keep player inside the designed map area
    cameraPos.x = glm::clamp(cameraPos.x, -35.0f, 35.0f);
    cameraPos.z = glm::clamp(cameraPos.z, -35.0f, 35.0f);

    // Gravity and jumping
    verticalVelocity += gravity * dt;
    cameraPos.y += verticalVelocity * dt;

    // Player stands on the terrain height plus eye height
    float groundY = getPlayerGroundHeight() + playerEyeHeight;

    if (cameraPos.y <= groundY) {
        cameraPos.y = groundY;
        verticalVelocity = 0.0f;
        isGrounded = true;
    }
}

void SceneBasic_Uniform::update(float t)
{
    float dt = t - lastTime;
    lastTime = t;

    processInput(dt);

    exposure = glm::mix(exposure, targetExposure, 0.05f);

    doorAngle = glm::mix(doorAngle, targetDoorAngle, 0.08f);
    energyCellSpin = t * 90.0f;

    if (animating()) {
        float pulseSpeed = reactorActivated ? 5.0f : 2.0f;
        corePulse = sin(t * pulseSpeed) * 0.5f + 0.5f;
    }

    if (window) {
        glfwSetWindowTitle(window, "SceneCW2Demo");
    }
}

void SceneBasic_Uniform::drawBox(const vec3& position, const vec3& scale, const vec3& color)
{
    model = mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);

    mat3 normalMatrix = mat3(glm::transpose(glm::inverse(model)));

    prog.setUniform("ModelMatrix", model);
    prog.setUniform("NormalMatrix", normalMatrix);
    prog.setUniform("ObjectColor", color);

    glBindVertexArray(vaoHandle);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::render()
{
    glm::vec3 currentLightPos = reactorActivated
        ? glm::vec3(0.8f, 5.2f, 1.2f)
        : lightPos;

    mat4 lightProjection = glm::ortho(-18.0f, 18.0f, -18.0f, 18.0f, 1.0f, 35.0f);
    mat4 lightView = glm::lookAt(currentLightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mat4 lightSpaceMatrix = lightProjection * lightView;

    glEnable(GL_DEPTH_TEST);

    // Shadow depth
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    depthProg.use();
    depthProg.setUniform("LightSpaceMatrix", lightSpaceMatrix);
    renderScene(depthProg, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Scene into HDR framebuffer
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.use();
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    prog.setUniform("ViewMatrix", view);
    prog.setUniform("ProjectionMatrix", projection);
    prog.setUniform("ViewPos", cameraPos);
    prog.setUniform("LightPos", currentLightPos);
    prog.setUniform("LightSpaceMatrix", lightSpaceMatrix);
    prog.setUniform("FogColor", glm::vec3(0.18f, 0.22f, 0.28f));
    prog.setUniform("FogStart", 22.0f);
    prog.setUniform("FogEnd", 70.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowTex);

    renderScene(prog, false);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blur bright buffer
    glDisable(GL_DEPTH_TEST);

    bool horizontal = true;
    bool firstIteration = true;
    const int blurAmount = 10;

    blurProg.use();

    for (int i = 0; i < blurAmount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        blurProg.setUniform("horizontal", horizontal ? 1 : 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firstIteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);

        renderQuad();

        horizontal = !horizontal;
        if (firstIteration) firstIteration = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Final composite
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    finalProg.use();
    finalProg.setUniform("bloom", bloomEnabled ? 1 : 0);
    finalProg.setUniform("exposure", exposure);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

    renderQuad();
    renderUI();
}

void SceneBasic_Uniform::initShadowMap()
{
    glGenFramebuffers(1, &shadowFBO);

    glGenTextures(1, &shadowTex);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::initBloom()
{
    if (window) {
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        width = fbw;
        height = fbh;
    }

    if (width <= 0 || height <= 0) return;

    if (hdrFBO != 0) {
        glDeleteFramebuffers(1, &hdrFBO);
        glDeleteTextures(2, colorBuffers);
        glDeleteRenderbuffers(1, &rboDepth);
        glDeleteFramebuffers(2, pingpongFBO);
        glDeleteTextures(2, pingpongColorbuffers);
    }

    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);

    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
    }

    if (quadVAO == 0) {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }
}

void SceneBasic_Uniform::renderQuad()
{
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

float SceneBasic_Uniform::getTerrainHeight(float x, float z)
{
    // Broad rolling land shape
    float largeShape =
        1.25f * sin(x * 0.055f) * cos(z * 0.045f);

    // Medium terrain variation
    float mediumDetail =
        0.45f * sin((x + z) * 0.16f) +
        0.35f * cos((x - z) * 0.12f);

    // Small surface breakup
    float smallDetail =
        0.12f * sin(x * 0.55f) * cos(z * 0.48f);

    float rawHeight = largeShape + mediumDetail + smallDetail;

    // Flat zone around reactor so the objective area stays playable
    float coreDistance = glm::length(glm::vec2(x, z));
    float flattenCore = glm::clamp((coreDistance - 3.0f) / 7.0f, 0.0f, 1.0f);

    // Flatten a rough path from z = 18 down to reactor at z = 0
    float pathWidth = 2.2f;
    float pathZMask = glm::clamp((z + 1.5f) / 18.0f, 0.0f, 1.0f);
    float pathXMask = glm::clamp(abs(x) / pathWidth, 0.0f, 1.0f);
    float flattenPath = glm::clamp(pathXMask + (1.0f - pathZMask), 0.0f, 1.0f);

    float flattenAmount = glm::min(flattenCore, flattenPath);

    return -0.85f + rawHeight * flattenAmount;
}

GLuint SceneBasic_Uniform::loadTexture2D(const char* path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) {
        format = GL_RED;
    }
    else if (channels == 3) {
        format = GL_RGB;
    }
    else if (channels == 4) {
        format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    // Repeat makes the grass tile across the terrain
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

glm::vec3 SceneBasic_Uniform::getTerrainNormal(float x, float z)
{
    const float eps = 0.1f;

    float hL = getTerrainHeight(x - eps, z);
    float hR = getTerrainHeight(x + eps, z);
    float hD = getTerrainHeight(x, z - eps);
    float hU = getTerrainHeight(x, z + eps);

    glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f * eps, hD - hU));
    return normal;
}

void SceneBasic_Uniform::buildTerrain()
{
    const int gridX = 180;
    const int gridZ = 180;
    const float worldSizeX = 80.0f;
    const float worldSizeZ = 80.0f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve((gridX + 1) * (gridZ + 1) * 6);
    indices.reserve(gridX * gridZ * 6);

    for (int z = 0; z <= gridZ; z++) {
        for (int x = 0; x <= gridX; x++) {
            float xPos = ((float)x / gridX - 0.5f) * worldSizeX;
            float zPos = ((float)z / gridZ - 0.5f) * worldSizeZ;
            float yPos = getTerrainHeight(xPos, zPos);

            glm::vec3 n = getTerrainNormal(xPos, zPos);

            float texScale = 0.35f;
            float u = xPos * texScale;
            float v = zPos * texScale;

            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);

            vertices.push_back(n.x);
            vertices.push_back(n.y);
            vertices.push_back(n.z);

            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (int z = 0; z < gridZ; z++) {
        for (int x = 0; x < gridX; x++) {
            int start = z * (gridX + 1) + x;

            indices.push_back(start);
            indices.push_back(start + gridX + 1);
            indices.push_back(start + 1);

            indices.push_back(start + 1);
            indices.push_back(start + gridX + 1);
            indices.push_back(start + gridX + 2);
        }
    }

    terrainIndexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);

    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void SceneBasic_Uniform::drawTerrain(GLSLProgram& shader, bool depthPass)
{
    model = glm::mat4(1.0f);

    shader.setUniform("ModelMatrix", model);

    if (!depthPass) {
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        shader.setUniform("NormalMatrix", normalMatrix);

        shader.setUniform("ObjectColor", glm::vec3(0.45f, 0.55f, 0.38f));
        shader.setUniform("UseTexture", 1);
        shader.setUniform("UseNoise", 1);
        shader.setUniform("NoiseScale", 0.08f);
        shader.setUniform("NoiseStrength", 0.25f);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grassTexture);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
    }

    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool SceneBasic_Uniform::isPlayerNearReactor() const
{
    glm::vec2 playerXZ(cameraPos.x, cameraPos.z);
    glm::vec2 reactorXZ(reactorPos.x, reactorPos.z);

    float dist = glm::length(playerXZ - reactorXZ);
    return dist <= interactionRadius;
}

bool SceneBasic_Uniform::isPlayerNearHutDoor() const
{
    glm::vec3 doorPos = hutPos + glm::vec3(0.0f, 0.0f, 2.4f);

    glm::vec2 playerXZ(cameraPos.x, cameraPos.z);
    glm::vec2 doorXZ(doorPos.x, doorPos.z);

    float dist = glm::length(playerXZ - doorXZ);
    return dist <= 3.0f;
}

bool SceneBasic_Uniform::loadOBJMesh(const char* path, GLuint& vao, GLuint& vbo, GLsizei& vertexCount)
{
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ: " << path << std::endl;
        return false;
    }

    struct ObjIndex {
        int v = 0;
        int vt = 0;
        int vn = 0;
    };

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<float> finalVertices;

    auto fixIndex = [](int index, int size) -> int {
        if (index > 0) return index - 1;
        if (index < 0) return size + index;
        return 0;
        };

    auto parseFaceToken = [](const std::string& token) -> ObjIndex {
        ObjIndex result;

        size_t firstSlash = token.find('/');

        if (firstSlash == std::string::npos) {
            result.v = std::stoi(token);
            return result;
        }

        result.v = std::stoi(token.substr(0, firstSlash));

        size_t secondSlash = token.find('/', firstSlash + 1);

        if (secondSlash == std::string::npos) {
            std::string vtText = token.substr(firstSlash + 1);
            if (!vtText.empty()) result.vt = std::stoi(vtText);
        }
        else {
            std::string vtText = token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
            std::string vnText = token.substr(secondSlash + 1);

            if (!vtText.empty()) result.vt = std::stoi(vtText);
            if (!vnText.empty()) result.vn = std::stoi(vnText);
        }

        return result;
        };

    auto addVertex = [&](const ObjIndex& index)
        {
            glm::vec3 pos = positions[fixIndex(index.v, static_cast<int>(positions.size()))];

            glm::vec2 uv(0.0f);
            if (index.vt != 0 && !texCoords.empty()) {
                uv = texCoords[fixIndex(index.vt, static_cast<int>(texCoords.size()))];
            }

            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            if (index.vn != 0 && !normals.empty()) {
                normal = normals[fixIndex(index.vn, static_cast<int>(normals.size()))];
            }

            finalVertices.push_back(pos.x);
            finalVertices.push_back(pos.y);
            finalVertices.push_back(pos.z);

            finalVertices.push_back(normal.x);
            finalVertices.push_back(normal.y);
            finalVertices.push_back(normal.z);

            finalVertices.push_back(uv.x);
            finalVertices.push_back(uv.y);
        };

    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (type == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            texCoords.push_back(uv);
        }
        else if (type == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "f") {
            std::vector<ObjIndex> face;
            std::string token;

            while (ss >> token) {
                face.push_back(parseFaceToken(token));
            }

            // Triangulate using fan method
            for (size_t i = 1; i + 1 < face.size(); i++) {
                addVertex(face[0]);
                addVertex(face[i]);
                addVertex(face[i + 1]);
            }
        }
    }

    if (finalVertices.empty()) {
        std::cerr << "OBJ loaded but had no drawable vertices: " << path << std::endl;
        return false;
    }

    vertexCount = static_cast<GLsizei>(finalVertices.size() / 8);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, finalVertices.size() * sizeof(float), finalVertices.data(), GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // texcoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    std::cout << "Loaded OBJ: " << path << " with " << vertexCount << " vertices" << std::endl;

    return true;
}

void SceneBasic_Uniform::drawRock(GLSLProgram& shader, bool depthPass, const glm::vec3& position, const glm::vec3& scale, float rotationY)
{
    if (rockVAO == 0 || rockVertexCount <= 0) return;

    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale);

    shader.setUniform("ModelMatrix", model);

    if (!depthPass) {
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

        shader.setUniform("NormalMatrix", normalMatrix);
        shader.setUniform("ObjectColor", glm::vec3(0.65f, 0.62f, 0.56f));
        shader.setUniform("UseTexture", 1);
        shader.setUniform("UseNoise", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rockTexture);
    }

    glBindVertexArray(rockVAO);
    glDrawArrays(GL_TRIANGLES, 0, rockVertexCount);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::resolveAABBCollision(const glm::vec3& boxCenter, const glm::vec3& boxHalfSize)
{
    glm::vec2 playerPos(cameraPos.x, cameraPos.z);
    glm::vec2 boxPos(boxCenter.x, boxCenter.z);
    glm::vec2 halfSize(boxHalfSize.x, boxHalfSize.z);

    glm::vec2 closestPoint;
    closestPoint.x = glm::clamp(playerPos.x, boxPos.x - halfSize.x, boxPos.x + halfSize.x);
    closestPoint.y = glm::clamp(playerPos.y, boxPos.y - halfSize.y, boxPos.y + halfSize.y);

    glm::vec2 difference = playerPos - closestPoint;
    float distance = glm::length(difference);

    if (distance < playerRadius && distance > 0.0001f) {
        glm::vec2 pushDir = glm::normalize(difference);
        glm::vec2 correctedPos = closestPoint + pushDir * playerRadius;

        cameraPos.x = correctedPos.x;
        cameraPos.z = correctedPos.y;
    }
    else if (distance <= 0.0001f) {
        cameraPos.z += playerRadius;
    }
}

float SceneBasic_Uniform::getHutBaseY()
{
    // Matches the hut base footprint.
    const float halfWidth = 3.1f;
    const float halfDepth = 2.7f;

    float x = hutPos.x;
    float z = hutPos.z;

    float h0 = getTerrainHeight(x, z);
    float h1 = getTerrainHeight(x - halfWidth, z - halfDepth);
    float h2 = getTerrainHeight(x + halfWidth, z - halfDepth);
    float h3 = getTerrainHeight(x - halfWidth, z + halfDepth);
    float h4 = getTerrainHeight(x + halfWidth, z + halfDepth);

    float highest = glm::max(h0, glm::max(glm::max(h1, h2), glm::max(h3, h4)));

    // Small lift so the hut sits on top of the grass.
    return highest + 0.08f;
}

bool SceneBasic_Uniform::isPlayerOnHutPlatform() const
{
    // Slightly smaller than the visible platform so stepping on/off feels natural.
    const float halfWidth = 3.0f;
    const float halfDepth = 2.55f;

    return cameraPos.x > hutPos.x - halfWidth &&
        cameraPos.x < hutPos.x + halfWidth &&
        cameraPos.z > hutPos.z - halfDepth &&
        cameraPos.z < hutPos.z + halfDepth;
}

float SceneBasic_Uniform::getPlayerGroundHeight()
{
    float terrainY = getTerrainHeight(cameraPos.x, cameraPos.z);
    float finalGroundY = terrainY;

    // Hut platform ground
    if (isPlayerOnHutPlatform()) {
        float hutPlatformTopY = getHutBaseY() + 0.36f;
        finalGroundY = glm::max(finalGroundY, hutPlatformTopY);
    }

    // Path slab ground
    float pathY = getPathGroundHeight();
    if (pathY > -999.0f) {
        finalGroundY = glm::max(finalGroundY, pathY);
    }

    // Reactor platform ground
    float reactorY = getReactorPlatformGroundHeight();
    if (reactorY > -999.0f) {
        finalGroundY = glm::max(finalGroundY, reactorY);
    }

    return finalGroundY;
}

float SceneBasic_Uniform::getPathGroundHeight()
{
    struct PathSlab
    {
        float x;
        float z;
        glm::vec3 scale;
    };

    // These must match your path slabs in renderScene()
    PathSlab slabs[] = {
        {  0.0f, 15.0f, glm::vec3(2.1f, 0.14f, 1.1f) },
        { -0.4f, 12.8f, glm::vec3(1.6f, 0.12f, 0.9f) },
        {  0.5f, 10.6f, glm::vec3(1.9f, 0.12f, 1.0f) },
        { -0.2f,  8.4f, glm::vec3(1.5f, 0.12f, 0.9f) },
        {  0.4f,  6.2f, glm::vec3(1.8f, 0.12f, 0.9f) },
        { -0.3f,  4.0f, glm::vec3(1.4f, 0.12f, 0.8f) },
        {  0.2f,  2.1f, glm::vec3(1.5f, 0.12f, 0.8f) }
    };

    float highestPathY = -9999.0f;

    for (const PathSlab& slab : slabs) {
        float halfX = slab.scale.x * 0.5f;
        float halfZ = slab.scale.z * 0.5f;

        // Small tolerance so the player does not jitter at the slab edges
        float edgeTolerance = playerRadius * 0.5f;

        bool onSlab =
            cameraPos.x >= slab.x - halfX - edgeTolerance &&
            cameraPos.x <= slab.x + halfX + edgeTolerance &&
            cameraPos.z >= slab.z - halfZ - edgeTolerance &&
            cameraPos.z <= slab.z + halfZ + edgeTolerance;

        if (onSlab) {
            // Path slab is drawn at terrain height + half scale,
            // so its top surface is terrain height + full scale.y
            float slabTopY = getTerrainHeight(slab.x, slab.z) + slab.scale.y;
            highestPathY = glm::max(highestPathY, slabTopY);
        }
    }

    return highestPathY;
}

void SceneBasic_Uniform::resolveHutCollisions()
{
    float hutY = getHutBaseY();

    // Hut dimensions must match your current draw code.
    float frontZ = hutPos.z + 2.35f;
    float backZ = hutPos.z - 2.35f;

    // Left wall
    resolveAABBCollision(
        glm::vec3(hutPos.x - 2.7f, hutY + 2.0f, hutPos.z),
        glm::vec3(0.22f * 0.5f, 2.0f, 4.7f * 0.5f)
    );

    // Right wall
    resolveAABBCollision(
        glm::vec3(hutPos.x + 2.7f, hutY + 2.0f, hutPos.z),
        glm::vec3(0.22f * 0.5f, 2.0f, 4.7f * 0.5f)
    );

    // Back wall
    resolveAABBCollision(
        glm::vec3(hutPos.x, hutY + 2.0f, backZ),
        glm::vec3(5.4f * 0.5f, 2.0f, 0.22f * 0.5f)
    );

    // Front wall pieces
    resolveAABBCollision(
        glm::vec3(hutPos.x - 1.75f, hutY + 2.0f, frontZ),
        glm::vec3(1.45f * 0.5f, 2.0f, 0.22f * 0.5f)
    );

    resolveAABBCollision(
        glm::vec3(hutPos.x + 1.75f, hutY + 2.0f, frontZ),
        glm::vec3(1.45f * 0.5f, 2.0f, 0.22f * 0.5f)
    );

    // Closed door blocks the doorway.
    // When the door is open enough, the player can enter.
    if (!doorOpen) {
        resolveAABBCollision(
            glm::vec3(hutPos.x, hutY + 1.6f, hutPos.z + 2.40f),
            glm::vec3(0.75f, 2.0f, 0.35f)
        );
    }
}

void SceneBasic_Uniform::resolveRockCollisions()
{
    glm::vec2 playerXZ(cameraPos.x, cameraPos.z);

    for (const RockInstance& rock : rockInstances) {
        glm::vec2 rockXZ(rock.position.x, rock.position.z);

        float rockRadius = glm::max(rock.scale.x, rock.scale.z) * 1.45f;
        float minDistance = playerRadius + rockRadius;

        glm::vec2 difference = playerXZ - rockXZ;
        float distance = glm::length(difference);

        if (distance < minDistance && distance > 0.0001f) {
            glm::vec2 pushDir = glm::normalize(difference);
            glm::vec2 correctedPos = rockXZ + pushDir * minDistance;

            cameraPos.x = correctedPos.x;
            cameraPos.z = correctedPos.y;

            playerXZ = correctedPos;
        }
        else if (distance <= 0.0001f) {
            cameraPos.x += minDistance;
            playerXZ.x = cameraPos.x;
        }
    }
}

void SceneBasic_Uniform::resolveReactorCollisions()
{
    float reactorGroundY = getTerrainHeight(0.0f, 0.0f);

    float baseHeight = 0.95f;
    float topHeight = 0.22f;

    float baseTopY = reactorGroundY + baseHeight;
    float topTopY = baseTopY + topHeight;

    float playerFeetY = cameraPos.y - playerEyeHeight;

    auto resolvePlatformSide = [&](const glm::vec3& boxCenter, const glm::vec3& boxHalfSize, float topY)
        {
            // If the player is already standing on top, do not push them sideways.
            if (playerFeetY >= topY - 0.12f) {
                return;
            }

            resolveAABBCollision(boxCenter, boxHalfSize);
        };

    // Big bottom reactor base side collision
    resolvePlatformSide(
        glm::vec3(0.0f, reactorGroundY + baseHeight * 0.5f, 0.0f),
        glm::vec3(1.6f, baseHeight * 0.5f, 1.6f),
        baseTopY
    );

    // Smaller top platform side collision
    resolvePlatformSide(
        glm::vec3(0.0f, baseTopY + topHeight * 0.5f, 0.0f),
        glm::vec3(0.95f, topHeight * 0.5f, 0.95f),
        topTopY
    );

    // Reactor model collision on top
    if (playerFeetY >= topTopY - 0.15f) {
        glm::vec2 playerXZ(cameraPos.x, cameraPos.z);
        glm::vec2 reactorXZ(0.0f, 0.0f);

        float reactorRadius = 0.55f;
        float minDistance = playerRadius + reactorRadius;

        glm::vec2 difference = playerXZ - reactorXZ;
        float distance = glm::length(difference);

        if (distance < minDistance && distance > 0.0001f) {
            glm::vec2 pushDir = glm::normalize(difference);
            glm::vec2 correctedPos = reactorXZ + pushDir * minDistance;

            cameraPos.x = correctedPos.x;
            cameraPos.z = correctedPos.y;
        }
    }
}

void SceneBasic_Uniform::drawReactor(GLSLProgram& shader, bool depthPass, const glm::vec3& position, const glm::vec3& scale, float rotationY)
{
    if (reactorVAO == 0 || reactorVertexCount <= 0) return;

    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale);

    shader.setUniform("ModelMatrix", model);

    if (!depthPass) {
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

        glm::vec3 idleColor = glm::mix(
            glm::vec3(1.2f, 1.8f, 3.0f),
            glm::vec3(1.8f, 2.4f, 4.0f),
            corePulse
        );

        glm::vec3 activeColor = glm::mix(
            glm::vec3(2.5f, 2.8f, 4.5f),
            glm::vec3(3.8f, 4.2f, 6.0f),
            corePulse
        );

        glm::vec3 reactorColor = reactorActivated ? activeColor : idleColor;

        shader.setUniform("NormalMatrix", normalMatrix);

        shader.setUniform("ObjectColor", reactorColor);
        shader.setUniform("UseTexture", 1);
        shader.setUniform("UseNoise", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, reactorTexture);
    }

    glBindVertexArray(reactorVAO);
    glDrawArrays(GL_TRIANGLES, 0, reactorVertexCount);
    glBindVertexArray(0);
}

float SceneBasic_Uniform::getReactorPlatformGroundHeight()
{
    float reactorGroundY = getTerrainHeight(0.0f, 0.0f);

    float baseHeight = 0.95f;
    float topHeight = 0.22f;

    float baseTopY = reactorGroundY + baseHeight;
    float topTopY = baseTopY + topHeight;

    glm::vec2 playerXZ(cameraPos.x, cameraPos.z);

    // Big bottom base footprint
    bool onBase =
        playerXZ.x >= -1.6f && playerXZ.x <= 1.6f &&
        playerXZ.y >= -1.6f && playerXZ.y <= 1.6f;

    // Smaller top platform footprint
    bool onTop =
        playerXZ.x >= -0.95f && playerXZ.x <= 0.95f &&
        playerXZ.y >= -0.95f && playerXZ.y <= 0.95f;

    if (onTop) {
        return topTopY;
    }

    if (onBase) {
        return baseTopY;
    }

    return -9999.0f;
}

bool SceneBasic_Uniform::isRockSpawnBlocked(float x, float z) const
{
    float reactorDist = glm::length(glm::vec2(x, z) - glm::vec2(0.0f, 0.0f));
    if (reactorDist < 6.5f) {
        return true;
    }

    bool onPath =
        std::abs(x) < 2.8f &&
        z > 1.0f &&
        z < 18.5f;

    if (onPath) {
        return true;
    }

    bool nearHut =
        std::abs(x - hutPos.x) < 5.8f &&
        std::abs(z - hutPos.z) < 5.0f;

    if (nearHut) {
        return true;
    }

    float spawnDist = glm::length(glm::vec2(x, z) - glm::vec2(0.0f, 18.0f));
    if (spawnDist < 4.5f) {
        return true;
    }

    return false;
}

void SceneBasic_Uniform::generateRockField()
{
    rockInstances.clear();

    std::mt19937 rng(1337);

    std::uniform_real_distribution<float> posX(-32.0f, 32.0f);
    std::uniform_real_distribution<float> posZ(-30.0f, 30.0f);
    std::uniform_real_distribution<float> scaleXZ(0.18f, 0.42f);
    std::uniform_real_distribution<float> scaleY(0.16f, 0.34f);
    std::uniform_real_distribution<float> rotY(0.0f, 360.0f);

    const int targetRockCount = 14;
    int attempts = 0;

    while (rockInstances.size() < targetRockCount && attempts < 1000) {
        attempts++;

        float x = posX(rng);
        float z = posZ(rng);

        if (isRockSpawnBlocked(x, z)) {
            continue;
        }

        float sx = scaleXZ(rng);
        float sy = scaleY(rng);
        float sz = scaleXZ(rng);

        glm::vec3 scale(sx, sy, sz);

        float rockLocalMinY = -0.65f;
        float y = getTerrainHeight(x, z) - (rockLocalMinY * sy);

        RockInstance rock;
        rock.position = glm::vec3(x, y, z);
        rock.scale = scale;
        rock.rotationY = rotY(rng);

        rockInstances.push_back(rock);
    }

    std::cout << "Generated " << rockInstances.size() << " procedural rocks" << std::endl;
}

glm::vec3 SceneBasic_Uniform::getEnergyCellWorldPos()
{
    float hutY = getHutBaseY();
    float platformTopY = hutY + 0.36f;

    // Inside hut, slightly to the right and back from the door
    return glm::vec3(
        hutPos.x + 1.25f,
        platformTopY + 0.75f,
        hutPos.z - 0.75f
    );
}

bool SceneBasic_Uniform::isPlayerNearEnergyCell()
{
    glm::vec3 cellPos = getEnergyCellWorldPos();

    glm::vec2 playerXZ(cameraPos.x, cameraPos.z);
    glm::vec2 cellXZ(cellPos.x, cellPos.z);

    float dist = glm::length(playerXZ - cellXZ);
    return dist <= energyCellInteractRadius;
}

void SceneBasic_Uniform::drawEnergyCell(GLSLProgram& shader, bool depthPass, const glm::vec3& position, const glm::vec3& scale, float rotationY)
{
    if (energyCellVAO == 0 || energyCellVertexCount <= 0) return;

    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale);

    shader.setUniform("ModelMatrix", model);

    if (!depthPass) {
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

        glm::vec3 cellColor = glm::mix(
            glm::vec3(1.8f, 2.4f, 0.8f),
            glm::vec3(3.0f, 3.8f, 1.2f),
            corePulse
        );

        shader.setUniform("NormalMatrix", normalMatrix);
        shader.setUniform("ObjectColor", cellColor);
        shader.setUniform("UseTexture", 0);
        shader.setUniform("UseNoise", 0);
    }

    glBindVertexArray(energyCellVAO);
    glDrawArrays(GL_TRIANGLES, 0, energyCellVertexCount);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::renderScene(GLSLProgram& shader, bool depthPass)
{
    drawTerrain(shader, depthPass);

    auto draw = [&](const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::scale(model, scale);

            shader.setUniform("ModelMatrix", model);

            if (!depthPass) {
                glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
                shader.setUniform("NormalMatrix", normalMatrix);
                shader.setUniform("ObjectColor", color);
                shader.setUniform("UseTexture", 0);
                shader.setUniform("UseNoise", 0);
            }

            glBindVertexArray(vaoHandle);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        };

    auto drawGroundBlock = [&](float x, float z, const glm::vec3& scale, const glm::vec3& color)
        {
            float y = getTerrainHeight(x, z) + (scale.y * 0.5f);
            draw(glm::vec3(x, y, z), scale, color);
        };

    auto drawTexturedGroundBlock = [&](float x, float z, const glm::vec3& scale, const glm::vec3& tint)
        {
            float y = getTerrainHeight(x, z) + (scale.y * 0.5f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            model = glm::scale(model, scale);

            shader.setUniform("ModelMatrix", model);

            if (!depthPass) {
                glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
                shader.setUniform("NormalMatrix", normalMatrix);
                shader.setUniform("ObjectColor", tint);

                shader.setUniform("UseTexture", 2);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, woodTexture);
            }

            glBindVertexArray(vaoHandle);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        };

    auto drawRockOnGround = [&](float x, float z, const glm::vec3& scale, float rotationY)
        {
            float y = getTerrainHeight(x, z) + 0.65f * scale.y;
            drawRock(shader, depthPass, glm::vec3(x, y, z), scale, rotationY);
        };

    for (const RockInstance& rock : rockInstances) {
        drawRock(shader, depthPass, rock.position, rock.scale, rock.rotationY);
    }

    // Broken stone path leading from spawn toward the reactor
    glm::vec3 pathTint = glm::vec3(0.75f, 0.68f, 0.55f);

    drawTexturedGroundBlock(0.0f, 15.0f, glm::vec3(2.1f, 0.14f, 1.1f), pathTint);
    drawTexturedGroundBlock(-0.4f, 12.8f, glm::vec3(1.6f, 0.12f, 0.9f), pathTint);
    drawTexturedGroundBlock(0.5f, 10.6f, glm::vec3(1.9f, 0.12f, 1.0f), pathTint);
    drawTexturedGroundBlock(-0.2f, 8.4f, glm::vec3(1.5f, 0.12f, 0.9f), pathTint);
    drawTexturedGroundBlock(0.4f, 6.2f, glm::vec3(1.8f, 0.12f, 0.9f), pathTint);
    drawTexturedGroundBlock(-0.3f, 4.0f, glm::vec3(1.4f, 0.12f, 0.8f), pathTint);
    drawTexturedGroundBlock(0.2f, 2.1f, glm::vec3(1.5f, 0.12f, 0.8f), pathTint);

    // Reactor platform, grounded properly on the terrain
    float reactorGroundY = getTerrainHeight(0.0f, 0.0f);

    float baseHeight = 0.95f;
    float topHeight = 0.22f;

    float baseTopY = reactorGroundY + baseHeight;
    float topTopY = baseTopY + topHeight;

    // Large bottom base, now reaches the grass
    draw(
        glm::vec3(0.0f, reactorGroundY + baseHeight * 0.5f, 0.0f),
        glm::vec3(3.2f, baseHeight, 3.2f),
        glm::vec3(0.28f, 0.28f, 0.34f)
    );

    // Smaller top platform
    draw(
        glm::vec3(0.0f, baseTopY + topHeight * 0.5f, 0.0f),
        glm::vec3(1.9f, topHeight, 1.9f),
        glm::vec3(0.38f, 0.38f, 0.44f)
    );

    // Reactor model sitting on top of the platform
    drawReactor(
        shader,
        depthPass,
        glm::vec3(0.0f, topTopY + 0.65f, 0.0f),
        glm::vec3(0.25f, 0.25f, 0.25f),
        corePulse * 20.0f
    );

    // Colour change after reactor activation
    glm::vec3 ruinColor = reactorActivated
        ? glm::vec3(0.42f, 0.45f, 0.56f)
        : glm::vec3(0.24f, 0.24f, 0.26f);

    glm::vec3 darkRock = glm::vec3(0.18f, 0.18f, 0.19f);
    glm::vec3 midRock = glm::vec3(0.25f, 0.24f, 0.23f);

	// Simple hut structure near the reactor
    glm::vec3 hutBase = hutPos;
    float hutY = getHutBaseY();

    glm::vec3 woodColor = glm::vec3(0.32f, 0.22f, 0.13f);
    glm::vec3 wallColor = glm::vec3(0.46f, 0.42f, 0.34f);
    glm::vec3 roofColor = glm::vec3(0.22f, 0.20f, 0.18f);
    glm::vec3 floorColor = glm::vec3(0.26f, 0.26f, 0.26f);
    glm::vec3 doorColor = glm::vec3(0.34f, 0.20f, 0.10f);

    // Base platform
    draw(glm::vec3(hutBase.x, hutY + 0.18f, hutBase.z),
        glm::vec3(6.2f, 0.36f, 5.4f),
        floorColor);

    // Back wall
    draw(glm::vec3(hutBase.x, hutY + 2.0f, hutBase.z - 2.35f),
        glm::vec3(5.4f, 3.8f, 0.22f),
        wallColor);

    // Left wall
    draw(glm::vec3(hutBase.x - 2.7f, hutY + 2.0f, hutBase.z),
        glm::vec3(0.22f, 3.8f, 4.7f),
        wallColor);

    // Right wall
    draw(glm::vec3(hutBase.x + 2.7f, hutY + 2.0f, hutBase.z),
        glm::vec3(0.22f, 3.8f, 4.7f),
        wallColor);

    // Roof
    draw(glm::vec3(hutBase.x, hutY + 3.95f, hutBase.z),
        glm::vec3(6.6f, 0.30f, 5.8f),
        roofColor);

    // Front support posts
    draw(glm::vec3(hutBase.x - 2.45f, hutY + 1.9f, hutBase.z + 2.15f),
        glm::vec3(0.24f, 3.6f, 0.24f),
        woodColor);

    draw(glm::vec3(hutBase.x + 2.45f, hutY + 1.9f, hutBase.z + 2.15f),
        glm::vec3(0.24f, 3.6f, 0.24f),
        woodColor);

    float frontZ = hutBase.z + 2.35f;
    float frontWallWidth = 5.4f;
    float frontWallHeight = 3.8f;
    float frontWallThickness = 0.22f;

    float platformTopY = hutY + 0.24f;

    // Door opening settings
    float doorWidth = 1.20f;
    float doorHeight = 2.75f;
    float sideWallWidth = (frontWallWidth - doorWidth) * 0.5f;
    float openingCenterX = hutBase.x;

    // Main front wall side pieces
    draw(glm::vec3(openingCenterX - (doorWidth * 0.5f + sideWallWidth * 0.5f),
        platformTopY + frontWallHeight * 0.5f,
        frontZ),
        glm::vec3(sideWallWidth, frontWallHeight, frontWallThickness),
        wallColor);

    draw(glm::vec3(openingCenterX + (doorWidth * 0.5f + sideWallWidth * 0.5f),
        platformTopY + frontWallHeight * 0.5f,
        frontZ),
        glm::vec3(sideWallWidth, frontWallHeight, frontWallThickness),
        wallColor);

    // Top wall above the door (lintel)
    float topWallHeight = frontWallHeight - doorHeight;
    draw(glm::vec3(openingCenterX,
        platformTopY + doorHeight + topWallHeight * 0.5f,
        frontZ),
        glm::vec3(doorWidth, topWallHeight, frontWallThickness),
        wallColor);

    glm::vec3 doorCenter = glm::vec3(
        openingCenterX,
        platformTopY + doorHeight * 0.5f,
        frontZ + 0.15f
    );

    glm::mat4 doorModel = glm::mat4(1.0f);
    doorModel = glm::translate(doorModel, doorCenter);

    // Hinge on left side
    doorModel = glm::translate(doorModel, glm::vec3(-doorWidth * 0.5f, 0.0f, 0.0f));
    doorModel = glm::rotate(doorModel, glm::radians(doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    doorModel = glm::translate(doorModel, glm::vec3(doorWidth * 0.5f, 0.0f, 0.0f));

    doorModel = glm::scale(doorModel, glm::vec3(doorWidth, doorHeight, 0.09f));

    shader.setUniform("ModelMatrix", doorModel);

    if (!depthPass) {
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(doorModel)));
        shader.setUniform("NormalMatrix", normalMatrix);
        shader.setUniform("ObjectColor", doorColor);
        shader.setUniform("UseTexture", 0);
    }

    glBindVertexArray(vaoHandle);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Energy Cell inside hut
    if (!energyCellCollected) {
        drawEnergyCell(
            shader,
            depthPass,
            getEnergyCellWorldPos(),
            glm::vec3(0.35f, 0.35f, 0.35f),
            energyCellSpin
        );
    }
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    projection = glm::perspective(glm::radians(60.0f), (float)w / (float)h, 0.1f, 100.0f);
}