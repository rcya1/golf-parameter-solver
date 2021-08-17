#pragma once
class IndexBuffer {
public:
	unsigned int id;

	IndexBuffer(unsigned int size, void* data, int stride, int type);
	void free();
	void bind();
};
