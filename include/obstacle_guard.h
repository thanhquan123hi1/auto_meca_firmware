#pragma once

#include <Arduino.h>

void initObstacleGuard();
void updateObstacleGuard();
char guardCommand(char cmd);
bool isGuardActive();
float getGuardDistanceCm();
