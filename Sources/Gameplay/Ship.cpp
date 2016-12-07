#include "../pch.h"
#include "Ship.h"

#include <cassert>

#include <Kore/Log.h>
#include <Kore/System.h>

#include "../Engine/Renderer.h"

using namespace Kore;

namespace {
	const float speed = 100;
	const int historySize = 10;

	void unpackInput(int input, bool &left, bool &right) {
		left = input & 2;
		right = input & 1;
	}

	void updatePosition(vec3 &position, int input, double time) {
		bool left, right;
		unpackInput(input, left, right);
		if (left) position -= vec3(time * speed, 0, 0);
		if (right) position += vec3(time * speed, 0, 0);
	}
}

Ship::Ship(vec3 startPos, const char* texture) {
	position = startPos;
	
	renderObject = addRenderObject(mat4::Translation(position.x(), position.y(), position.z()), "rocket.obj", texture);
	renderObject->isVisible = false;

	historyIndex = 0;
	history = new History[historySize];
	history[0].time = System::time();
	history[0].input = 0;
	for (int i = 1; i < historySize; ++i) {
		history[i].time = -1;
	}
}

Ship::~Ship() {
	delete[] history;
	// TODO: Remove and delete render object
}

vec3 Ship::getHistoricPosition(double time) {
	vec3 result = position + offset;

	int offset = 0;
	int pos = historyIndex;
	double lastTime = System::time();
	// Fully revert all inputs after the time
	while (time < history[pos].time) {
		updatePosition(result, history[pos].input, -(lastTime - history[pos].time));
		
		lastTime = history[pos].time;
		offset++;
		pos = (historyIndex - offset) % historySize;

		assert(offset < historySize);
	}
	// Partly revert the last inputs before the time
	updatePosition(result, history[pos].input, -(lastTime - time));
	
	return result;
}

void Ship::applyInput(double time, int input) {
	if (input == history[historyIndex].input)
		return;

	double elapsed = System::time() - time;
	// Undo recent movement
	updatePosition(position, history[historyIndex].input, -elapsed);
	log(LogLevel::Info, "rolled back %f", elapsed);

	// Since we only get a time offset based on the ping there is no way to identify stray packets
	// This means that it is sufficient to use only the most recent input and not the full history
	historyIndex = (++historyIndex) % historySize;
	history[historyIndex].input = input;
	history[historyIndex].time = time;

	// Redo received movement
	updatePosition(position, history[historyIndex].input, elapsed);
}

void Ship::applyPosition(double time, vec3 remotePosition, bool smoothen) {
	// Suppress changes based on not yet transmitted input changes
	double sinceChange = time - history[historyIndex].time;
	if (sinceChange > 0 && sinceChange < 0.5f)
		return;

	// Calculate how much we are off
	vec3 diff = remotePosition - getHistoricPosition(time);
	if (smoothen) {
		offset = diff;
	}
	else {
		position += diff;
	}
}

void Ship::update(double deltaTime, bool isVisible) {
	updatePosition(position, history[historyIndex].input, deltaTime);
	
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
