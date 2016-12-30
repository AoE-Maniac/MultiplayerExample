#pragma once

#include <Kore/Math/Vector.h>

void initRockets();
void deleteRockets();

int fireRocket(Kore::vec3 pos, int player);
void updateRockets(float deltaTime);
