#include "Shape.h"
#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;
#include <limits>


// copy the data from the shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;
	name = shape.name;				// ADDITION
}

void Shape::measure()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = (std::numeric_limits<float>::max)();
	maxX = maxY = maxZ = -(std::numeric_limits<float>::max)();

	//Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3*v+0] < minX) minX = posBuf[3 * v + 0];
		if (posBuf[3*v+0] > maxX) maxX = posBuf[3 * v + 0];

		if (posBuf[3*v+1] < minY) minY = posBuf[3 * v + 1];
		if (posBuf[3*v+1] > maxY) maxY = posBuf[3 * v + 1];

		if (posBuf[3*v+2] < minZ) minZ = posBuf[3 * v + 2];
		if (posBuf[3*v+2] > maxZ) maxZ = posBuf[3 * v + 2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
}

void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

	// Send the normal array to the GPU
	if (norBuf.empty())
	{
		cout << name << "no norms" << endl;
		norBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
	}

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::calcNorms() {
	//initialize vector to zeros
	norBuf.resize(posBuf.size());
	std::fill(norBuf.begin(), norBuf.end(), 0);

	// fill vertices array
	vector<vec3> vertices(posBuf.size() / 3);
	for (int s = 0; s < vertices.size(); s++) {
		float Xp = posBuf[s * 3];
		float Yp = posBuf[s * 3 + 1];
		float Zp = posBuf[s * 3 + 2];
		vertices[s] = vec3(Xp, Yp, Zp);
	}

	// calc Normals by computing 2 edges of a face, and cross them
	for (int i = 0; i < eleBuf.size() / 3; i++) {
		vec3 edge1 = vec3(
			(vertices[eleBuf[i * 3]].x - vertices[eleBuf[i * 3 + 1]].x),
			(vertices[eleBuf[i * 3]].y - vertices[eleBuf[i * 3 + 1]].y),
			(vertices[eleBuf[i * 3]].z - vertices[eleBuf[i * 3 + 1]].z));

		vec3 edge2 = vec3(
			(vertices[eleBuf[i * 3 + 1]].x - vertices[eleBuf[i * 3 + 2]].x),
			(vertices[eleBuf[i * 3 + 1]].y - vertices[eleBuf[i * 3 + 2]].y),
			(vertices[eleBuf[i * 3 + 1]].z - vertices[eleBuf[i * 3 + 2]].z));

		// calc the normal of the face created by v0, v1, v2
		vec3 normal = cross(edge1, edge2);

		// update vertex 0
		norBuf[3 * eleBuf[i * 3]] += normal.x;
		norBuf[3 * eleBuf[i * 3] + 1] += normal.y;
		norBuf[3 * eleBuf[i * 3] + 2] += normal.z;	

		// update vertex 1
		norBuf[3 * eleBuf[i * 3 + 1]] += normal.x;
		norBuf[3 * eleBuf[i * 3 + 1] + 1] += normal.y;
		norBuf[3 * eleBuf[i * 3 + 1] + 2] += normal.z;

		// update vertex 2
		norBuf[3 * eleBuf[i * 3 + 2]] += normal.x;
		norBuf[3 * eleBuf[i * 3 + 2] + 1] += normal.y;
		norBuf[3 * eleBuf[i * 3 + 2] + 2] += normal.z;
	}
}

void Shape::normalizeNorBuf() {
	for (int i = 0; i < norBuf.size() / 3; i++) {
		vec3 temp = normalize(vec3(norBuf[i * 3], norBuf[i * 3 + 1], norBuf[i * 3 + 2]));
		norBuf[i * 3] = temp.x;
		norBuf[i * 3 + 1] = temp.y;
		norBuf[i * 3 + 2] = temp.z;
	}
}

void Shape::reverseNormals() {
	for (int i = 0; i < norBuf.size() / 3; i++) {
		vec3 temp = -vec3(norBuf[i * 3], norBuf[i * 3 + 1], norBuf[i * 3 + 2]);
		norBuf[i * 3] = temp.x;
		norBuf[i * 3 + 1] = temp.y;
		norBuf[i * 3 + 2] = temp.z;
	}
}
