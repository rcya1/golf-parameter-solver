#pragma once

namespace opengl {
class VertexArray {
 public:
  unsigned int id;

  VertexArray();
  void free();
  void bind();
  void unbind();
};
}  // namespace opengl
