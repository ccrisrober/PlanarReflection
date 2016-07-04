#pragma once
#include <GL/glew.h>
class RenderObject {
public:
	unsigned int _vao;
	unsigned int _size;

	virtual void render() {
		glBindVertexArray(_vao);
		glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0);
	}
	GLuint texture = -1;

	void bindTexture() {
		glBindTexture(GL_TEXTURE_2D, texture);
	}
};

