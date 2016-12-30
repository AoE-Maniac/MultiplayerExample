#pragma once

#include <Kore/Math/Vector.h>

void initRockets();
void deleteRockets();

int fireRocket(int player, Kore::vec3 pos);
bool updateRocket(double timeOffset, int player, int id, Kore::vec3 pos);
void updateRockets(float deltaTime);
