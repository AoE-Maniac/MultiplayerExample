#include "../pch.h"

#include "Rockets.h"

#include <cassert>
#include <cstdio>
#include <Kore/Math/Random.h>
#include "../Engine/Renderer.h"

using namespace Kore;

namespace {
	struct Ufo {
		float lifeTime;
		vec3 position;
		RenderObject* renderObject;
	};

	const int MAX_UFOS = 30;
	const float SCALING = 30.f;
	const float SPEED = 50.f;
	const float SPAWNDELAY_MIN = 1.f;
	const float SPAWNDELAY_MAX = 3.f;

	int currUfos;
	Ufo* ufos;

	float spawnDelay;
	float spawnXMin, spawnXMax, spawnY;

	float getRand(float min, float max) {
		return Random::get(min * 1000, max * 1000) / 1000.f;
	}

	void resetSpawnTimer() {
		spawnDelay = getRand(SPAWNDELAY_MIN, SPAWNDELAY_MAX);
	}

	void spawnUfo(vec3 pos) {
		assert(currUfos < MAX_UFOS);

		if (currUfos < MAX_UFOS) {
			ufos[currUfos].lifeTime = 0;
			ufos[currUfos].position = pos;
			ufos[currUfos].renderObject->isVisible = true;
			++currUfos;
		}
	}

	void despawnUfo(int index) {
		--currUfos;
		ufos[index].position = ufos[currUfos].position;
		ufos[currUfos].renderObject->isVisible = false;
	}
}

void initUfos(float spwnXMin, float spwnXMax, float spwnY) {
	spawnXMin = spwnXMin;
	spawnXMax = spwnXMax;
	spawnY = spwnY;
	currUfos = 0;

	ufos = new Ufo[MAX_UFOS];
	for (int i = 0; i < MAX_UFOS; ++i) {
		ufos[i].renderObject = addRenderObject(mat4::Identity(), "ufo.obj", "ufo.png");
		ufos[i].renderObject->isVisible = false;
	}

	resetSpawnTimer();
}

void deleteUfos() {
	delete[] ufos;
}

void updateUfo(double timeOffset, vec3 pos) {
	/*if (lastId[player] > id) {
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
	}*/
}

void updateUfos(float deltaTime) {
	spawnDelay -= deltaTime;

	if (spawnDelay <= 0) {
		spawnUfo(vec3(getRand(spawnXMin, spawnXMax), spawnY, 0.f));
		resetSpawnTimer();
	}

	for (int i = 0; i < currUfos; ++i) {
		ufos[i].lifeTime += deltaTime;
		ufos[i].position.y() -= SPEED * deltaTime;

		ufos[i].renderObject->M = mat4::Translation(ufos[i].position.x(), ufos[i].position.y(), ufos[i].position.z()) *
			mat4::RotationZ(ufos[i].lifeTime) *
			mat4::RotationX(0.5f * pi) *
			mat4::Scale(SCALING, SCALING, SCALING);

		if (ufos[i].position.y() < -spawnY) {
			despawnUfo(i);
			--i;
		}
	}
}