/*
.obj importer written by Florian Mehm and Robert Konrad
used with permission
*/

#include "../pch.h"
#include "ObjLoader.h"
#include <Kore/IO/FileReader.h>
#include <cstring>
#include <cstdlib>

using namespace Kore;

namespace {
	char* tokenize(char* s, char delimiter, int& i) {
		int lastIndex = i;
		char* index = strchr(s + lastIndex + 1, delimiter);
		if (index == nullptr) {
			return nullptr;
		}
		int newIndex = (int)(index - s);
		i = newIndex;
		int length = newIndex - lastIndex;
		char* token = new char[length + 1];
		strncpy(token, s + lastIndex + 1, length);
		token[length] = 0;
		return token;
	}

	int countFirstCharLines(char* source, const char* start) {
		int count = 0;

		int index = 0;
		char* line = tokenize(source, '\n', index);

		while (line != nullptr) {
			char *pch = strstr(line, start);
			if (pch == line)
				count++;
			line = tokenize(source, '\n', index);
		}
		return count;
	}

	int countFacesInLine(char* line) {
		char* token = strtok(line, " ");
		int i = -1;
		while (token != nullptr) {
			token = strtok(nullptr, " ");
			i++;
		}

		// For now, handle tris and quads

		if (i == 3) {
			return 1;
		}
		else {
			return 2;
		}
	}

	int countFaces(char* source) {
		int count = 0;

		int index = 0;
		char* line = tokenize(source, '\n', index);

		while (line != nullptr) {
			if (line[0] == 'f') {
				count += countFacesInLine(line);
			}
			line = tokenize(source, '\n', index);
		}
		return count;
	}

	int countVertices(char* source) {
		return countFirstCharLines(source, "v ");
	}

	int countNormals(char* source) {
		return countFirstCharLines(source, "vn ");
	}

	int countUVs(char* source) {
		return countFirstCharLines(source, "vt ");
	}

	void parseVertex(Mesh* mesh, char* line) {
		char* token;
		for (int i = 0; i < 3; i++) {
			token = strtok(nullptr, " ");
			mesh->curVertex[i] = (float)strtod(token, nullptr);
		}

		mesh->curVertex += 3;
		mesh->curVertex[0] = 0;
		mesh->curVertex[1] = 0;
		mesh->curVertex += 5;

		mesh->numVertices++;
	}

	void setUV(Mesh* mesh, int index, float u, float v) {
		mesh->vertices[(index * 8) + 3] = u;
		mesh->vertices[(index * 8) + 4] = v;
	}

	void setNormal(Mesh* mesh, int index, float x, float y, float z) {
		mesh->vertices[(index * 8) + 5] = x;
		mesh->vertices[(index * 8) + 6] = y;
		mesh->vertices[(index * 8) + 7] = z;
	}

	void parseFace(Mesh* mesh, char* line) {
		char* token;
		int verts[4];
		int uvIndex[4];
		int normalIndex[4];
		bool hasUV[4];
		bool hasNormal[4];
		for (int i = 0; i < 3; i++) {
			token = strtok(nullptr, " ");
			char* endPtr;
			verts[i] = (int)strtol(token, &endPtr, 0) - 1;
			if (endPtr[0] == '/') {
				// Parse the uv
				hasUV[i] = true;
				uvIndex[i] = (int)strtol(endPtr + 1, &endPtr, 0) - 1;
			} else {
				// There is no uv
				hasUV[i] = false;
				hasNormal[i] = false;
			}
			if (endPtr[0] == '/') {
				hasNormal[i] = true;
				normalIndex[i] = (int)strtol(endPtr + 1, nullptr, 0) - 1;
			} 
		}
		token = strtok(nullptr, " ");
		//char* result;
		if (token != nullptr) {
			verts[3] = (int)strtol(token, nullptr, 0) - 1;
			// We have a quad
			mesh->curIndex[0] = verts[0];
			mesh->curIndex[1] = verts[1];
			mesh->curIndex[2] = verts[2];
			mesh->curIndex += 3;
			mesh->curIndex[0] = verts[2];
			mesh->curIndex[1] = verts[3];
			mesh->curIndex[2] = verts[0];
			mesh->curIndex += 3;
			mesh->numFaces += 2;
		}
		else {
			// We have a triangle
			for (int i = 0; i < 3; i++) {
				mesh->curIndex[i] = verts[i];

				if (!hasUV[i]) continue;

				// Set the UVs
				setUV(mesh, mesh->curIndex[i], mesh->uvs[uvIndex[i] * 2], mesh->uvs[(uvIndex[i] * 2) + 1]);

				if (!hasNormal[i]) continue;

				// Set the Normal
				setNormal(mesh, mesh->curIndex[i], mesh->normals[normalIndex[i] * 3], mesh->normals[normalIndex[i] * 3 + 1], mesh->normals[normalIndex[i] * 3 + 2]);
			}
			mesh->curIndex += 3;
			mesh->numFaces += 1;
		}
	}

	void parseUV(Mesh* mesh, char* line) {
		char* token;
		for (int i = 0; i < 2; i++) {
			token = strtok(nullptr, " ");
			*mesh->curUV = (float)strtod(token, nullptr);
			mesh->curUV++;
		}
	}

	void parseNormal(Mesh* mesh, char* line) {
		char* token;
		for (int i = 0; i < 3; i++) {
			token = strtok(nullptr, " ");
			*mesh->curNormal = (float)strtod(token, nullptr);
			mesh->curNormal++;
		}
	}

	void parseLine(Mesh* mesh, char* line) {
		char* token = strtok(line, " ");
		if (strcmp(token, "v") == 0) {
			// Read some vertex data
			parseVertex(mesh, line);
		}
		else if (strcmp(token, "f") == 0) {
			// Read some face data
			parseFace(mesh, line);
		}
		else if (strcmp(token, "vt") == 0) {
			parseUV(mesh, line);
		} else if (strcmp(token, "vn") == 0) {
			parseNormal(mesh, line);
		}

		// Ignore all other commands (for now)	
	}
}

Mesh* loadObj(const char* filename, VertexStructure* vertexStructure) {
	FileReader fileReader(filename, FileReader::Asset);
	void* data = fileReader.readAll();
	int length = fileReader.size() + 1;
	char* source = new char[length + 1];
	for (int i = 0; i < length; ++i) source[i] = reinterpret_cast<char*>(data)[i];
	source[length] = 0;
	
	Mesh* mesh = new Mesh;

	int vertices = countVertices(source);
	mesh->vertices = new float[vertices * 8];
	mesh->curVertex = mesh->vertices;
	int faces = countFaces(source);
	mesh->indices = new int[faces * 3];
	mesh->curIndex = mesh->indices;
	mesh->numUVs = countUVs(source);
	mesh->uvs = new float[mesh->numUVs * 2];
	mesh->curUV = mesh->uvs;
	int normals = countNormals(source);
	mesh->numNormals = normals;
	mesh->normals = new float[normals * 3];
	mesh->curNormal = mesh->normals;

	mesh->numVertices = 0;
	mesh->numFaces = 0;
	
	int index = 0;
	char* line = tokenize(source, '\n', index);
	
	int lineNumber = 0;
	while (line != nullptr) {
		parseLine(mesh, line);
		lineNumber++;
		delete[] line;
		line = tokenize(source, '\n', index);
	}
	
	mesh->vertexBuffer = new VertexBuffer(mesh->numVertices, *vertexStructure);
	float* vertData = mesh->vertexBuffer->lock();
	for (int i = 0; i < mesh->numVertices; ++i) {
		vertData[i * 8 + 0] = mesh->vertices[i * 8 + 0];
		vertData[i * 8 + 1] = mesh->vertices[i * 8 + 1];
		vertData[i * 8 + 2] = mesh->vertices[i * 8 + 2];
		vertData[i * 8 + 3] = mesh->vertices[i * 8 + 3];
		vertData[i * 8 + 4] = 1.0f - mesh->vertices[i * 8 + 4];
		vertData[i * 8 + 5] = mesh->vertices[i * 8 + 5];
		vertData[i * 8 + 6] = mesh->vertices[i * 8 + 6];
		vertData[i * 8 + 7] = mesh->vertices[i * 8 + 7];
	}
	mesh->vertexBuffer->unlock();
	
	mesh->indexBuffer = new IndexBuffer(mesh->numFaces * 3);
	int* indices = mesh->indexBuffer->lock();
	for (int i = 0; i < mesh->numFaces * 3; i++) {
		indices[i] = mesh->indices[i];
	}
	mesh->indexBuffer->unlock();

	return mesh;
}
