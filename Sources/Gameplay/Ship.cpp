#include "../pch.h"
#include "Ship.h"

#include <cassert>

#include <Kore/Log.h>
#include <Kore/System.h>

#include "../Engine/Renderer.h"
#include "Rockets.h"

using namespace Kore;

namespace {
	const float SPEED = 100;
	const float FIRERATE = 1.f;
	const int HISTORYSIZE = 10;

	float fireCooldown = 0.f;

	void unpackInput(int input, bool &left, bool &right, bool &fire) {
		fire = input & 4;
		left = input & 2;
		right = input & 1;
	}

	void updatePosition(vec3 &position, bool left, bool right, double time) {
		if (left) position -= vec3(time * SPEED, 0, 0);
		if (right) position += vec3(time * SPEED, 0, 0);
	}
}

Ship::Ship(vec3 startPos, const char* texture) {
	position = startPos;
	
	renderObject = addRenderObject(mat4::Translation(position.x(), position.y(), position.z()), "rocket.obj", texture);
	renderObject->isVisible = false;

	historyIndex = 0;
	history = new History[HISTORYSIZE];
	history[0].time = System::time();
	history[0].input = 0;
	for (int i = 1; i < HISTORYSIZE; ++i) {
		history[i].time = -1;
	}
}

Ship::~Ship() {
	delete[] history;
	// TODO: Remove and delete render object
}

vec3 Ship::getHistoricPosition(double time) {
	vec3 result = position + offset;

	bool left, right, fire;
	int offset = 0;
	int pos = historyIndex;
	double lastTime = System::time();
	// Fully revert all inputs after the time
	while (time < history[pos].time) {
		unpackInput(history[pos].input, left, right, fire);
		updatePosition(result, left, right, -(lastTime - history[pos].time));
		
		lastTime = history[pos].time;
		offset++;
		pos = (historyIndex - offset) % HISTORYSIZE;

		assert(offset < HISTORYSIZE);
	}
	// Partly revert the last inputs before the time
	unpackInput(history[pos].input, left, right, fire);
	updatePosition(result, left, right, -(lastTime - time));
	
	return result;
}

int Ship::getCurrentInput() {
	return history[historyIndex].input;
}

void Ship::applyInput(double time, int input) {
	if (input == history[historyIndex].input)
		return;

	bool left, right, fire;
	double elapsed = System::time() - time;
	// Undo recent movement
	unpackInput(history[historyIndex].input, left, right, fire);
	updatePosition(position, left, right, -elapsed);
	log(LogLevel::Info, "rolled back %f", elapsed);

	// Since we only get a time offset based on the ping there is no way to identify stray packets
	// This means that it is sufficient to use only the most recent input and not the full history
	historyIndex = (++historyIndex) % HISTORYSIZE;
	history[historyIndex].input = input;
	history[historyIndex].time = time;

	// Redo received movement
	unpackInput(history[historyIndex].input, left, right, fire);
	updatePosition(position, left, right, elapsed);
}

void Ship::applyPosition(double time, vec3 remotePosition) {
	// Suppress changes based on not yet transmitted input changes
	double sinceChange = time - history[historyIndex].time;
	if (sinceChange > 0 && sinceChange < 0.5f)
		return;

	// Calculate how much we are off
	vec3 diff = remotePosition - getHistoricPosition(time);
	offset = diff;
	// For immediate correction: position += diff;
}

void Ship::update(double deltaTime, bool isVisible) {
	bool left, right, fire;
	unpackInput(history[historyIndex].input, left, right, fire);

	updatePosition(position, left, right, deltaTime);

	fireCooldown -= deltaTime;
	if (fire && fireCooldown <= 0) {
		fireRocket(position);
		fireCooldown = FIRERATE;
	}

	// Alternative method
	//float change = 0.25 * deltaTime;
	//position += offset * change;
	//offset -= offset * change;
	if (offset.squareLength() < deltaTime * deltaTime) {
		position += offset;
		offset = vec3(0, 0, 0);
	}
	else {
		position += offset * (float)deltaTime;
		offset -= offset * (float)deltaTime;
	}

	renderObject->isVisible = isVisible;
	renderObject->M = mat4::Translation(position.x(), position.y(), position.z()) * mat4::Scale(5, 5, 5);
}
