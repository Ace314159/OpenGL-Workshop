#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "WorldGen.h"

std::pair<GLuint, GLsizei> createVAO() {

    const auto& [vertices, indices] = makeVertexData();

    // Create VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and set data to VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(vertices[0]) * vertices.size()), vertices.data(),
                 GL_STATIC_DRAW);

    // Create and set data to EBO
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(indices[0]) * indices.size()), indices.data(),
                 GL_STATIC_DRAW);

    // Configure vertex attributes
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    return {VAO, indices.size()};
}

GLuint createShaderProgram() {
    // Shader Source
    const char *vertexShaderSrc = R"delim(
#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 proj;
uniform mat4 view;

out vec3 outColor;

void main() {
    gl_Position = proj * view * aPos;
    outColor = aColor;
}
)delim";
    const char *fragmentShaderSrc = R"delim(
#version 330 core
in vec3 outColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(outColor, 1.0f);
}
)delim";

    // Create, set source, and compile shaders
    GLuint vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    // Exercise for the reader: Error handling
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    // Link shader program consisting of vertex and fragment shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // After linking, delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL Workshop", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        glfwTerminate();
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    });
    glEnable(GL_DEPTH_TEST);

    const auto [VAO, numIndices] = createVAO();
    GLuint shaderProgram = createShaderProgram();

    int projLoc = glGetUniformLocation(shaderProgram, "proj");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMat));

    double yaw = -90, pitch = 45;
    glm::mat4 viewMat(1);
    double prevX = 0, prevY = 0;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    auto callback = [&prevX, &prevY, &yaw, &pitch, &viewMat](double x, double y) {
        double sensitivity = 0.1;
        double dX = (x - prevX) * sensitivity;
        double dY = -(y - prevY) * sensitivity;
        prevX = x;
        prevY = y;

        yaw += dX;
        pitch += dY;
        if(pitch > 89) pitch = 89;
        else if(pitch < -89) pitch = -89;

        const glm::vec3 front = glm::normalize(glm::vec3(
                cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
                sin(glm::radians(pitch)),
                cos(glm::radians(pitch)) * sin(glm::radians(yaw))
        ));
        const glm::vec3 up{0, 1, 0};
        viewMat = glm::lookAt(pos, pos + front, up);
    };
    glfwSetWindowUserPointer(window, &callback);
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
        auto callbackPtr = static_cast<decltype(&callback)>(glfwGetWindowUserPointer(window));
        (*callbackPtr)(x, y);
    });
    glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused) {
        if (focused) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    });

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.53, 0.81, 0.92, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            std::cout << glm::to_string(viewMat) << std::endl;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        // Bind VAO
        glBindVertexArray(VAO);
        // Use Shader
        glUseProgram(shaderProgram);
        // Draw
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
