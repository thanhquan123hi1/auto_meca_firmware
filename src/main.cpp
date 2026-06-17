#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "config.h"
#include "motor_control.h"
#include "obstacle_guard.h"
#include "telemetry_udp.h"

namespace {
WiFiUDP udp;
uint32_t lastCommandTime = 0;
uint32_t lastDistanceLogAt = 0;
bool failsafeStopped = false;
char lastAppliedCommand = 'S';

bool isValidCommand(char cmd) {
  const char normalized = static_cast<char>(toupper(static_cast<unsigned char>(cmd)));
  switch (normalized) {
    case 'F':
    case 'B':
    case 'Q':
    case 'E':
    case 'S':
    case 'L':
    case 'R':
    case 'G':
    case 'H':
    case 'J':
    case 'K':
      return true;
    default:
      return false;
  }
}

bool startAccessPoint() {
  constexpr uint8_t MAX_AP_RETRIES = 3;

  WiFi.persistent(false);
  WiFi.disconnect(true, true);
  delay(200);

  for (uint8_t attempt = 1; attempt <= MAX_AP_RETRIES; ++attempt) {
    WiFi.mode(WIFI_OFF);
    delay(120);
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);
    if (!WiFi.softAPConfig(WIFI_AP_LOCAL_IP, WIFI_AP_GATEWAY, WIFI_AP_SUBNET)) {
      Serial.println("WARN: softAPConfig failed, continuing with default AP IP");
    }
    delay(150);

    const bool ok = WiFi.softAP(
        WIFI_AP_SSID,
        WIFI_AP_PASSWORD,
        WIFI_AP_CHANNEL,
        WIFI_AP_HIDDEN,
        WIFI_AP_MAX_CLIENTS
    );
    delay(200);

    if (ok) {
      Serial.println();
      Serial.println("=== WiFi AP Started ===");
      Serial.print("Attempt: ");
      Serial.println(attempt);
      Serial.print("SSID: ");
      Serial.println(WIFI_AP_SSID);
      Serial.print("Password: ");
      Serial.println(WIFI_AP_PASSWORD);
      Serial.print("AP IP: ");
      Serial.println(WiFi.softAPIP());
      Serial.print("AP channel: ");
      Serial.println(WIFI_AP_CHANNEL);
      Serial.print("AP max clients: ");
      Serial.println(WIFI_AP_MAX_CLIENTS);
      Serial.print("WiFi mode: ");
      Serial.println(static_cast<int>(WiFi.getMode()));
      return true;
    }

    Serial.print("ERROR: softAP failed on attempt ");
    Serial.println(attempt);
    delay(300);
  }

  Serial.println("FATAL: WiFi AP failed after retries");
  return false;
}

void startUdpServer() {
  if (udp.begin(UDP_PORT)) {
    Serial.print("UDP server listening on port ");
    Serial.println(UDP_PORT);
  } else {
    Serial.print("ERROR: failed to start UDP on port ");
    Serial.println(UDP_PORT);
  }
}

void applySafeStop(const char* reason) {
  stopMotors();
  failsafeStopped = true;
  lastAppliedCommand = 'S';
  Serial.println(reason);
}

void processUdp() {
  const int packetSize = udp.parsePacket();
  if (packetSize <= 0) {
    return;
  }

  char buffer[16] = {0};
  const int bytesRead = udp.read(buffer, sizeof(buffer) - 1);
  if (bytesRead <= 0) {
    applySafeStop("UDP packet empty/read error -> STOP");
    return;
  }

  const char cmd = static_cast<char>(toupper(static_cast<unsigned char>(buffer[0])));
  if (!isValidCommand(cmd)) {
    Serial.print("Invalid UDP cmd: '");
    Serial.print(buffer[0]);
    Serial.println("' -> STOP");
    applySafeStop("Invalid command -> STOP");
    return;
  }

  rememberTelemetryPeer(udp.remoteIP(), udp.remotePort());

  const char safeCmd = guardCommand(cmd);
  const bool ok = handleCommand(safeCmd);
  if (!ok) {
    applySafeStop("Command handler rejected cmd -> STOP");
    return;
  }

  lastCommandTime = millis();
  failsafeStopped = (safeCmd == 'S');
  lastAppliedCommand = safeCmd;

  if (safeCmd != cmd) {
    Serial.print("GUARD: ");
    Serial.print(cmd);
    Serial.print(" -> ");
    Serial.println(safeCmd);
  }

  Serial.print("CMD: ");
  Serial.print(safeCmd);
  Serial.print(" from ");
  Serial.print(udp.remoteIP());
  Serial.print(":");
  Serial.print(udp.remotePort());
  Serial.print(" size=");
  Serial.println(bytesRead);

  if (bytesRead != 1) {
    Serial.print("WARN: expected 1-byte UDP command, got ");
    Serial.println(bytesRead);
  }
}

void handleFailsafe() {
  const uint32_t now = millis();
  if ((now - lastCommandTime) > COMMAND_TIMEOUT_MS) {
    if (!failsafeStopped) {
      applySafeStop("FAILSAFE timeout -> STOP");
    }
  }
}
} // namespace

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("Booting ESP32-S3 Mecanum UDP firmware...");

  initMotors();
  initObstacleGuard();
  stopMotors();
  lastCommandTime = millis();
  lastDistanceLogAt = 0;
  failsafeStopped = true;
  lastAppliedCommand = 'S';

  if (startAccessPoint()) {
    initTelemetry();
    startUdpServer();
    Serial.println("System ready. Waiting for UDP commands...");
  } else {
    Serial.println("System degraded: WiFi AP unavailable, UDP server not started.");
  }
}

void loop() {
  updateObstacleGuard();
  setTelemetryDistance(getGuardDistanceCm());
  setTelemetryGuardActive(isGuardActive());
  processUdp();
  updateTelemetry();
  updateMotors();
  handleFailsafe();

  const uint32_t now = millis();
  if ((now - lastDistanceLogAt) > 1000UL) {
    lastDistanceLogAt = now;
    Serial.print("DIST: ");
    Serial.print(getGuardDistanceCm(), 1);
    Serial.print(" cm | AP clients: ");
    Serial.println(WiFi.softAPgetStationNum());
  }

  delay(5);
}
