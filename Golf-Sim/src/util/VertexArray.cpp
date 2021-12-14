#include "VertexArray.h"

#include <glad/glad.h>

namespace opengl {
VertexArray::VertexArray() { glGenVertexArrays(1, &this->id); }

void VertexArray::free() { glDeleteVertexArrays(1, &this->id); }

void VertexArray::bind() { glBindVertexArray(id); }

void VertexArray::unbind() { glBindVertexArray(0); }
}  // namespace opengl
