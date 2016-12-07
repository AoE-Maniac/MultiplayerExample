#pragma once

#include <Kore/Math/Matrix.h>
#include <Kore/Graphics/VertexStructure.h>
#include <Kore/Graphics/Graphics.h>

struct RenderObject;

class Ship {
public:
	Ship(Kore::vec3 startPos, const char* texture);
	~Ship();

	Kore::vec3 position;

	void update(float deltaTime, bool left, bool right, bool isVisible);

private:
	RenderObject* renderObject;
};

