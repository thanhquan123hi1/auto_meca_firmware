#include "obstacle_guard.h"

#include "config.h"
#include "ultrasonic_sensor.h"

namespace {
bool guardActive = false;
bool emergencyReverse = false;
uint32_t emergencyReverseStartedAt = 0;

bool isForwardIntent(char cmd) {
  switch (static_cast<char>(toupper(static_cast<unsigned char>(cmd)))) {
    case 'F':
    case 'G':
    case 'H':
      return true;
    default:
      return false;
  }
}
} // namespace

void initObstacleGuard() {
  initUltrasonic();
  guardActive = false;
  emergencyReverse = false;
  emergencyReverseStartedAt = 0;
}

void updateObstacleGuard() {
  updateUltrasonic();
}

char guardCommand(char cmd) {
  const char normalized = static_cast<char>(toupper(static_cast<unsigned char>(cmd)));
  const uint32_t now = millis();

  if (emergencyReverse) {
    if ((now - emergencyReverseStartedAt) < EMERGENCY_REVERSE_MS) {
      guardActive = true;
      return 'B';
    }
    emergencyReverse = false;
  }

  if (!isForwardIntent(normalized)) {
    guardActive = false;
    return normalized;
  }

  if (isObstacleEmergency()) {
    emergencyReverse = true;
    emergencyReverseStartedAt = now;
    guardActive = true;
    return 'B';
  }

  if (isObstacleClose()) {
    guardActive = true;
    return 'S';
  }

  guardActive = false;
  return normalized;
}

bool isGuardActive() {
  return guardActive;
}

float getGuardDistanceCm() {
  return getDistanceCm();
}
