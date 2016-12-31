#include "../pch.h"

#include "Rockets.h"

#include <cassert>
#include <cstdio>
#include <Kore/Math/Random.h>
#include "../Engine/Renderer.h"
#include "Rockets.h"

using namespace Kore;

namespace {
	struct Ufo {
		float lifeTime;
		float hitpoints;
		vec3 position;
		RenderObject* renderObject;
	};

	const int MAX_UFOS = 30;
	const float HITPOINTS_MAX = 5.f;
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
			ufos[currUfos].hitpoints = HITPOINTS_MAX;
			ufos[currUfos].position = pos;
			ufos[currUfos].renderObject->isVisible = true;
			++currUfos;
		}
	}

	void despawnUfo(int index) {
		--currUfos;
		ufos[index].lifeTime = ufos[currUfos].lifeTime;
		ufos[index].hitpoints = ufos[currUfos].hitpoints;
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
	pos.y() -= SPEED * timeOffset;
	spawnUfo(pos);
}

bool updateUfos(float deltaTime, bool canSpawn, vec3 &spawnPos) {
	bool didSpawn = false;
	spawnDelay -= deltaTime;

	if (canSpawn && spawnDelay <= 0) {
		spawnPos = vec3(getRand(spawnXMin, spawnXMax), spawnY, 0.f);
		spawnUfo(spawnPos);
		resetSpawnTimer();
		didSpawn = true;
	}

	for (int i = 0; i < currUfos; ++i) {
		ufos[i].lifeTime += deltaTime;
		ufos[i].position.y() -= SPEED * deltaTime;

		ufos[i].renderObject->M = mat4::Translation(ufos[i].position.x(), ufos[i].position.y(), ufos[i].position.z()) *
			mat4::RotationZ(ufos[i].lifeTime) *
			mat4::RotationX(0.5f * pi) *
			mat4::Scale(SCALING, SCALING, SCALING);

		// Check rocket collision
		// Since ufos are perfectly synced to the clients and rocket position converges fast
		// this check can be assumed to be correct enough on the client, no additional sync necessary
		// Score should be calculated only on the client though
		ufos[i].hitpoints -= checkRocketCollisions(ufos[i].position, 1.f * SCALING);

		// Left screen
		if (ufos[i].position.y() < -spawnY || ufos[i].hitpoints <= 0) {
			despawnUfo(i);
			--i;
		}
	}

	return didSpawn;
}