#pragma once

#include <Arduino.h>
#include <IPAddress.h>

// ============================================================
//  TELEMETRY UDP (ESP32 -> Android)
//
//  Gui khoang cach HC-SR04 va trang thai guard ve dien thoai
//  qua mot goi UDP rieng, KHONG pha giao thuc lenh 1 byte.
//
//  Dinh dang payload (ASCII, de parse):
//      "T,<distanceCm>,<guardActive>"
//  Vi du: "T,27.4,1"  hoac  "T,-1.0,0" khi khong do duoc.
// ============================================================

// Khoi tao socket gui telemetry. Goi 1 lan trong setup().
void initTelemetry();

// Cap nhat va gui telemetry theo chu ky. Goi lien tuc trong loop().
void updateTelemetry();

// Ghi nho dia chi/port cua Android tu packet lenh gan nhat,
// de biet noi gui telemetry ve.
void rememberTelemetryPeer(const IPAddress& ip, uint16_t port);

// Cap nhat gia tri se gui di (goi truoc updateTelemetry()).
void setTelemetryDistance(float distanceCm);
void setTelemetryGuardActive(bool active);
