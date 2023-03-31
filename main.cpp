#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include "externals/includes/irrKlang.h"

using namespace irrklang;

ISoundEngine *SoundEngine = createIrrKlangDevice();

#include <iostream>

#define RAIN 100
#define GRASS 4000
#define TALL_GRASS 100
float randomSize, variant[GRASS], variantT[TALL_GRASS];
glm::mat4 mdlG[GRASS], mdlTG[TALL_GRASS], mdlR[RAIN];

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
float lightAngle;
float carPoz = 0;
float fall;
int rainToggle = -1;
bool car_anim;
// skybox
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
std::vector<const GLchar *> faces;

// camera
gps::Camera myCamera(
        glm::vec3(39.0f, 4.2f, 45.5f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

std::ifstream cameraFile("moveCamera.txt");
//std::ofstream cameraOut("moveCamera.txt");

bool firstMouse = true;
float yaw = -90.0f;    // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
int carsnd;

GLfloat cameraSpeed = 0.2f;

GLint night = 0;

GLboolean pressedKeys[1024];

// models
gps::Model3D ground, bridge, grass, tallGrass, rain, water, tree, leaves, mirror, car;

// shaders
gps::Shader myBasicShader, grassShader, rainShader, shadowShader, debugDepthQuad, reflectShader;

bool moveCalemra;

// wind
glm::vec2 wd = glm::vec2(0.1, 0.1);
float ws = 0.5f;
float wind = 0.0f;
float veloc = 0.005f;
float windAngle;

float waterAngle;

bool debug = false;

GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow *window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    //TODO
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    grassShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    rainShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void moveCamera() {
    std::string val;
    if (getline(cameraFile, val)) {
        myCamera.setxCameraPosition(std::stof(val));
    } else {
        moveCalemra = false;
        return;
    }
    if (getline(cameraFile, val)) {
        myCamera.setyCameraPosition(std::stof(val));
    } else {
        moveCalemra = false;
        return;
    }
    if (getline(cameraFile, val)) {
        myCamera.setzCameraPosition(std::stof(val));
    } else {
        moveCalemra = false;
        return;
    }


    if (getline(cameraFile, val)) {
        myCamera.setxCameraFront(std::stof(val));
    } else {
        moveCalemra = false;
        return;
    }
    if (getline(cameraFile, val)) {
        myCamera.setyCameraFront(std::stof(val));
    } else {
        moveCalemra = false;
        return;
    }
    if (getline(cameraFile, val)) {
        myCamera.setzCameraFront(std::stof(val));
    } else {
        moveCalemra = false;
        return;
    }

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE,
                       glm::value_ptr(view));
    grassShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    rainShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement() {
//    if (cameraOut.is_open()) {
//        cameraOut << myCamera.getCameraPosition().x << std::endl;
//        cameraOut << myCamera.getCameraPosition().y << std::endl;
//        cameraOut << myCamera.getCameraPosition().z << std::endl;
//
//        cameraOut << myCamera.getCameraFront().x << std::endl;
//        cameraOut << myCamera.getCameraFront().y << std::endl;
//        cameraOut << myCamera.getCameraFront().z << std::endl;
//    }
    if (pressedKeys[GLFW_KEY_F]) {
        moveCalemra = true;
    }
    if (pressedKeys[GLFW_KEY_G]) {
        moveCalemra = false;
    }
    if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        grassShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        rainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_SPACE]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        grassShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        rainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        grassShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        rainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        grassShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        rainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        grassShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        rainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        grassShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        rainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        rainToggle = -1;
        SoundEngine->stopAllSounds();
        SoundEngine->play2D("audio/wind.mp3", true);
    }

    if (pressedKeys[GLFW_KEY_E]) {
        rainToggle = 1;
        SoundEngine->stopAllSounds();
        SoundEngine->play2D("audio/rain.mp3", true);
    }

    if (pressedKeys[GLFW_KEY_K]) {
        lightAngle += 0.3f;
        if (lightAngle > 360.0f)
            lightAngle -= 360.0f;
        glm::vec3 lightDirTr = glm::vec3(
                glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDirTr));
        grassShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDirTr));
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle -= 0.3f;
        if (lightAngle < 0.0f)
            lightAngle += 360.0f;
        glm::vec3 lightDirTr = glm::vec3(
                glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDirTr));
        grassShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDirTr));
    }

    if (pressedKeys[GLFW_KEY_O]) {
        windAngle -= 0.01f;
    }

    if (pressedKeys[GLFW_KEY_P]) {
        windAngle += 0.01f;
    }

    if (pressedKeys[GLFW_KEY_N]) {
        debug = true;
    }

    if (pressedKeys[GLFW_KEY_M]) {
        debug = false;
    }

    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    if (pressedKeys[GLFW_KEY_U]) {
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram,"fogOn"),1);
        reflectShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(reflectShader.shaderProgram,"fogOn"),1);
        grassShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(grassShader.shaderProgram,"fogOn"),1);
    }

    if (pressedKeys[GLFW_KEY_I]) {
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram,"fogOn"),0);
        reflectShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(reflectShader.shaderProgram,"fogOn"),0);
        grassShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(grassShader.shaderProgram,"fogOn"),0);
    }

    if (pressedKeys[GLFW_KEY_C]) {
        car_anim = true;
        carsnd = 0;
    }

    if (pressedKeys[GLFW_KEY_V]) {
        night = 0;
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "night"), night);
        grassShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(grassShader.shaderProgram, "night"), night);
        reflectShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(reflectShader.shaderProgram, "night"), night);
        skyboxShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(skyboxShader.shaderProgram, "night"), night);
    }

    if (pressedKeys[GLFW_KEY_B]) {
        night = 1;
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "night"), night);
        grassShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(grassShader.shaderProgram, "night"), night);
        reflectShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(reflectShader.shaderProgram, "night"), night);
        skyboxShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(skyboxShader.shaderProgram, "night"), night);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

gps::Model3D test;

void initModels() {

    faces.push_back("skybox/px.png");
    faces.push_back("skybox/nx.png");
    faces.push_back("skybox/py.png");
    faces.push_back("skybox/ny.png");
    faces.push_back("skybox/pz.png");
    faces.push_back("skybox/nz.png");

    ground.LoadModel("models/ground/gnd.obj");
    bridge.LoadModel("models/bridge/brg.obj");
    grass.LoadModel("models/grass/grs.obj");
    tallGrass.LoadModel("models/grass/grs1.obj");
    rain.LoadModel("models/rain/rain.obj");
//    test.LoadModel("models/test/test.obj");
    water.LoadModel("models/water/water.obj");
    tree.LoadModel("models/tree/tree.obj");
    leaves.LoadModel("models/tree/leaves.obj");
    mirror.LoadModel("models/water/mirror.obj");
    car.LoadModel("models/car/car.obj");
}

void initShaders() {
    myBasicShader.loadShader(
            "shaders/basic.vert",
            "shaders/basic.frag");
    grassShader.loadShader(
            "shaders/grass.vert",
            "shaders/grass.frag");
    rainShader.loadShader(
            "shaders/rain.vert",
            "shaders/rain.frag");
    shadowShader.loadShader(
            "shaders/shadow.vert",
            "shaders/shadow.frag");
    debugDepthQuad.loadShader(
            "shaders/quad.vert",
            "shaders/quad.frag");
    reflectShader.loadShader(
            "shaders/reflect.vert",
            "shaders/reflect.frag");


    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
                       glm::value_ptr(view));
    projection = glm::perspective(glm::radians(45.0f), (float) myWindow.getWindowDimensions().width /
                                                       (float) myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
}

const GLfloat near_plane = -100.0f, far_plane = 100.0f;

glm::mat4 computeLightSpaceTrMatrix() {

    glm::mat4 lightProjection = glm::ortho(-65.0f, 65.0f, -65.0f, 65.0f, near_plane, far_plane);

    glm::vec3 lightDirTr = glm::vec3(
            glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::vec4(lightDir, 1.0f));
    glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    return lightProjection * lightView;
}

const unsigned int SHADOW_WIDTH = 3072, SHADOW_HEIGHT = 3072;
unsigned int depthMapFBO;
// create depth texture
unsigned int depthMap;

void initFBOs() {
    glGenFramebuffers(1, &depthMapFBO);
    // configure depth map FBO
    // -----------------------
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::mat4(1.0f);

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    // send view matrix to shader
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
                                  (float) myWindow.getWindowDimensions().width /
                                  (float) myWindow.getWindowDimensions().height,
                                  0.1f, 1000.0f);
    // send projection matrix to shader
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 5.0f, 10.0f);
    // send light dir to shader
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    // send light color to shader
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "viewPos"), 1,
                 glm::value_ptr(myCamera.getCameraPosition()));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight1"), 1,
                 glm::value_ptr(glm::vec3(2.7f, 9.8f, 33.5f)));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight2"), 1,
                 glm::value_ptr(glm::vec3(1.6f, 9.8f, 22.9f)));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight3"), 1,
                 glm::value_ptr(glm::vec3(1.6f, 9.8f, 14.5f)));

    grassShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(grassShader.shaderProgram, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(grassShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(grassShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(grassShader.shaderProgram, "pointLight1"), 1,
                 glm::value_ptr(glm::vec3(2.7f, 9.8f, 33.5f)));
    glUniform3fv(glGetUniformLocation(grassShader.shaderProgram, "pointLight2"), 1,
                 glm::value_ptr(glm::vec3(1.6f, 9.8f, 22.9f)));
    glUniform3fv(glGetUniformLocation(grassShader.shaderProgram, "pointLight3"), 1,
                 glm::value_ptr(glm::vec3(1.6f, 9.8f, 14.5f)));

    rainShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(rainShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(rainShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

    reflectShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(reflectShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(reflectShader.shaderProgram, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(reflectShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(reflectShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(reflectShader.shaderProgram, "pointLight1"), 1,
                 glm::value_ptr(glm::vec3(2.7f, 9.8f, 33.5f)));
    glUniform3fv(glGetUniformLocation(reflectShader.shaderProgram, "pointLight2"), 1,
                 glm::value_ptr(glm::vec3(1.6f, 9.8f, 22.9f)));
    glUniform3fv(glGetUniformLocation(reflectShader.shaderProgram, "pointLight3"), 1,
                 glm::value_ptr(glm::vec3(1.6f, 9.8f, 14.5f)));


    myBasicShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "night"), night);
    grassShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(grassShader.shaderProgram, "night"), night);
    reflectShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(reflectShader.shaderProgram, "night"), night);
    skyboxShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(skyboxShader.shaderProgram, "night"), night);

    myBasicShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram,"fogOn"),0);
    reflectShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(reflectShader.shaderProgram,"fogOn"),0);
    grassShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(grassShader.shaderProgram,"fogOn"),0);

}

void grassAnim() {

    if (wind >= ws)
        veloc = -0.005;
    if (wind <= -1 * ws)
        veloc = 0.005;

    wind += veloc;
    wd.x = cos(windAngle);
    wd.y = sin(windAngle);
}

void rainAnim() {
    if (fall >= 10.0)
        fall = 0.0;
    fall += 2;
}

void waterAnim() {
    if (waterAngle == 360)
        waterAngle = -360;
    waterAngle += 0.05f;
}

void randomRain() {
    float LOx = myCamera.getCameraPosition().x - 7.0;
    float HIx = myCamera.getCameraPosition().x + 7.0;
    float LOz = myCamera.getCameraPosition().z - 7.0;
    float HIz = myCamera.getCameraPosition().z + 7.0;
    float randomX, randomZ;

    for (int i = 0; i < RAIN; i++) {
        glm::mat4 mdl = glm::mat4(1.0f);
        randomX = LOx + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIx - LOx)));
        randomZ = LOz + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIz - LOz)));


//        glm::vec3 dist = glm::vec3(randomX,myCamera.getCameraPosition().y,randomZ)-myCamera.getCameraPosition();
//        glm::vec3 dirA = glm::vec3(0.0f,0.0f,1.0f);
//        glm::vec3 dirB = glm::normalize(dist);
//        float angle = acos(glm::dot(dirA,dirB));

        float angle = 0;

        if (randomX > myCamera.getCameraPosition().x + 0.5) {
            angle = 90;
            if (randomZ > myCamera.getCameraPosition().z + 0.5)
                angle -= 45;
            else
                angle += 45;
        } else if (randomX < myCamera.getCameraPosition().x - 0.5) {
            angle = 270;
            if (randomZ > myCamera.getCameraPosition().z + 0.5)
                angle += 45;
            else
                angle -= 45;
        } else if (randomX < myCamera.getCameraPosition().x + 0.5 && randomX > myCamera.getCameraPosition().x - 0.5) {
            if (randomZ < myCamera.getCameraPosition().z - 0.5)
                angle = 180;
        }
        mdl = glm::translate(mdl, glm::vec3(randomX, myCamera.getCameraPosition().y + 10.0f, randomZ));
        mdl = glm::rotate(mdl, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

        mdlR[i] = mdl;
    }
}

void randomGrass() {
    float LOx = -35.5;
    float HIx = 35.5;
    float LOz = -35.5;
    float HIz = 35.5;
    float LOv = -1.0;
    float HIv = 1.0;
    float LOs = 0.7;
    float HIs = 1.3;
    float randomX, randomZ, y;

    for (int i = 0; i < GRASS; i++) {
        glm::mat4 mdl = glm::mat4(1.0f);

        randomX = LOx + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIx - LOx)));
        randomZ = LOz + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIz - LOx)));
        if ((randomX > 10.f && randomX < 20.f) && (randomZ > 10.f && randomZ < 20.f))
            y = -2000.0f;
        else
            y = 0.0f;

        variant[i] = LOv + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIv - LOv)));
        randomSize = LOs + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIs - LOs)));
        mdl = glm::translate(mdl, glm::vec3(randomX, y, randomZ));
        mdl = glm::scale(mdl, glm::vec3(randomSize, randomSize, randomSize));
        mdlG[i] = mdl;
    }

    for (int i = 0; i < TALL_GRASS; i++) {
        glm::mat4 mdl = glm::mat4(1.0f);

        randomX = LOx + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIz - LOx)));
        randomZ = LOz + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIz - LOx)));
        if ((randomX > 10.f && randomX < 20.f) && (randomZ > 10.f && randomZ < 20.f))
            y = -2000.0f;
        else
            y = 0.0f;

        variantT[i] = LOv + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIv - LOv)));
        randomSize = LOs + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIs - LOs)));
        mdl = glm::translate(mdl, glm::vec3(randomX, y, randomZ));
        mdl = glm::scale(mdl, glm::vec3(randomSize, randomSize * 0.7, randomSize));
        mdlTG[i] = mdl;
    }
}

void renderGrass(gps::Shader shader, int i) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(mdlG[i]));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));

    float var = variant[i];
    GLuint wsLoc = glGetUniformLocation(shader.shaderProgram, "wind");
    glUniform1f(wsLoc, var * wind);
    GLuint wdLoc = glGetUniformLocation(shader.shaderProgram, "windDir");
    glUniform2fv(wdLoc, 1, glm::value_ptr(wd));

    // draw teapot
    grass.Draw(shader);
}

void renderTallGrass(gps::Shader shader, int i) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(mdlTG[i]));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));

    float var = variantT[i];
    GLuint wsLoc = glGetUniformLocation(shader.shaderProgram, "wind");
    glUniform1f(wsLoc, var * wind);
    GLuint wdLoc = glGetUniformLocation(shader.shaderProgram, "windDir");
    glUniform2fv(wdLoc, 1, glm::value_ptr(wd));

    // draw teapot
    tallGrass.Draw(shader);
}

void renderRain(gps::Shader shader, int i) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader

    //send teapot normal matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(mdlR[i]));


    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));


    glUniform1f(glGetUniformLocation(shader.shaderProgram, "fall"), fall + i % 10);

    // draw teapot
    rain.Draw(shader);
    glCheckError();

}

void renderObject(gps::Shader shader, gps::Model3D &object) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));
    // draw teapot
    object.Draw(shader);
}

void renderObjectShadow(gps::Shader shader, gps::Model3D &object) {
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    object.Draw(shader);
}

void renderShadow(gps::Shader shader) {
    shader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(computeLightSpaceTrMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderObjectShadow(shader, bridge);
    renderObjectShadow(shader, ground);
    renderObjectShadow(shader, tree);
    renderObjectShadow(shader, leaves);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene() {

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_FRONT);
    renderShadow(shadowShader);
    glCullFace(GL_BACK);

    //render the scene
    gps::Shader shader = myBasicShader;

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(view));

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(computeLightSpaceTrMatrix()));

    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "lightDirMatrix"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat3(glm::inverseTranspose(view))));

    // render the teapot
    renderObject(myBasicShader, ground);
    renderObject(myBasicShader, bridge);
    renderObject(myBasicShader, tree);
    model = glm::translate(model, glm::vec3(carPoz, 0.0f, 0.0f));
    renderObject(myBasicShader, car);
    model = glm::mat4(1.0f);

    shader = grassShader;
    shader.useShaderProgram();
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(computeLightSpaceTrMatrix()));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(view));
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "lightDirMatrix"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat3(glm::inverseTranspose(view))));


    glDisable(GL_CULL_FACE);
    renderObject(grassShader, leaves);

    for (int i = 0; i < GRASS; i++)
        renderGrass(grassShader, i);
    for (int i = 0; i < TALL_GRASS; i++)
        renderTallGrass(grassShader, i);
    if (rainToggle > 0)
        for (int i = 0; i < RAIN; i++)
            renderRain(rainShader, i);
    glEnable(GL_CULL_FACE);

    debugDepthQuad.useShaderProgram();
    glUniform1f(glGetUniformLocation(debugDepthQuad.shaderProgram, "near_plane"), near_plane);
    glUniform1f(glGetUniformLocation(debugDepthQuad.shaderProgram, "near_plane"), far_plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    if (debug)
        renderQuad();

    shader = reflectShader;
    shader.useShaderProgram();
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(computeLightSpaceTrMatrix()));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(view));
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "lightDirMatrix"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat3(glm::inverseTranspose(view))));

    renderObject(reflectShader, mirror);
    model = glm::translate(model, glm::vec3(15.5f, -0.57, 16.5f));
    model = glm::rotate(model, glm::radians(waterAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-15.5f, +0.57, -16.5f));
    renderObject(reflectShader, water);
    model = glm::mat4(1.0f);
    mySkyBox.Draw(skyboxShader, view, projection);

}

void carAnim() {
    carPoz--;
    if (carPoz <= -100) {
        carPoz = 0;
        car_anim = false;
//        SoundEngine->stopAllSounds();
        SoundEngine->play2D("audio/wind.mp3", true);
    }
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char *argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    initOpenGLState();
    initFBOs();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    randomGrass();
    glCheckError();


    // application loop
    SoundEngine->play2D("audio/wind.mp3", true);
    while (!glfwWindowShouldClose(myWindow.getWindow())) {

        if (moveCalemra == true) {
            moveCamera();
        }

        processMovement();
        renderScene();


        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());
        grassAnim();
        waterAnim();
        if (rainToggle > 0) {
            randomRain();
            rainAnim();
        }
        if (car_anim) {
            if (carsnd == 0) {
//                SoundEngine->stopAllSounds();
                SoundEngine->play2D("audio/car.mp3", false);
                carsnd = 1;
            }
            carAnim();
        }
        glCheckError();

    }

    cleanup();

    return EXIT_SUCCESS;
}
