#include "../pch.h"

#include "Rockets.h"

#include <cassert>
#include <cstdio>
#include "../Engine/Renderer.h"

using namespace Kore;

namespace {
	struct Rocket {
		int id;
		vec3 pos;
		RenderObject* renderObject;
	};

	const int MAX_ROCKETS_PER_PLAYER = 30;
	const float SCALING = 3.f;
	const float SPEED = 100.f;

	int lastId[3];
	int firstIndex[3];
	int currRockets[3];
	Rocket* rockets;
}

void initRockets() {
	for (int i = 0; i < 3; ++i) {
		lastId[i] = 0;
		firstIndex[i] = 0;
		currRockets[i] = 0;
	}

	rockets = new Rocket[MAX_ROCKETS_PER_PLAYER * 3];
	char texture[13];
	for (int player = 0; player < 3; ++player) {
		sprintf(texture, "player_%i.png", player);
		for (int i = 0; i < MAX_ROCKETS_PER_PLAYER; ++i) {
			rockets[player * MAX_ROCKETS_PER_PLAYER + i].renderObject = addRenderObject(mat4::Identity(), "rocket.obj", texture);
			rockets[player * MAX_ROCKETS_PER_PLAYER + i].renderObject->isVisible = false;
		}
	}
}

void deleteRockets() {
	delete[] rockets;
}

int fireRocket(vec3 pos, int player) {
	assert(currRockets[player] < MAX_ROCKETS_PER_PLAYER);

	if (currRockets[player] < MAX_ROCKETS_PER_PLAYER) {
		int i = player * MAX_ROCKETS_PER_PLAYER + (firstIndex[player] + currRockets[player]) % MAX_ROCKETS_PER_PLAYER;

		rockets[i].id = lastId[player]++;
		rockets[i].pos = pos;
		rockets[i].renderObject->isVisible = true;
		++currRockets[player];

		return rockets[i].id;
	}
	return -1;
}

void updateRockets(float deltaT) {
	int i;
	for (int player = 0; player < 3; ++player) {
		for (int index = 0; index < currRockets[player]; ++index) {
			i = player * MAX_ROCKETS_PER_PLAYER + (firstIndex[player] + index) % MAX_ROCKETS_PER_PLAYER;

			rockets[i].pos += vec3(0, SPEED * deltaT, 0);
			rockets[i].renderObject->M = mat4::Translation(rockets[i].pos.x(), rockets[i].pos.y(), rockets[i].pos.z()) * mat4::Scale(SCALING, SCALING, SCALING);

			if (rockets[i].pos.y() > 10.0f) {
				rockets[i].renderObject->isVisible = false;
				if (index == 0) {
					firstIndex[player] = (firstIndex[player] + 1) % MAX_ROCKETS_PER_PLAYER;
					--currRockets[player];
					--index;
				}
			}
		}
	}
}