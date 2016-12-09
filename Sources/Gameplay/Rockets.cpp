#include "../pch.h"

#include "Rockets.h"

#include <cassert>
#include "../Engine/Renderer.h"

using namespace Kore;

namespace {
	struct Rocket {
		vec3 pos;
		RenderObject* renderObject;
	};

	const int MAX_ROCKETS = 90;
	const float SCALING = 3.f;
	const float SPEED = 100.f;

	int currRockets;
	Rocket* rockets;
}

void initRockets() {
	rockets = new Rocket[MAX_ROCKETS];
	for (int i = 0; i < MAX_ROCKETS; ++i) {
		rockets[i].renderObject = addRenderObject(mat4::Identity(), "rocket.obj", "rocket.png");
		rockets[i].renderObject->isVisible = false;
	}
}

void deleteRockets() {
	delete[] rockets;
}

void fireRocket(vec3 pos) {
	assert(currRockets < MAX_ROCKETS);

	if (currRockets < MAX_ROCKETS) {
		rockets[currRockets].pos = pos;
		rockets[currRockets].renderObject->isVisible = true;
		++currRockets;
	}
}

void updateRockets(float deltaT) {
	for (int i = 0; i < currRockets; ++i) {
		rockets[i].pos += vec3(0, SPEED * deltaT, 0);
		rockets[i].renderObject->M = mat4::Translation(rockets[i].pos.x(), rockets[i].pos.y(), rockets[i].pos.z()) * mat4::Scale(SCALING, SCALING, SCALING);

		if (rockets[i].pos.y() > 10.0f) {
			--currRockets;
			rockets[i].pos = rockets[currRockets].pos;
			rockets[currRockets].renderObject->isVisible = false;
			--i;
		}
	}
}