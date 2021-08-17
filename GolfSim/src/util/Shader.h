#pragma once

class Shader {
public:
	unsigned int id;

	Shader(const char *vertexPath, const char *fragmentPath);
	void free();
	void activate() const;
	void setBool(const char *name, bool value) const;
	void setInt(const char* name, int value) const;
	void setFloat(const char* name, float value) const;
	void setVec3f(const char* name, float value1, float value2, float value3);
	void setVec4f(const char* name, float value1, float value2, float value3, float value4);
	void setMat3f(const char* name, bool transpose, float* value);
	void setMat4f(const char* name, bool transpose, float* value);
};
