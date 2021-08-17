#pragma once
class VertexBuffer {
private:
	int stride;
public:
	unsigned int id;

	VertexBuffer(unsigned int size, void* data, int stride, int type);
	void free();
	void bind();
	void setVertexAttribute(int index, int size, int type, int offset);
};
