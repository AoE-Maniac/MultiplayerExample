#include "../pch.h"
#include "Ship.h"

#include "../Engine/Renderer.h"

using namespace Kore;

Ship::Ship(vec3 startPos, const char* texture) {
	position = startPos;
	
	renderObject = addRenderObject(mat4::Translation(position.x(), position.y(), position.z()), "rocket.obj", texture);
	renderObject->isVisible = false;
}

Ship::~Ship() { }

void Ship::update(float deltaTime, bool isVisible) {
	renderObject->isVisible = isVisible;
	renderObject->M = mat4::Translation(position.x(), position.y(), position.z()) * mat4::Scale(5, 5, 5);
}
