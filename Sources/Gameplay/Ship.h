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

	int getCurrentInput();
	void applyInput(double time, int input);
	void applyPosition(double time, Kore::vec3 remotePosition);
	bool update(double deltaTime, bool isVisible, Kore::vec3 &firePos);

private:
	struct History {
		double time;
		int input;
	};

	float fireCooldown = 0.f;
	bool altFire = false;

	int historyIndex;
	History* history;
	Kore::vec3 offset;
	RenderObject* renderObject;

	Kore::vec3 Ship::getHistoricPosition(double time);
};

