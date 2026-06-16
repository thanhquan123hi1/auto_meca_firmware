#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "config.h"
#include "motor_control.h"

namespace {
WiFiUDP udp;
uint32_t lastCommandTime = 0;
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

void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);

  Serial.println();
  Serial.println("=== WiFi AP Started ===");
  Serial.print("SSID: ");
  Serial.println(WIFI_AP_SSID);
  Serial.print("Password: ");
  Serial.println(WIFI_AP_PASSWORD);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
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

  const bool ok = handleCommand(cmd);
  if (!ok) {
    applySafeStop("Command handler rejected cmd -> STOP");
    return;
  }

  lastCommandTime = millis();
  failsafeStopped = (cmd == 'S');
  lastAppliedCommand = cmd;

  Serial.print("CMD: ");
  Serial.print(cmd);
  Serial.print(" from ");
  Serial.print(udp.remoteIP());
  Serial.print(":");
  Serial.print(udp.remotePort());
  Serial.print(" size=");
  Serial.println(bytesRead);

  // Neu packet dai hon 1 byte thi van xu ly byte dau, nhung canh bao de debug.
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
  stopMotors();
  lastCommandTime = millis();
  failsafeStopped = true;
  lastAppliedCommand = 'S';

  startAccessPoint();
  startUdpServer();

  Serial.println("System ready. Waiting for UDP commands...");
}

void loop() {
  processUdp();
  updateMotors();
  handleFailsafe();
  delay(5);
}
