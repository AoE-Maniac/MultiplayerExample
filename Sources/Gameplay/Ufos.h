#pragma once

#include <Kore/Math/Vector.h>

void initUfos(float spawnXMin, float spawnXMax, float spawnY);
void deleteUfos();

void updateUfo(double timeOffset, Kore::vec3 pos);
bool updateUfos(float deltaTime, bool canSpawn, Kore::vec3 &spawnPos);
