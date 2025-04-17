#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/parser.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

SceneNode* rootNode;
SceneNode* lightNode;
SceneNode* cameraNode;
std::vector<SceneNode*> scenes;

glm::mat4 projection;

// These are heap allocated, because they should not be initialised at the start of the program
Gloom::Shader* shader;
Gloom::Shader* depthShader;

int currentScene = 0;
int currentAttribute = 0;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

double totalElapsedTime = 0;
float nearPlane = 0.1f;
float farPlane = 50.0f;

GLuint ssaoDepthTextureID;
GLuint ssaoFramebufferID;

GLuint mv_pos;
GLuint proj_pos;
GLuint norm_pos;
GLuint ldir_pos;
GLuint time_pos;
GLuint depthTexture_pos;
GLuint width_pos;
GLuint height_pos;
GLuint isWater_pos;
GLuint mainColor_pos;
GLuint secondaryColor_pos;
GLuint textureID_pos;

GLuint animateWater_pos;
GLuint notRenderDepth_pos;
GLuint distortion_pos;
GLuint notDistortAboveObjects_pos;
GLuint waterDepth_pos;
GLuint posterizeWater_pos;
GLuint foam_pos;
GLuint waterSpec_pos;
GLuint colorWaveHeight_pos;
GLuint smoothDiff_pos;
GLuint specular_pos;
GLuint rim_pos;

//depth shader
GLuint mv_depth_pos;
GLuint proj_depth_pos;

//shader settings
bool animateWater = true;
bool notRenderDepth = true;
bool distortion = true;
bool notDistortAboveObjects = true;
bool waterDepth = true;
bool posterizeWater = true;
bool foam = true;
bool waterSpec = true;
bool colorWaveHeight = true;
bool smoothDiff = true;
bool specular = true;
bool rim = true;

double mouseSensitivity = 10.0;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    double deltaX = x - width / 2;
    double deltaY = y - height / 2;


    if (mouseLeftPressed || mouseRightPressed){   
        rootNode->rotation += glm::vec3(0, mouseSensitivity * deltaX / width, 0);
        cameraNode->rotation += glm::vec3(mouseSensitivity * deltaY / height, 0, 0);
    }

    glfwSetCursorPos(window, width / 2, height / 2);
}

void cycleAttribute(int diff){
    currentAttribute += diff;
    if(currentScene == 0){        
        currentAttribute = currentAttribute < 0 ? 3 : currentAttribute % 4;
        smoothDiff = currentAttribute > 0;
        specular = currentAttribute > 1;
        rim = currentAttribute > 2;
    }else if (currentScene == 1) {
        currentAttribute = currentAttribute < 0 ? 8 : currentAttribute % 9;
        animateWater = currentAttribute > 6;
        notRenderDepth = currentAttribute != 1;
        distortion = currentAttribute > 4;
        notDistortAboveObjects = currentAttribute > 5;
        waterDepth = currentAttribute > 1;
        posterizeWater = currentAttribute > 2;
        foam = currentAttribute > 3;
        waterSpec = currentAttribute > 7;
        colorWaveHeight = currentAttribute > 6;
    }
}

void cycleScene(int delta){
    currentScene = (currentScene + delta) % scenes.size();
    currentAttribute = 0;
    for(auto scene : scenes){
        scene->active = false;
    }
    scenes[currentScene]->active = true;

    animateWater = false;
    notRenderDepth = true;
    distortion = false;
    notDistortAboveObjects = false;
    waterDepth = false;
    posterizeWater = false;
    foam = false;
    waterSpec = false;
    colorWaveHeight = false;
    smoothDiff = false;
    specular = false;
    rim = false;
    if(currentScene == 1){
        smoothDiff = true;
        specular = true;
        rim = true;
    }
    else if(currentScene == 2){
        animateWater = true;
        notRenderDepth = true;
        distortion = true;
        notDistortAboveObjects = true;
        waterDepth = true;
        posterizeWater = true;
        foam = true;
        waterSpec = true;
        colorWaveHeight = true;
        smoothDiff = true;
        specular = true;
        rim = true;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT) {
            cycleScene(-1);
        } else if (key == GLFW_KEY_RIGHT) {
            cycleScene(1);
        }
        else if (key == GLFW_KEY_UP) {
            cycleAttribute(1);
        }
        else if (key == GLFW_KEY_DOWN) {
            cycleAttribute(-1);
        }

    }
}

void createScenes(){
    Mesh sphere = generateSphere(1.0, 40, 40);
    Mesh surface = parseFileToMesh("surface.obj");
    Mesh ground = parseFileToMesh("ground_hole.obj");
    Mesh pirateShipSail = parseFileToMesh("pirate_ship_sail.obj");
    Mesh pirateShipBase = parseFileToMesh("pirate_ship_base.obj");
    Mesh pirateShipBase2 = parseFileToMesh("pirate_ship_base2.obj");
    Mesh birds = parseFileToMesh("birds.obj");
    Mesh isle = parseFileToMesh("isle.obj");
    Mesh palms = parseFileToMesh("palms.obj");
    Mesh palms1 = parseFileToMesh("palms1.obj");
    Mesh text = parseFileToMesh("text.obj");

    unsigned int sphereVAO = generateBuffer(sphere);
    unsigned int surfaceVAO = generateBuffer2(surface);
    unsigned int groundVAO = generateBuffer2(ground);
    unsigned int pirateShipSailVAO = generateBuffer2(pirateShipSail);
    unsigned int pirateShipBaseVAO = generateBuffer2(pirateShipBase);
    unsigned int pirateShipBase2VAO = generateBuffer2(pirateShipBase2);
    unsigned int birdsVAO = generateBuffer2(birds);
    unsigned int isleVAO = generateBuffer2(isle);
    unsigned int palmsVAO = generateBuffer2(palms);
    unsigned int palms1VAO = generateBuffer2(palms1);
    unsigned int textVAO = generateBuffer2(text);

    cameraNode = createSceneNode();
    rootNode = createSceneNode();

    cameraNode->children.push_back(rootNode);
    lightNode = createSceneNode();

    cameraNode->rotation = glm::vec3(0.5, 0, 0);
    rootNode->position = glm::vec3(0, -2, -4);
    lightNode->position = glm::vec3(.4, 1, .4);
    
    //Scene1
    SceneNode* scene1 = createSceneNode();
    scenes.push_back(scene1);
    SceneNode* s1_sphere = createSceneNode();
    rootNode->children.push_back(scene1);
    scene1->children.push_back(s1_sphere);
    s1_sphere->vertexArrayObjectID = sphereVAO;
    s1_sphere->VAOIndexCount       = sphere.indices.size();
    s1_sphere->material.mainColor = {0.416, 0.988, 0.165};

    //Scene3
    SceneNode* scene3 = createSceneNode();
    scenes.push_back(scene3);
    rootNode->children.push_back(scene3);

    glm::vec3 shipPos = {.8, -0.7, 0};
    glm::vec3 shipScl = glm::vec3(1.2);
    SceneNode* s3_ship_sail = createSceneNode();
    scene3->children.push_back(s3_ship_sail);
    s3_ship_sail->vertexArrayObjectID = pirateShipSailVAO;
    s3_ship_sail->VAOIndexCount       = pirateShipSail.indices.size();
    s3_ship_sail->material.mainColor = glm::vec3(.2);
    s3_ship_sail->position = shipPos;
    s3_ship_sail->scale = shipScl;

    SceneNode* s3_ship_base = createSceneNode();
    scene3->children.push_back(s3_ship_base);
    s3_ship_base->vertexArrayObjectID = pirateShipBaseVAO;
    s3_ship_base->VAOIndexCount       = pirateShipBase.indices.size();
    s3_ship_base->material.mainColor = {0.541, 0.243, 0.043};
    s3_ship_base->position = shipPos;
    s3_ship_base->scale = shipScl;

    SceneNode* s3_ship_base2 = createSceneNode();
    scene3->children.push_back(s3_ship_base2);
    s3_ship_base2->vertexArrayObjectID = pirateShipBase2VAO;
    s3_ship_base2->VAOIndexCount       = pirateShipBase2.indices.size();
    s3_ship_base2->material.mainColor = {1, 0.871, 0.565};
    s3_ship_base2->position = shipPos;
    s3_ship_base2->scale = shipScl;

    SceneNode* s3_birds = createSceneNode();
    scene3->children.push_back(s3_birds);
    s3_birds->vertexArrayObjectID = birdsVAO;
    s3_birds->VAOIndexCount       = birds.indices.size();
    s3_birds->material.mainColor = {.8, .8, .8};
    s3_birds->position = shipPos;
    s3_birds->scale = shipScl;

    SceneNode* s3_isle = createSceneNode();
    scene3->children.push_back(s3_isle);
    s3_isle->vertexArrayObjectID = isleVAO;
    s3_isle->VAOIndexCount       = isle.indices.size();
    s3_isle->scale = {4,4,4};
    s3_isle->material.mainColor = {1, 0.914, 0.498};

    SceneNode* s3_palms = createSceneNode();
    scene3->children.push_back(s3_palms);
    s3_palms->vertexArrayObjectID = palmsVAO;
    s3_palms->VAOIndexCount       = palms.indices.size();
    s3_palms->scale = {4,4,4};
    s3_palms->material.mainColor = {0.722, 0.463, 0.137};

    SceneNode* s3_palms1 = createSceneNode();
    scene3->children.push_back(s3_palms1);
    s3_palms1->vertexArrayObjectID = palms1VAO;
    s3_palms1->VAOIndexCount       = palms1.indices.size();
    s3_palms1->scale = {4,4,4};
    s3_palms1->material.mainColor = {0.369, 0.769, 0.169};

    SceneNode* s3_text = createSceneNode();
    scene3->children.push_back(s3_text);
    s3_text->vertexArrayObjectID = textVAO;
    s3_text->VAOIndexCount       = text.indices.size();
    s3_text->position = {2,-.6,0};
    s3_text->rotation = {0, 0, glm::radians(60.0)};
    s3_text->material.mainColor = {.7,.7,.7};

    SceneNode* s3_surface = createSceneNode();
    scene3->children.push_back(s3_surface);
    s3_surface->vertexArrayObjectID = surfaceVAO;
    s3_surface->VAOIndexCount = surface.indices.size();
    s3_surface->position = {0,-0.4,0};
    s3_surface->scale = {5, 1, 5};
    s3_surface->material = {{0.129, 0.082, 0.89}, {0.333, 0.965, 1}};
    s3_surface->nodeType = WATER;
}

void initGame(GLFWwindow* window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    depthShader = new Gloom::Shader();
    depthShader->makeBasicShader("../res/shaders/depth.vert", "../res/shaders/depth.frag");

    mv_pos = shader->getUniformFromName("modelview");
    proj_pos = shader->getUniformFromName("projection");
    norm_pos = shader->getUniformFromName("normal");
    ldir_pos = shader->getUniformFromName("lightDir");
    time_pos = shader->getUniformFromName("time");
    depthTexture_pos = shader->getUniformFromName("depthTexture");
    width_pos = shader->getUniformFromName("width");
    height_pos = shader->getUniformFromName("height");
    isWater_pos = shader->getUniformFromName("isWater");
    mainColor_pos = shader->getUniformFromName("mainColor");
    secondaryColor_pos = shader->getUniformFromName("secondaryColor");
    textureID_pos = shader->getUniformFromName("textureID");

    animateWater_pos = shader->getUniformFromName("animateWater");
    notRenderDepth_pos = shader->getUniformFromName("notRenderDepth");
    distortion_pos = shader->getUniformFromName("distortion");
    notDistortAboveObjects_pos = shader->getUniformFromName("notDistortAboveObjects");
    waterDepth_pos = shader->getUniformFromName("waterDepth");
    posterizeWater_pos = shader->getUniformFromName("posterizeWater");
    foam_pos = shader->getUniformFromName("foam");
    waterSpec_pos = shader->getUniformFromName("waterSpec");
    colorWaveHeight_pos = shader->getUniformFromName("colorWaveHeight");
    smoothDiff_pos = shader->getUniformFromName("smoothDiff");
    specular_pos = shader->getUniformFromName("specular");
    rim_pos = shader->getUniformFromName("rim");

    proj_depth_pos = depthShader->getUniformFromName("mvp");
    
    initDepthTexture(window);
    loadTextures();
    
    createScenes();
    cycleScene(1);
    cycleAttribute(-1);
}

void initDepthTexture(GLFWwindow* window) {
    glDepthRange(0.0, 1.0);
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glGenFramebuffers(1, &ssaoFramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFramebufferID);

    glGenTextures(1, &ssaoDepthTextureID);
    glBindTexture(GL_TEXTURE_2D, ssaoDepthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach depth textureID to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ssaoDepthTextureID, 0);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void loadTextures(){
    int texCnt = 4;
    GLuint textureID[texCnt];
    std::string paths[texCnt] = {"noiseTexture.png", "flag.png", "Z_kai_main1.png", "Z_kai_main5.png"};
    glGenTextures(texCnt, textureID);

    for (int i = 0; i < texCnt; ++i){
        PNGImage img = loadPNGFile("../res/textures/" + paths[i]);
        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_2D, textureID[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels.data());

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTextureUnit(1 + i, textureID[i]);
    }
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    //input handling
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }
    
    totalElapsedTime += timeDelta;

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if(!mouseLeftPressed && !mouseRightPressed){
        rootNode->rotation.y += 0.15 * timeDelta;
    }

    projection = glm::perspective(glm::radians(60.0f), float(width) / float(height), nearPlane, farPlane);
    updateNodeTransformations(cameraNode, glm::mat4(1.0f));
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentM = transformationThusFar * transformationMatrix;
    node->currentMV = node->currentM;

    if (node->vertexArrayObjectID != -1){
        node->currentTransInvM = glm::transpose(glm::inverse(glm::mat3(node->currentMV)));
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentM);
    }
}

void renderNode(SceneNode* node, bool depth) {
    if (!node->active || (node->nodeType == WATER && depth)) return;

    if (node->vertexArrayObjectID != -1){
        glUniform1f(isWater_pos, node->nodeType == WATER ? true : false);

        if (depth){
            glUniformMatrix4fv(proj_depth_pos, 1, GL_FALSE, glm::value_ptr(projection * node->currentMV));
        }else{
            glUniformMatrix4fv(mv_pos, 1, GL_FALSE, glm::value_ptr(node->currentMV));
            glUniformMatrix3fv(norm_pos, 1, GL_FALSE, glm::value_ptr(node->currentTransInvM));

            glUniform3fv(mainColor_pos, 1, glm::value_ptr(node->material.mainColor));
            if (node->nodeType == WATER){
                glUniform3fv(secondaryColor_pos, 1, glm::value_ptr(node->material.secondaryColor));
            }
            glUniform1f(textureID_pos, node->material.textureID);
        }

        glBindVertexArray(node->vertexArrayObjectID);
        glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
    }
 
    for(SceneNode* child : node->children) {
        renderNode(child, depth);
    }
}

void renderDepth(int width, int height){
    depthShader->activate();
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFramebufferID);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    renderNode(rootNode, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene(int width, int height){
    shader->activate();
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    glm::vec4 rootPos = cameraNode->currentMV * glm::vec4(rootNode->position, 1);
    glm::vec4 lightPos = rootNode->currentMV * glm::vec4(lightNode->position, 1);
    glm::vec3 ld = glm::normalize(glm::vec3(lightPos - rootPos));
    
    glUniform3fv(ldir_pos, 1, glm::value_ptr(ld));
    glUniform1f(time_pos, totalElapsedTime);

    glUniformMatrix4fv(proj_pos, 1, GL_FALSE, glm::value_ptr(projection));

    glBindTexture(GL_TEXTURE_2D, ssaoDepthTextureID);
    glUniform1i(depthTexture_pos, 0);
    glUniform1f(width_pos, width);
    glUniform1f(height_pos, height);
    
    glUniform1f(animateWater_pos, animateWater);
    glUniform1f(notRenderDepth_pos, notRenderDepth);
    glUniform1f(distortion_pos, distortion);
    glUniform1f(notDistortAboveObjects_pos, notDistortAboveObjects);
    glUniform1f(waterDepth_pos, waterDepth);
    glUniform1f(posterizeWater_pos, posterizeWater);
    glUniform1f(foam_pos, foam);
    glUniform1f(waterSpec_pos, waterSpec);
    glUniform1f(colorWaveHeight_pos, colorWaveHeight);
    glUniform1f(smoothDiff_pos, smoothDiff);
    glUniform1f(specular_pos, specular);
    glUniform1f(rim_pos, rim);

    renderNode(rootNode, false);
}

void renderFrame(GLFWwindow* window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    renderDepth(width, height);
    renderScene(width, height);
}
