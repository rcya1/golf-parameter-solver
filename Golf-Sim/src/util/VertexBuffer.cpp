#include "VertexBuffer.h"
#include <glad/glad.h>

VertexBuffer::VertexBuffer(unsigned int size, void* data, int stride, int type) {
	glGenBuffers(1, &this->id);
	glBindBuffer(GL_ARRAY_BUFFER, this->id);
	glBufferData(GL_ARRAY_BUFFER, size, data, type);
	this->stride = stride;
}

void VertexBuffer::free() {
	glDeleteBuffers(1, &this->id);
}

void VertexBuffer::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, this->id);
}

void VertexBuffer::setVertexAttribute(int index, int size, int type, int offset) {
	glVertexAttribPointer(index, size, type, GL_FALSE, this->stride, (void*) offset);
	glEnableVertexAttribArray(index);
}
