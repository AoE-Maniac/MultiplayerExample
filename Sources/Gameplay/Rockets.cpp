#include "../pch.h"

#include "Rockets.h"

#include <cassert>
#include <cstdio>
#include "../Engine/Renderer.h"

using namespace Kore;

namespace {
	struct Rocket {
		int id;
		vec3 position;
		vec3 offset;
		RenderObject* renderObject;
	};

	const int MAX_ROCKETS_PER_PLAYER = 30;
	const float SCALING = 3.f;
	const float SPEED = 300.f;

	float despawnHeight;

	int lastId[3];
	int firstIndex[3];
	int currRockets[3];
	Rocket* rockets;
}

void initRockets(float despawnHght) {
	despawnHeight = despawnHght;

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

int fireRocket(int player, vec3 pos) {
	assert(currRockets[player] < MAX_ROCKETS_PER_PLAYER);

	if (currRockets[player] < MAX_ROCKETS_PER_PLAYER) {
		int i = player * MAX_ROCKETS_PER_PLAYER + (firstIndex[player] + currRockets[player]) % MAX_ROCKETS_PER_PLAYER;

		rockets[i].id = lastId[player]++;
		rockets[i].position = pos;
		rockets[i].offset = vec3(0.f, 0.f, 0.f);
		rockets[i].renderObject->isVisible = true;
		++currRockets[player];

		return rockets[i].id;
	}
	return -1;
}

bool updateRocket(double timeOffset, int player, int id, vec3 pos) {
	if (lastId[player] > id) {
		// Update existing rocket
		int i = player * MAX_ROCKETS_PER_PLAYER + id % MAX_ROCKETS_PER_PLAYER;

		assert(rockets[i].id == id);
		if (rockets[i].id != id) return false;

		rockets[i].offset = rockets[i].position - (pos - vec3(0, SPEED * timeOffset, 0));
		return false;
	}
	else {
		// Fire new rocket
		assert(lastId[player] == id);

		fireRocket(player, pos - vec3(0, SPEED * timeOffset, 0));

		return true;
	}
}

void updateRockets(float deltaTime) {
	int i;
	for (int player = 0; player < 3; ++player) {
		for (int index = 0; index < currRockets[player]; ++index) {
			i = player * MAX_ROCKETS_PER_PLAYER + (firstIndex[player] + index) % MAX_ROCKETS_PER_PLAYER;

			if (rockets[i].offset.squareLength() < deltaTime * deltaTime) {
				rockets[i].position += rockets[i].offset;
				rockets[i].offset = vec3(0, 0, 0);
			}
			else {
				rockets[i].position += rockets[i].offset * (float)deltaTime;
				rockets[i].offset -= rockets[i].offset * (float)deltaTime;
			}

			rockets[i].position += vec3(0, SPEED * deltaTime, 0);
			rockets[i].renderObject->M = mat4::Translation(rockets[i].position.x(), rockets[i].position.y(), rockets[i].position.z()) *
				mat4::Scale(SCALING, SCALING, SCALING);

			if (rockets[i].position.y() > despawnHeight) {
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