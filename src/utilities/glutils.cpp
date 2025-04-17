#include <glad/glad.h>
#include <program.hpp>
#include "glutils.h"
#include <vector>

template <class T>
unsigned int generateAttribute(int id, int elementsPerEntry, std::vector<T> data, bool normalize) {
    unsigned int bufferID;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(id, elementsPerEntry, GL_FLOAT, normalize ? GL_TRUE : GL_FALSE, sizeof(T), 0);
    glEnableVertexAttribArray(id);
    return bufferID;
}

unsigned int generateBuffer(Mesh &mesh) {
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    generateAttribute(0, 3, mesh.vertices, false);
    if (mesh.normals.size() > 0) {
        generateAttribute(1, 3, mesh.normals, true);
    }
    if (mesh.textureCoordinates.size() > 0) {
        generateAttribute(2, 2, mesh.textureCoordinates, false);
    }

    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    return vaoID;
}

unsigned int generateBuffer2(Mesh &mesh){
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    generateAttribute(0, 3, mesh.vertices, false);
    generateAttribute(1, 3, mesh.normals, true);
    generateAttribute(2, 2, mesh.textureCoordinates, false);
    
    GLuint eboIndices;
    glGenBuffers(1, &eboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    // Clean up (unbind VAO, VBOs, and EBO)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return vao;
}


