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

	void applyInput(double time, int input);
	void update(double deltaTime, bool isVisible);

private:
	struct History {
		double time;
		int input;
	};

	int historyIndex;
	History* history;
	RenderObject* renderObject;
};

