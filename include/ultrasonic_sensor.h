#pragma once

#include <Arduino.h>

// Khoi tao chan Trig/Echo cua HC-SR04.
void initUltrasonic();

// Do va cap nhat khoang cach moi nhat theo chu ky dinh truoc.
void updateUltrasonic();

// Tra ve khoang cach median moi nhat (cm). Tra -1.0f neu khong hop le / timeout.
float getDistanceCm();

// true neu co vat can o vung gan can phai chan tien.
bool isObstacleClose();

// true neu vat can qua gan, can lui ngan de giai toa.
bool isObstacleEmergency();
