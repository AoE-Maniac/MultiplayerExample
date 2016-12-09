#pragma once

#include <Kore/Math/Vector.h>

void initRockets();
void deleteRockets();

void fireRocket(Kore::vec3 pos);
void updateRockets(float deltaTime);
