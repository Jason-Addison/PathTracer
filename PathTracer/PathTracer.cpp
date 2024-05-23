// PathTracer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define EMPTY 0
#define SPHERE 1
#define CUBOID 2
#define PLANE 3
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <SOIL/SOIL.h>

double x = 0.0;
double y = 0.0;
double z = 6.0;
double speed = 0.1;
double xRot, yRot, zRot = 0.0f;
int frame = 0;
int MAX_FRAMES = 300;
int screenWidth = 800;
int screenHeight = 800;
int objectCount = 17;

GLuint framebuffer;
GLuint textureColorBuffer;
GLuint shaderProgram;
std::string readShaderFile(const std::string& filePath)
{
    std::ifstream shaderFile(filePath);
    if (!shaderFile)
    {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

GLuint loadImage(std::string file)
{
    GLuint textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height;
    unsigned char* image = SOIL_load_image(file.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

    if (image == nullptr)
    {
        std::cerr << "Error loading image: " << SOIL_last_result() << std::endl;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    return textureID;
}

void clearScreen()
{
    frame = 0;
}

double toRadians(double degrees)
{
    return degrees * 0.0174532925;
}

int numSamples = 8;
int maxBounces = 4;

void sizeCallback(GLFWwindow* window, int width, int height)
{
    std::cout << "Window resized: " << width << "x" << height << std::endl;
    screenWidth = width;
    screenHeight = height;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &textureColorBuffer);
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
    glViewport(0, 0, screenWidth, screenHeight);
}

void getInput(GLFWwindow* window)
{
    float xRotRad = toRadians(xRot);
    float yRotRad = toRadians(yRot);
    float zRotRad = toRadians(zRot);

    float forwardX = std::sin(yRotRad);
    float forwardZ = std::cos(yRotRad);

    float rightX = std::sin(yRotRad - toRadians(90.0f));
    float rightZ = std::cos(yRotRad - toRadians(90.0f));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        x -= speed * forwardX;
        z -= speed * forwardZ;
        clearScreen();
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        x += speed * forwardX;
        z += speed * forwardZ;
        clearScreen();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        x += speed * rightX;
        z += speed * rightZ;
        clearScreen();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        x -= speed * rightX;
        z -= speed * rightZ;
        clearScreen();
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        y -= speed;
        clearScreen();
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        y += speed;
        clearScreen();
    }
}

double lastMouseX = 0;
double lastMouseY = 0;
void mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    const float sensitivity = 0.1f;
    if (lastMouseX != xPos || lastMouseY != yPos)
    {
        clearScreen();
        xRot += (lastMouseY - yPos) * sensitivity;
        yRot += (lastMouseX - xPos) * sensitivity;
    }
    lastMouseX = xPos;
    lastMouseY = yPos;
}

void loadObject(GLuint shader, int type, float x, float y, float z, float xScale, float yScale, float zScale, float wScale,
    float r, float g, float b, float re, float ge, float be, float roughness, float ior, int id)
{
    glUseProgram(shaderProgram);
    GLuint loc;
    std::string obj = "objects[" + std::to_string(id - 1) + "].";
    loc = glGetUniformLocation(shader, (obj + "id").c_str());
    glUniform1i(loc, id);
    loc = glGetUniformLocation(shader, (obj + "type").c_str());
    glUniform1i(loc, type);
    loc = glGetUniformLocation(shader, (obj + "position").c_str());
    glUniform3f(loc, x, y, z);
    loc = glGetUniformLocation(shader, (obj + "scale").c_str());
    glUniform4f(loc, xScale, yScale, zScale, 0.0);

    loc = glGetUniformLocation(shader, (obj + "material.color").c_str());
    glUniform3f(loc, r, g, b);
    loc = glGetUniformLocation(shader, (obj + "material.emission").c_str());
    glUniform3f(loc, re, ge, be);
    loc = glGetUniformLocation(shader, (obj + "material.roughness").c_str());
    glUniform1f(loc, roughness);
    loc = glGetUniformLocation(shader, (obj + "material.indexOfRefraction").c_str());
    glUniform1f(loc, ior);
}

void loadObjects(GLuint shader)
{
    loadObject(shader, SPHERE, 0, 3, 3, 0.50, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1);
    loadObject(shader, CUBOID, 0, -10, 0, 100.0, 10.0, 100.0, 2.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1, 0.0, 2);
    loadObject(shader, CUBOID, 0, 10, 0, 100.0, 10.0, 100.0, 2.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 3);
    loadObject(shader, CUBOID, 0, 0, -10, 10.0, 100.0, 10.0, 0.0, 0.2, 0.2, 0.3, 0.0, 0.0, 0.0, 1.0, 0.0, 4);
    loadObject(shader, CUBOID, -10, 0, 0, 10.0, 100.0, 100.0, 0.0, 0.9, 0.1, 0.7, 0.0, 0.0, 0.0, 1.0, 0.0, 5);
    loadObject(shader, CUBOID, 10, 0, -45, 10.0, 100.0, 100.0, 0.0, 0.0, 0.2, 0.5, 0.0, 0.0, 0.0, 0.4, 0.0, 6);
    loadObject(shader, CUBOID, 0, 0, 15, 100.0, 100.0, 1.0, 2.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 7);
    loadObject(shader, SPHERE, 0, 3, 0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 8);
    loadObject(shader, CUBOID, -4, -3, 7, 3.0, 3.0, 3.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 9);
    loadObject(shader, CUBOID, 4, 3, 7, 3.0, 30.0, 5.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10);
    loadObject(shader, CUBOID, 15, 3, 7, 3.0, 30.0, 50.0, 0.0, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.00, 0.0, 11);
    loadObject(shader, CUBOID, 10, 5, 12, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1, 0.0, 12);
    loadObject(shader, CUBOID, 10, 5, 9, 1.0, 1.0, 3.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1, 0.0, 13);
    loadObject(shader, SPHERE, -3, -3, -3, 2.0, 2.0, 2.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 14);
    loadObject(shader, SPHERE, 0, -4, 0, 1.0, 2.0, 2.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 15);
    loadObject(shader, SPHERE, 2, -4, 0, 1.0, 2.0, 2.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.02, 1.5, 16);
    loadObject(shader, SPHERE, 4, -4, 0, 1.0, 2.0, 2.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 5.0, 17);
}

float objectSize = 1.0f;

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset > 0)
    {
        if (objectSize > 0)
        {
            objectSize *= 1.1;
        }
        else
        {
            objectSize += 0.1;
        }
    }
    else
    {
        objectSize /= 1.1;
        if (objectSize < 0.05)
        {
            objectSize = 0;
        }
    }
  
    loadObject(shaderProgram, SPHERE, 0, 3, 3, objectSize, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1);
    clearScreen();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_UP)
        {
            numSamples++;
            clearScreen();
        }
        else if (key == GLFW_KEY_DOWN)
        {
            numSamples = std::max(numSamples - 1, 1);
            clearScreen();
        }
        else if (key == GLFW_KEY_RIGHT)
        {
            maxBounces++;
            clearScreen();
        }
        else if (key == GLFW_KEY_LEFT)
        {
            maxBounces = std::max(maxBounces - 1, 1);
            clearScreen();
        }
        else if (key == GLFW_KEY_0)
        {
            loadObject(shaderProgram, SPHERE, 0, 3, 3, objectSize, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1);
            clearScreen();
        }
        else if (key == GLFW_KEY_1)
        {
            loadObject(shaderProgram, SPHERE, 0, 3, 3, objectSize, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1);
            clearScreen();
        }
        else if (key == GLFW_KEY_2)
        {
            loadObject(shaderProgram, SPHERE, 0, 3, 3, objectSize, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1);
            clearScreen();
        }
        else if (key == GLFW_KEY_3)
        {
            loadObject(shaderProgram, SPHERE, 0, 3, 3, objectSize, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0, 0, 1.0, 1.0, 0.0, 1);
            clearScreen();
        }
    }
}
GLuint shaderProgram2D = 0;
GLuint VAO2D = 0;
void generate2DImageShader()
{
    GLfloat vertices[] =
    {
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f
    };
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;

        out vec2 TexCoord;

        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 TexCoord;
        uniform sampler2D textureSampler;

        void main() {
            FragColor = texture(textureSampler, TexCoord);
        }
    )";

    GLuint VBO;
    glGenVertexArrays(1, &VAO2D);
    glBindVertexArray(VAO2D);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    shaderProgram2D = shaderProgram;
    glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Path Tracer", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    GLfloat vertices[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    std::string vertexShaderSource = readShaderFile("shaders/tracer.vert");
    if (vertexShaderSource.empty()) {
        std::cerr << "Failed to read vertex shader source" << std::endl;
        return -1;
    }
    const char* vertexShaderStr = vertexShaderSource.c_str();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderStr, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return -1;
    }

    std::string fragmentShaderSource = readShaderFile("shaders/tracer_constants.frag") + readShaderFile("shaders/tracer_intersects.frag") + readShaderFile("shaders/tracer.frag");
    
    if (fragmentShaderSource.empty())
    {
        std::cerr << "Failed to read fragment shader source" << std::endl;
        return -1;
    }

    const char* fragmentShaderStr = fragmentShaderSource.c_str();
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderStr, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        return -1;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint resolutionLocation = glGetUniformLocation(shaderProgram, "resolution");
    GLint frameLocation = glGetUniformLocation(shaderProgram, "frame");
    GLint cameraPositionLocation = glGetUniformLocation(shaderProgram, "cameraPosition");
    GLint cameraRotationLocation = glGetUniformLocation(shaderProgram, "cameraRotation");
    GLint lastFrameLocation = glGetUniformLocation(shaderProgram, "lastFrame");
    GLint bouncesLocation = glGetUniformLocation(shaderProgram, "MAX_BOUNCES");
    GLint samplesLocation = glGetUniformLocation(shaderProgram, "SAMPLES");
    GLint objectCountLocation = glGetUniformLocation(shaderProgram, "OBJECT_COUNT");

    glUseProgram(shaderProgram);
    glUniform2f(resolutionLocation, static_cast<float>(screenWidth), static_cast<float>(screenHeight));
    glUniform1i(frameLocation, 0);
    glUniform3f(cameraPositionLocation, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    glUniform1i(lastFrameLocation, 0);
    glUniform1i(objectCountLocation, objectCount);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    loadObjects(shaderProgram);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    GLuint texture = loadImage("textures/wood.png");
    
    GLuint textureLocation = glGetUniformLocation(shaderProgram, "textureMap");
    glUniform1i(textureLocation, 1);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    generate2DImageShader();
    glfwSetFramebufferSizeCallback(window, sizeCallback);

    std::cout << "Starting...";
    while (!glfwWindowShouldClose(window))
    {
        std::string title = std::string("Path Tracer | Bounces: ") + std::to_string(maxBounces) + std::string(" Samples: ") + std::to_string(numSamples) + std::string(" Frame: ") + std::to_string(frame);
        glfwSetWindowTitle(window, title.c_str());
        glfwPollEvents();

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        if (frame == 0)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        getInput(window);
        if (frame < MAX_FRAMES)
        {
            glUseProgram(shaderProgram);
            glUniform3f(cameraPositionLocation, x, y, z);
            glUniform3f(cameraRotationLocation, xRot, yRot, zRot);
            glUniform2f(resolutionLocation, screenWidth, screenHeight);
            glUniform1i(bouncesLocation, maxBounces);
            glUniform1i(samplesLocation, numSamples);
            glBindVertexArray(VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glBindVertexArray(0);
            frame++;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        glUseProgram(shaderProgram);
        glUniform1i(frameLocation, frame);

        glUseProgram(shaderProgram2D);
        glBindVertexArray(VAO2D);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
