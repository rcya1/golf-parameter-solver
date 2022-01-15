#pragma once

namespace opengl {
class VertexArray2 {
 public:
  unsigned int id;

  VertexArray2();
  void free();
  void bind();
  void unbind();
};
}  // namespace opengl
