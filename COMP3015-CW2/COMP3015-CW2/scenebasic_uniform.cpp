#include "scenebasic_uniform.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <GLFW/glfw3.h>
#include "helper/glutils.h"

using glm::vec3;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform()
    : vaoHandle(0),
    vboHandle(0),
    shadowFBO(0),
    shadowTex(0),
    cameraPos(0.0f, 1.2f, 2.5f),
    cameraFront(0.0f, 0.0f, -1.0f),
    cameraUp(0.0f, 1.0f, 0.0f),
    lightPos(1.5f, 3.5f, 2.0f),
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

        prog.use();
        prog.setUniform("ShadowMap", 0);
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);

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

    prog.use();
    prog.setUniform("LightPos", vec3(0.0f, 2.5f, 2.0f));
    prog.setUniform("LightPos", lightPos);
    prog.setUniform("ShadowMap", 0);
}

void SceneBasic_Uniform::processInput(float dt)
{
    if (!window) return;

    const float moveSpeed = 3.0f * dt;

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
}

void SceneBasic_Uniform::update(float t)
{
    float dt = t - lastTime;
    lastTime = t;

    processInput(dt);

    if (animating()) {
        corePulse = sin(t * 2.0f) * 0.5f + 0.5f;
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
    mat4 lightProjection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 1.0f, 15.0f);
    mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mat4 lightSpaceMatrix = lightProjection * lightView;

    // Pass 1: depth from light
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    depthProg.use();
    depthProg.setUniform("LightSpaceMatrix", lightSpaceMatrix);
    renderScene(depthProg, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Pass 2: normal render with shadow map
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.use();

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    prog.setUniform("ViewMatrix", view);
    prog.setUniform("ProjectionMatrix", projection);
    prog.setUniform("ViewPos", cameraPos);
    prog.setUniform("LightPos", lightPos);
    prog.setUniform("LightSpaceMatrix", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowTex);

    renderScene(prog, false);
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

void SceneBasic_Uniform::renderScene(GLSLProgram& shader, bool depthPass)
{
    auto draw = [&](const vec3& position, const vec3& scale, const vec3& color)
        {
            model = mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::scale(model, scale);

            shader.setUniform("ModelMatrix", model);

            if (!depthPass) {
                mat3 normalMatrix = mat3(glm::transpose(glm::inverse(model)));
                shader.setUniform("NormalMatrix", normalMatrix);
                shader.setUniform("ObjectColor", color);
            }

            glBindVertexArray(vaoHandle);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        };

    draw(vec3(0.0f, -0.6f, 0.0f), vec3(8.0f, 0.2f, 8.0f), vec3(0.25f, 0.25f, 0.30f));
    draw(vec3(0.0f, 1.4f, -4.0f), vec3(8.0f, 4.0f, 0.2f), vec3(0.18f, 0.18f, 0.22f));
    draw(vec3(0.0f, 1.4f, 4.0f), vec3(8.0f, 4.0f, 0.2f), vec3(0.18f, 0.18f, 0.22f));
    draw(vec3(-4.0f, 1.4f, 0.0f), vec3(0.2f, 4.0f, 8.0f), vec3(0.16f, 0.16f, 0.20f));
    draw(vec3(4.0f, 1.4f, 0.0f), vec3(0.2f, 4.0f, 8.0f), vec3(0.16f, 0.16f, 0.20f));

    vec3 coreColor = glm::mix(vec3(0.2f, 0.6f, 1.0f), vec3(0.6f, 0.9f, 1.0f), corePulse);
    draw(vec3(0.0f, 0.2f, 0.0f), vec3(0.8f, 0.8f, 0.8f), coreColor);
    draw(vec3(0.0f, -0.2f, 0.0f), vec3(1.6f, 0.2f, 1.6f), vec3(0.30f, 0.30f, 0.35f));
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    projection = glm::perspective(glm::radians(60.0f), (float)w / (float)h, 0.1f, 100.0f);
}