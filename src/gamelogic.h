#pragma once

#include <utilities/window.hpp>
#include "sceneGraph.hpp"

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar);
void initGame(GLFWwindow* window);
void initDepthTexture(GLFWwindow* window);
void loadTextures();
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);