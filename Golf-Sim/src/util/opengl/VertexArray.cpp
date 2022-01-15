#include "VertexArray.h"

#include <glad/glad.h>

namespace opengl {
VertexArray2::VertexArray2() { glGenVertexArrays(1, &this->id); }

void VertexArray2::free() { glDeleteVertexArrays(1, &this->id); }

void VertexArray2::bind() { glBindVertexArray(id); }

void VertexArray2::unbind() { glBindVertexArray(0); }
}  // namespace opengl
