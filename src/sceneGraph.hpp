#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stack>
#include <vector>
#include <cstdio>
#include <stdbool.h>
#include <cstdlib> 
#include <ctime> 
#include <chrono>
#include <fstream>

struct Material{
	glm::vec3 mainColor;
	glm::vec3 secondaryColor;
	int textureID;
};

enum SceneNodeType {
	NORMAL, WATER
};

struct SceneNode {
	SceneNode() {
		active = true;
		position = glm::vec3(0, 0, 0);
		rotation = glm::vec3(0, 0, 0);
		scale = glm::vec3(1, 1, 1);

        referencePoint = glm::vec3(0, 0, 0);
        nodeType = NORMAL;
        
		vertexArrayObjectID = -1;
        VAOIndexCount = 0;
		material = {{1,1,1}, {1,1,1}, 0};
	}
	bool active;

	std::vector<SceneNode*> children;
	
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	// The location of the node's reference point
	glm::vec3 referencePoint;

	// The ID of the VAO containing the "appearance" of this SceneNode.
	int vertexArrayObjectID;
	unsigned int VAOIndexCount;
	
	SceneNodeType nodeType;

	Material material;

	//Updated at runtime
	glm::mat4 currentM;
	glm::mat4 currentMV;
	glm::mat3 currentTransInvM;
};



SceneNode* createSceneNode();
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
int totalChildren(SceneNode* parent);