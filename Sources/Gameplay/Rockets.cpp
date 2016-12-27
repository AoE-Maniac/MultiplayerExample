#include "../pch.h"

#include "Rockets.h"

#include <cassert>
#include "../Engine/Renderer.h"

using namespace Kore;

namespace {
	struct Rocket {
		int id;
		vec3 pos;
		RenderObject* renderObject;
	};

	const int MAX_ROCKETS = 90;
	const float SCALING = 3.f;
	const float SPEED = 100.f;

	int lastId;
	int firstIndex;
	int currRockets;
	Rocket* rockets;
}

void initRockets() {
	lastId = 0;
	firstIndex = 0;
	currRockets = 0;

	rockets = new Rocket[MAX_ROCKETS];
	for (int i = 0; i < MAX_ROCKETS; ++i) {
		rockets[i].renderObject = addRenderObject(mat4::Identity(), "rocket.obj", "rocket.png");
		rockets[i].renderObject->isVisible = false;
	}
}

void deleteRockets() {
	delete[] rockets;
}

int fireRocket(vec3 pos) {
	assert(currRockets < MAX_ROCKETS);

	if (currRockets < MAX_ROCKETS) {
		int i = (firstIndex + currRockets) % MAX_ROCKETS;

		rockets[i].id = lastId++;
		rockets[i].pos = pos;
		rockets[i].renderObject->isVisible = true;
		++currRockets;

		return rockets[i].id;
	}
	return -1;
}

void updateRockets(float deltaT) {
	int i;
	for (int index = 0; index < currRockets; ++index) {
		i = (firstIndex + index) % MAX_ROCKETS;

		rockets[i].pos += vec3(0, SPEED * deltaT, 0);
		rockets[i].renderObject->M = mat4::Translation(rockets[i].pos.x(), rockets[i].pos.y(), rockets[i].pos.z()) * mat4::Scale(SCALING, SCALING, SCALING);

		if (rockets[i].pos.y() > 10.0f) {
			rockets[i].renderObject->isVisible = false;
			if (index == 0) {
				firstIndex = (firstIndex + 1) % MAX_ROCKETS;
				--currRockets;
				--index;
			}
		}
	}
}