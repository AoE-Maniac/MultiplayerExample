#include "../pch.h"
#include "Ship.h"

#include "../Engine/Renderer.h"
#include <Kore/System.h>

using namespace Kore;

namespace {
	const float speed = 100;
	const int historySize = 10;
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

	// Undo recent movement
	double elapsed = System::time() - time;
	if (history[historyIndex].input & 2) position -= vec3(-elapsed * speed, 0, 0);
	if (history[historyIndex].input & 1) position += vec3(-elapsed * speed, 0, 0);

	// TODO: Use full history
	historyIndex = (++historyIndex) % historySize;
	history[historyIndex].input = input;
	history[historyIndex].time = time;

	// Redo received movement
	if (history[historyIndex].input & 2) position -= vec3(elapsed * speed, 0, 0);
	if (history[historyIndex].input & 1) position += vec3(elapsed * speed, 0, 0);
}

void Ship::update(double deltaTime, bool isVisible) {
	if (history[historyIndex].input & 2) position -= vec3(deltaTime * speed, 0, 0);
	if (history[historyIndex].input & 1) position += vec3(deltaTime * speed, 0, 0);

	renderObject->isVisible = isVisible;
	renderObject->M = mat4::Translation(position.x(), position.y(), position.z()) * mat4::Scale(5, 5, 5);
}
