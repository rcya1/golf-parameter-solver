#include "IndexBuffer.h"
#include <glad/glad.h>

IndexBuffer::IndexBuffer(unsigned int size, void* data, int type) {
	glGenBuffers(1, &this->id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, type);
}

void IndexBuffer::free() {
	glDeleteBuffers(1, &this->id);
}

void IndexBuffer::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

