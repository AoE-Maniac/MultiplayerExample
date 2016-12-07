#include "../pch.h"
#include "Ship.h"

#include "../Engine/Renderer.h"
#include <Kore/System.h>

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

void Ship::applyInput(double time, int input) {
	if (input == history[historyIndex].input)
		return;

	double elapsed = System::time() - time;
	// Undo recent movement
	updatePosition(position, history[historyIndex].input, -elapsed);

	// TODO: Use full history
	historyIndex = (++historyIndex) % historySize;
	history[historyIndex].input = input;
	history[historyIndex].time = time;

	// Redo received movement
	updatePosition(position, history[historyIndex].input, elapsed);
}

void Ship::update(double deltaTime, bool isVisible) {
	updatePosition(position, history[historyIndex].input, deltaTime);

	renderObject->isVisible = isVisible;
	renderObject->M = mat4::Translation(position.x(), position.y(), position.z()) * mat4::Scale(5, 5, 5);
}
