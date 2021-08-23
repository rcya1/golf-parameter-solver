#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "../util/VertexArray.h"
#include "../util/VertexBuffer.h"
#include "../util/IndexBuffer.h"

namespace sphere {
    VertexArray *sphereVertexArray;
    VertexBuffer *sphereVertexBuffer;
    IndexBuffer *sphereIndexBuffer;

    const float PI = 3.14159265f;

    const int SECTOR_COUNT = 36;
    const int STACK_COUNT = 18;

    const float SECTOR_STEP = 2 * PI / SECTOR_COUNT;
    const float STACK_STEP = PI / STACK_COUNT;

    std::vector<float> vertexData;
    std::vector<unsigned int> indexData;

    // returns the VAO ID
    // Credit to http://www.songho.ca/opengl/gl_sphere.html
    VertexArray* generateSphereModel(float radius) {
        vertexData.clear();
        indexData.clear();

        for(int i = 0; i <= STACK_COUNT; i++) {
            float stackAngle = PI / 2 - i * STACK_STEP;
            float xy = radius * cosf(stackAngle);
            float z = radius * sinf(stackAngle);

            int k1 = i * (SECTOR_COUNT + 1);
            int k2 = k1 + SECTOR_COUNT + 1;

            for(int j = 0; j <= SECTOR_COUNT; j++, k1++, k2++) {
                float sectorAngle = j * SECTOR_STEP;

                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                // position
                vertexData.push_back(x);
                vertexData.push_back(y);
                vertexData.push_back(z);

                // normal
                vertexData.push_back(x / radius);
                vertexData.push_back(y / radius);
                vertexData.push_back(z / radius);

                // indices
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if(i != 0) {
                    indexData.push_back(k1);
                    indexData.push_back(k2);
                    indexData.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if(i != (STACK_COUNT - 1)) {
                    indexData.push_back(k1 + 1);
                    indexData.push_back(k2);
                    indexData.push_back(k2 + 1);
                }
            }
        }

        sphereVertexArray = new VertexArray();
        sphereVertexArray->bind();

        sphereVertexBuffer = new VertexBuffer(vertexData.size() * sizeof(float), vertexData.data(), 6 * sizeof(float), GL_STATIC_DRAW);
        sphereVertexBuffer->setVertexAttribute(0, 3, GL_FLOAT, 0); // position
        sphereVertexBuffer->setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float)); // normal

        sphereIndexBuffer = new IndexBuffer(indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

        return sphereVertexArray;
    }

    void freeSphereModel() {
        sphereIndexBuffer->free();
        sphereVertexBuffer->free();
        sphereVertexArray->free();

        delete sphereIndexBuffer;
        delete sphereVertexBuffer;
        delete sphereVertexArray;
    }
}
