#pragma once

#include <Kore/Math/Matrix.h>
#include <Kore/Graphics/Texture.h>

struct Mesh;

struct RenderObject {
	bool isVisible;
	Kore::mat4 M;
	Mesh* mesh;
	Kore::Texture* texture;
};

void initRenderer(Kore::mat4 v, Kore::mat4 p);
void deleteRenderer();

RenderObject* addRenderObject(Kore::mat4 M, const char* mesh, const char* texture);
void renderObjects();
