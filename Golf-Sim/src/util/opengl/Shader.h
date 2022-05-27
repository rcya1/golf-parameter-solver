#pragma once

#include "glm/glm.hpp"

namespace opengl {
class Shader {
 public:
  unsigned int id;

  Shader(const char* vertexPath, const char* fragmentPath,
         const char* geometryPath = nullptr);
  void load();
  void free();
  void activate() const;
  void setBool(const char* name, bool value) const;
  void setInt(const char* name, int value) const;
  void setFloat(const char* name, float value) const;
  void setVec2f(const char* name, float value1, float value2);
  void setVec2f(const char* name, glm::vec2& vec);
  void setVec3f(const char* name, float value1, float value2, float value3);
  void setVec3f(const char* name, glm::vec3& vec);
  void setVec4f(const char* name, float value1, float value2, float value3,
                float value4);
  void setMat3f(const char* name, bool transpose, const glm::f32* value);
  void setMat4f(const char* name, bool transpose, const glm::f32* value);

 private:
  const char* vertexPath;
  const char* fragmentPath;
  const char* geometryPath;
};
}  // namespace opengl
