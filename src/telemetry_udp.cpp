#include "telemetry_udp.h"

#include <WiFiUdp.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

namespace {
WiFiUDP telemetryUdp;
IPAddress peerAddress;
uint16_t peerPort = 0;
bool hasPeer = false;
float latestDistanceCm = -1.0f;
bool latestGuardActive = false;
uint32_t lastTelemetryAt = 0;

constexpr uint16_t TELEMETRY_PORT_OFFSET = 1;

void buildPayload(char* buffer, size_t size) {
  const float distanceCm = latestDistanceCm;
  snprintf(
      buffer,
      size,
      "T,%.1f,%d",
      static_cast<double>(distanceCm),
      latestGuardActive ? 1 : 0
  );
}
} // namespace

void initTelemetry() {
  telemetryUdp.begin(UDP_PORT + TELEMETRY_PORT_OFFSET);
  hasPeer = false;
  peerPort = 0;
  latestDistanceCm = -1.0f;
  latestGuardActive = false;
  lastTelemetryAt = 0;
}

void rememberTelemetryPeer(const IPAddress& address, uint16_t port) {
  peerAddress = address;
  peerPort = static_cast<uint16_t>(port + TELEMETRY_PORT_OFFSET);
  hasPeer = true;
}

void setTelemetryDistance(float distanceCm) {
  latestDistanceCm = distanceCm;
}

void setTelemetryGuardActive(bool active) {
  latestGuardActive = active;
}

void updateTelemetry() {
  if (!hasPeer) return;

  const uint32_t now = millis();
  if ((now - lastTelemetryAt) < 150UL) return;
  lastTelemetryAt = now;

  char payload[96] = {0};
  buildPayload(payload, sizeof(payload));
  telemetryUdp.beginPacket(peerAddress, peerPort);
  telemetryUdp.write(reinterpret_cast<const uint8_t*>(payload), strlen(payload));
  telemetryUdp.endPacket();
}
