#include "ultrasonic_sensor.h"

#include "config.h"

namespace {
constexpr float SOUND_SPEED_CM_PER_US = 0.0343f;

float sampleBuffer[ULTRASONIC_MEDIAN_SAMPLES] = {};
uint8_t sampleCount = 0;
uint8_t sampleIndex = 0;
float lastDistanceCm = -1.0f;
uint32_t lastMeasureAt = 0;

float medianOfSamples() {
  if (sampleCount == 0) {
    return -1.0f;
  }

  float sorted[ULTRASONIC_MEDIAN_SAMPLES] = {};
  for (uint8_t i = 0; i < sampleCount; ++i) {
    sorted[i] = sampleBuffer[i];
  }

  for (uint8_t i = 0; i < sampleCount; ++i) {
    for (uint8_t j = i + 1; j < sampleCount; ++j) {
      if (sorted[j] < sorted[i]) {
        const float tmp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = tmp;
      }
    }
  }

  if ((sampleCount % 2) == 1) {
    return sorted[sampleCount / 2];
  }

  const uint8_t upper = sampleCount / 2;
  const uint8_t lower = upper - 1;
  return (sorted[lower] + sorted[upper]) * 0.5f;
}

void pushSample(float distanceCm) {
  sampleBuffer[sampleIndex] = distanceCm;
  sampleIndex = (sampleIndex + 1) % ULTRASONIC_MEDIAN_SAMPLES;
  if (sampleCount < ULTRASONIC_MEDIAN_SAMPLES) {
    ++sampleCount;
  }
  lastDistanceCm = medianOfSamples();
}

void invalidateSamples() {
  sampleCount = 0;
  sampleIndex = 0;
  lastDistanceCm = -1.0f;
}

float measureDistanceCm() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  const unsigned long durationUs = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);
  if (durationUs == 0UL) {
    return -1.0f;
  }

  const float distanceCm = (static_cast<float>(durationUs) * SOUND_SPEED_CM_PER_US) * 0.5f;
  if (distanceCm < ULTRASONIC_MIN_VALID_CM) {
    return -1.0f;
  }
  return distanceCm;
}
} // namespace

void initUltrasonic() {
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  invalidateSamples();
  lastMeasureAt = 0;
}

void updateUltrasonic() {
  const uint32_t now = millis();
  if ((now - lastMeasureAt) < ULTRASONIC_MEASURE_INTERVAL_MS) {
    return;
  }
  lastMeasureAt = now;

  const float distanceCm = measureDistanceCm();
  if (distanceCm < 0.0f) {
    invalidateSamples();
    return;
  }

  pushSample(distanceCm);
}

float getDistanceCm() {
  return lastDistanceCm;
}

bool isObstacleClose() {
  const float distanceCm = getDistanceCm();
  return distanceCm > 0.0f && distanceCm <= ULTRASONIC_AVOID_DISTANCE_CM;
}

bool isObstacleEmergency() {
  const float distanceCm = getDistanceCm();
  return distanceCm > 0.0f && distanceCm <= ULTRASONIC_EMERGENCY_DISTANCE_CM;
}
