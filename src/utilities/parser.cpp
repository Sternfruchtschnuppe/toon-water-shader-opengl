#include "parser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <unistd.h>

Mesh parseFileToMesh(const std::string& filename) {
    Mesh mesh;

    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);

    std::cout << std::string(buff) << std::endl;
    std::ifstream file("../res/models/" + filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: res/models/" + filename);
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> textureCoordinates;
    std::vector<glm::vec3> normals;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "v") {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (type == "vt") {
            glm::vec2 t;
            iss >> t.x >> t.y;
            textureCoordinates.push_back(t);
        } else if (type == "vn") {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        } else if (type == "f") {
            for (int i = 0; i < 3; i++){
                std::string v;
                iss >> v;
                // unsigned int idx = std::stoi(v.substr(0, v.find('/'))) - 1;
                // mesh.indices.push_back(idx);

                // mesh.vertices.push_back(vertices[idx]);
                // mesh.textureCoordinates.push_back(textureCoordinates[idx]);
                // mesh.normals.push_back(normals[idx]);
                
                std::stringstream ss(v);
                std::string token;
                std::getline(ss, token, '/');
                int vertexIndex = std::stoi(token) - 1;
                std::getline(ss, token, '/');
                int textureIndex = std::stoi(token) - 1;
                std::getline(ss, token, '/');
                int normalIndex = std::stoi(token) - 1;

                //why? I don't know either, but the shape's Mesh file is exactly the same
                mesh.indices.push_back(mesh.indices.size());

                mesh.vertices.push_back(vertices[vertexIndex]);
                mesh.textureCoordinates.push_back(textureCoordinates[textureIndex]);
                mesh.normals.push_back(normals[normalIndex]);
            }
        }
    }

    // std::cout << "vertices" << std::endl;
    // for (auto v : mesh.vertices){
    //     std::cout << v.x << "," << v.y << "," << v.z << std::endl;
    // }
    // std::cout << "texcoords" << std::endl;
    // for (auto v : mesh.textureCoordinates){
    //     std::cout << v.x << "," << v.y << std::endl;
    // }
    // std::cout << "normals" << std::endl;
    // for (auto v : mesh.normals){
    //     std::cout << v.x << "," << v.y << "," << v.z << std::endl;
    // }
    // std::cout << "indices" << std::endl;
    // for (auto v : mesh.indices){
    //     std::cout << v << std::endl;
    // }

    return mesh;
}
