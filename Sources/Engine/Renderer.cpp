#include "../pch.h"
#include "Renderer.h"

#include <cassert>

#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Shader.h>
#include <Kore/IO/FileReader.h>

#include "ObjLoader.h"

using namespace Kore;

namespace {
	struct MeshEntry {
		const char* name;
		Mesh* mesh;
	};

	struct TextureEntry {
		const char* name;
		Texture* texture;
	};

	const int MAX_OBJECTS = 100;
	const int MAX_MESHES = 10;
	const int MAX_TEXTURES = 10;

	Program* program;
	Shader* vertexShader;
	Shader* fragmentShader;

	VertexStructure* vertexStructures[2];
	VertexBuffer* vertexBufferModel;
	TextureUnit tex;

	mat4 view;
	mat4 projection;

	RenderObject* objects;
	int currObjects;

	MeshEntry* meshes;
	int currMeshes;

	TextureEntry* textures;
	int currTextures;

	void setVertex(float* data, int instanceIndex, int off, int size, float x, float y, float z, float u, float v) {
		int offset = off + instanceIndex * size;
		data[offset + 0] = x;
		data[offset + 1] = y;
		data[offset + 2] = z;
		data[offset + 3] = u;
		data[offset + 4] = v;
	}

	void setVertex(float* data, int instanceIndex, int off, int size, float x, float y, float z, float u, float v, float nx, float ny, float nz) {
		int offset = off + instanceIndex * size;
		data[offset + 0] = x;
		data[offset + 1] = y;
		data[offset + 2] = z;
		data[offset + 3] = u;
		data[offset + 4] = v;
		data[offset + 5] = nx;
		data[offset + 6] = ny;
		data[offset + 7] = nz;
	}

	void setVec2(float* data, int instanceIndex, int off, int size, vec2 v) {
		int offset = off + instanceIndex * size;
		data[offset + 0] = v.x();
		data[offset + 1] = v.y();
	}

	void setVec4(float* data, int instanceIndex, int off, int size, vec4 v) {
		int offset = off + instanceIndex * size;
		data[offset + 0] = v.x();
		data[offset + 1] = v.y();
		data[offset + 2] = v.z();
		data[offset + 3] = v.w();
	}

	void setMatrix(float* data, int instanceIndex, int off, int size, mat4 m) {
		int offset = off + instanceIndex * size;
		data[offset + 0] = m[0][0];
		data[offset + 1] = m[1][0];
		data[offset + 2] = m[2][0];
		data[offset + 3] = m[3][0];
		data[offset + 4] = m[0][1];
		data[offset + 5] = m[1][1];
		data[offset + 6] = m[2][1];
		data[offset + 7] = m[3][1];
		data[offset + 8] = m[0][2];
		data[offset + 9] = m[1][2];
		data[offset + 10] = m[2][2];
		data[offset + 11] = m[3][2];
		data[offset + 12] = m[0][3];
		data[offset + 13] = m[1][3];
		data[offset + 14] = m[2][3];
		data[offset + 15] = m[3][3];
	}
}

void initRenderer(mat4 v, mat4 p) {
	view = v;
	projection = p;

	objects = new RenderObject[MAX_OBJECTS];
	currObjects = 0;
	meshes = new MeshEntry[MAX_MESHES];
	currMeshes = 0;
	textures = new TextureEntry[MAX_TEXTURES];
	currTextures = 0;

	FileReader vs("meshes.vert");
	vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
	FileReader fs("meshes.frag");
	fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);

	vertexStructures[0] = new VertexStructure();
	vertexStructures[0]->add("pos", Float3VertexData);
	vertexStructures[0]->add("tex", Float2VertexData);
	vertexStructures[0]->add("nor", Float3VertexData);

	vertexStructures[1] = new VertexStructure();
	vertexStructures[1]->add("MVP", Float4x4VertexData);
	vertexStructures[1]->add("MIT", Float4x4VertexData);

	vertexBufferModel = new VertexBuffer(1, *vertexStructures[1]);

	program = new Program;
	program->setVertexShader(vertexShader);
	program->setFragmentShader(fragmentShader);
	program->link(vertexStructures, 2);

	tex = program->getTextureUnit("text");
	Graphics::setTextureAddressing(tex, U, Repeat);
	Graphics::setTextureAddressing(tex, V, Repeat);
}

void deleteRenderer() {
	delete program;
	delete vertexShader;
	delete fragmentShader;

	delete[] vertexStructures;

	delete[] objects;
	// TODO: Delete individual Meshes
	delete[] meshes;
	// TODO: Delete individual textures
	delete[] textures;
}

RenderObject* addRenderObject(mat4 M, const char* mesh, const char* texture) {
	assert(currObjects < MAX_OBJECTS);

	if (currObjects < MAX_OBJECTS) {
		// TODO: Use atlas
		objects[currObjects].M = M;
		objects[currObjects].mesh = loadObj(mesh, vertexStructures[0]);
		objects[currObjects].texture = new Texture(texture, true);

		++currObjects;

		return &objects[currObjects - 1];
	}
}

void renderObjects() {
	program->set();
	Graphics::setBlendingMode(SourceAlpha, InverseSourceAlpha);
	Graphics::setRenderState(BlendingState, true);
	Graphics::setRenderState(DepthTest, true);
	Graphics::setRenderState(DepthTestCompare, ZCompareLess);
	Graphics::setRenderState(DepthWrite, true);

	mat4 PV = projection * view;
	for (int i = 0; i < currObjects; ++i) {
		if (!objects[i].isVisible)
			continue;

		float* data = vertexBufferModel->lock();
		setMatrix(data, 0, 0, 32, PV * objects[i].M);
		setMatrix(data, 0, 16, 32, objects[i].M.Invert().Transpose());
		vertexBufferModel->unlock();

		VertexBuffer* vbs[2];
		vbs[0] = objects[i].mesh->vertexBuffer;
		vbs[1] = vertexBufferModel;

		Graphics::setTexture(tex, objects[i].texture);
		Graphics::setVertexBuffers(vbs, 2);
		Graphics::setIndexBuffer(*objects[i].mesh->indexBuffer);
		Graphics::drawIndexedVertices();
	}
}
