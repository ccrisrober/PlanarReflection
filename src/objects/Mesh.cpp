#include "Mesh.h"
#include <vector>
#include "objload.h"
#include "../SOIL/SOIL.h"

Mesh::Mesh(std::string filename) : RenderObject()
{
	std::vector< float > vertices;
	std::vector< float > normals;
	std::vector< float > uvs;
	std::vector< unsigned int > facets;

	_readObj(filename, vertices, normals, uvs, facets);
	
	_size = facets.size();

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	std::vector< unsigned int > vbos;
	vbos.resize(4);
	glGenBuffers(4, vbos.data());

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertices.size(),
		vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* uvs.size(),
		uvs.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* normals.size(),
		normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* _size,
		facets.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	vertices.clear();
	normals.clear();
	uvs.clear();
	facets.clear();
}

void Mesh::_readObj(
	std::string& fileName_,
	std::vector< float >& vertices_,
	std::vector< float >& normals_,
	std::vector< float >& uvs_,
	std::vector< unsigned int >& facets_)
{
	obj::Model m = obj::loadModelFromFile(fileName_.c_str());
	vertices_ = m.vertex;
	normals_ = m.normal;
	uvs_ = m.texCoord;

	for (unsigned i = 0; i < m.faces["default"].size(); i++) {
		facets_.push_back(m.faces["default"][i]);
	}
}