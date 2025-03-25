#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>


#ifndef M_PI
#define M_PI asin(1.0) * 2.0
#endif

const int WINDOW_SIZE = 600;
const float CIRCLE_RADIUS = 25.0f;
const float SEGMENT_LENGTH = WINDOW_SIZE / 3.0f;
const float SEGMENT_THICKNESS = 3.0f;
const float VECTOR_LENGTH = 10.0f; // slower movement
const float ANGLE = 25.0f * M_PI / 180.0f; 

// Vertex shader for both circle and segment
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
)";
// Fragment shader for circle
const char* circleFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec2 circleCenter;
uniform float radius;
uniform vec3 centerColor = vec3(1.0, 0.0, 0.0); // Red
uniform vec3 borderColor = vec3(0.0, 1.0, 0.0); // Green
void main() {
    float dist = distance(gl_FragCoord.xy, circleCenter);
    if (dist <= radius) {
        float t = dist / radius;
        FragColor = vec4(mix(centerColor, borderColor, t), 1.0);
    } else {
        FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Yellow background
    }
}
)";
// Fragment shader for segment
const char* segmentFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue
}
)";

void checkShaderCompile(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
}

void checkProgramLink(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }
}


float segmentY = WINDOW_SIZE / 2.0f;
float circleX = WINDOW_SIZE / 2.0f;
float circleY = WINDOW_SIZE / 2.0f;
float velocityX = 5.0f; 
float velocityY = 0.0f;
bool vectorMode = false;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            segmentY += 5.0f;
            if (segmentY + SEGMENT_THICKNESS / 2.0f > WINDOW_SIZE) segmentY = WINDOW_SIZE - SEGMENT_THICKNESS / 2.0f;
        }
        if (key == GLFW_KEY_DOWN) {
            segmentY -= 5.0f;
            if (segmentY - SEGMENT_THICKNESS / 2.0f < 0) segmentY = SEGMENT_THICKNESS / 2.0f;
        }
        if (key == GLFW_KEY_S && !vectorMode) {
            velocityX = VECTOR_LENGTH * cos(ANGLE);
            velocityY = VECTOR_LENGTH * sin(ANGLE);
            vectorMode = true;
        }
    }
}
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "Bouncing Circle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

   
    glfwSwapInterval(1);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    // Compile vertex shader (shared)
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompile(vertexShader);

    // Compile circle fragment shader
    GLuint circleFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(circleFragmentShader, 1, &circleFragmentShaderSource, NULL);
    glCompileShader(circleFragmentShader);
    checkShaderCompile(circleFragmentShader);

    // Compile segment fragment shader
    GLuint segmentFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(segmentFragmentShader, 1, &segmentFragmentShaderSource, NULL);
    glCompileShader(segmentFragmentShader);
    checkShaderCompile(segmentFragmentShader);

    // Link circle program
    GLuint circleProgram = glCreateProgram();
    glAttachShader(circleProgram, vertexShader);
    glAttachShader(circleProgram, circleFragmentShader);
    glLinkProgram(circleProgram);
    checkProgramLink(circleProgram);

    // Link segment program
    GLuint segmentProgram = glCreateProgram();
    glAttachShader(segmentProgram, vertexShader);
    glAttachShader(segmentProgram, segmentFragmentShader);
    glLinkProgram(segmentProgram);
    checkProgramLink(segmentProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(circleFragmentShader);
    glDeleteShader(segmentFragmentShader);

    // Quad for circle (full screen)
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    GLuint circleVAO, circleVBO, circleEBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glGenBuffers(1, &circleEBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Segment vertices (normalized coordinates)
    float segmentXLeft = (WINDOW_SIZE - SEGMENT_LENGTH) / 2.0f / WINDOW_SIZE * 2.0f - 1.0f;
    float segmentXRight = (WINDOW_SIZE + SEGMENT_LENGTH) / 2.0f / WINDOW_SIZE * 2.0f - 1.0f;
    GLuint segmentVAO, segmentVBO;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Update circle position
        circleX += velocityX;
        circleY += velocityY;

        // Bounce logic
        if (circleX - CIRCLE_RADIUS < 0) {
            velocityX = -velocityX;
            circleX = CIRCLE_RADIUS;
        }
        if (circleX + CIRCLE_RADIUS > WINDOW_SIZE) {
            velocityX = -velocityX;
            circleX = WINDOW_SIZE - CIRCLE_RADIUS;
        }
        if (circleY - CIRCLE_RADIUS < 0) {
            velocityY = -velocityY;
            circleY = CIRCLE_RADIUS;
        }
        if (circleY + CIRCLE_RADIUS > WINDOW_SIZE) {
            velocityY = -velocityY;
            circleY = WINDOW_SIZE - CIRCLE_RADIUS;
        }
        // Draw circle
        glUseProgram(circleProgram);
        glUniform2f(glGetUniformLocation(circleProgram, "circleCenter"), circleX, circleY);
        glUniform1f(glGetUniformLocation(circleProgram, "radius"), CIRCLE_RADIUS);
        glBindVertexArray(circleVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Update and draw segment
        float segmentYNorm = segmentY / WINDOW_SIZE * 2.0f - 1.0f;
        float thicknessNorm = SEGMENT_THICKNESS / WINDOW_SIZE * 2.0f;
        float segmentVertices[] = {
            segmentXLeft, segmentYNorm - thicknessNorm / 2.0f,
            segmentXRight, segmentYNorm - thicknessNorm / 2.0f,
            segmentXRight, segmentYNorm + thicknessNorm / 2.0f,
            segmentXLeft, segmentYNorm + thicknessNorm / 2.0f
        };

        glGenVertexArrays(1, &segmentVAO);
        glGenBuffers(1, &segmentVBO);
        glBindVertexArray(segmentVAO);
        glBindBuffer(GL_ARRAY_BUFFER, segmentVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(segmentVertices), segmentVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glUseProgram(segmentProgram);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteBuffers(1, &circleEBO);
    glDeleteVertexArrays(1, &segmentVAO);
    glDeleteBuffers(1, &segmentVBO);
    glDeleteProgram(circleProgram);
    glDeleteProgram(segmentProgram);

    glfwTerminate();
    return 0;
}
