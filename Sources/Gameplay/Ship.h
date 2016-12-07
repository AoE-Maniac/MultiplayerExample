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
	void applyPosition(double time, Kore::vec3 remotePosition, bool smoothen);
	void update(double deltaTime, bool isVisible);

private:
	struct History {
		double time;
		int input;
	};

	int historyIndex;
	History* history;
	Kore::vec3 offset;
	RenderObject* renderObject;

	Kore::vec3 Ship::getHistoricPosition(double time);
};

