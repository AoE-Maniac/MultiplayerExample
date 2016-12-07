/*
.obj importer written by Florian Mehm and Robert Konrad
*/

#pragma once

#include <Kore/Graphics/Graphics.h>

struct Mesh {
	int numFaces;
	int numVertices;
	int numUVs;
	int numNormals;

	float* vertices;
	int* indices;
	float* uvs;
	float * normals;

	// very private
	float* curVertex;
	int* curIndex;
	float* curUV;
	float* curNormal;

	Kore::VertexBuffer* vertexBuffer;
	Kore::IndexBuffer* indexBuffer;
};

Mesh* loadObj(const char* filename, Kore::VertexStructure* vertexStructure);
