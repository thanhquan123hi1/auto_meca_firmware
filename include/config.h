#pragma once

#include <Arduino.h>

// ============================================================
//  CAU HINH MANG (WiFi Access Point + UDP)
// ============================================================
#define WIFI_AP_SSID      "Mecanum-Car"
#define WIFI_AP_PASSWORD  "12345678"
#define WIFI_AP_CHANNEL   1
#define WIFI_AP_HIDDEN    0
#define WIFI_AP_MAX_CLIENTS 4
#define WIFI_AP_LOCAL_IP  IPAddress(192, 168, 4, 1)
#define WIFI_AP_GATEWAY   IPAddress(192, 168, 4, 1)
#define WIFI_AP_SUBNET    IPAddress(255, 255, 255, 0)
#define UDP_PORT          4210

// ============================================================
//  FAILSAFE
// ============================================================
// Android app dang gui lenh moi 100ms/l?n.
// 600ms du de chiu jitter nhe, nhung van dung xe nhanh khi mat ket noi.
#define COMMAND_TIMEOUT_MS  600UL

// ============================================================
//  PWM
// ============================================================
#define PWM_FREQUENCY         20000
#define PWM_RESOLUTION        8
#define MOTOR_SPEED_DEFAULT   120
#define ROTATE_SPEED_DEFAULT  165
#define FORWARD_DIAGONAL_SPEED 130

// Soft-start / soft-reverse de giam giat, giam sut ap.
#define PWM_RAMP_STEP         8
#define PWM_RAMP_TICK_MS      10UL

// ============================================================
//  DAO CHIEU RIENG TUNG BANH
//
// Dat true neu banh do dang quay nguoc so voi logic mong muon.
// Vi du: bam F ma banh sau quay nguoc thi co the dat RL/RR = true.
// ============================================================
#define FL_REVERSED  false
#define FR_REVERSED  false
#define RL_REVERSED  true
#define RR_REVERSED  true

// ============================================================
//  PIN DIEU KHIEN MOTOR - CAP NHAT THEO FILE "DI day.docx"
//
// So do hien tai cua xe:
// Driver 1:
//   INA -> GPIO17
//   INB -> GPIO16
//   INC -> GPIO5
//   IND -> GPIO4
// Driver 2:
//   INA -> GPIO14
//   INB -> GPIO13
//   INC -> GPIO18
//   IND -> GPIO15
//
// Mapping banh xe:
//   FL = Driver 1 Motor B = INC GPIO5,  IND GPIO4
//   FR = Driver 1 Motor A = INA GPIO17, INB GPIO16
//   RL = Driver 2 Motor B = INC GPIO18, IND GPIO15
//   RR = Driver 2 Motor A = INA GPIO14, INB GPIO13
// ============================================================

// --- Front Left (Driver 1 Motor B) ---
#define FL_IN1   5
#define FL_IN2   4

// --- Front Right (Driver 1 Motor A) ---
#define FR_IN1   17
#define FR_IN2   16

// --- Rear Left (Driver 2 Motor B) ---
#define RL_IN1   18
#define RL_IN2   15

// --- Rear Right (Driver 2 Motor A) ---
#define RR_IN1   14
#define RR_IN2   13

// ============================================================
//  KENH LEDC (PWM)
// ============================================================
#define FL_CH1   0
#define FL_CH2   1
#define FR_CH1   2
#define FR_CH2   3
#define RL_CH1   4
#define RL_CH2   5
#define RR_CH1   6
#define RR_CH2   7

// ============================================================
//  CAM BIEN SIEU AM HC-SR04
//
//  Echo -> GPIO12, Trig -> GPIO20.
//  ESP32 dong vai tro lop an toan tang thap: chan lenh tien khi
//  vat can o gan, lui ngan khi qua gan. Android van quyet dinh huong ne.
// ============================================================
#define ULTRASONIC_TRIG_PIN               20
#define ULTRASONIC_ECHO_PIN               12
#define ULTRASONIC_MEASURE_INTERVAL_MS    60UL
#define ULTRASONIC_TIMEOUT_US             30000UL   // ~500cm max range
#define ULTRASONIC_MIN_VALID_CM           2.0f      // duoi nguong nay coi la nhieu/loi
#define ULTRASONIC_AVOID_DISTANCE_CM      30.0f     // <= : chan tien (S)
#define ULTRASONIC_EMERGENCY_DISTANCE_CM  12.0f     // <= : lui ngan (B)
#define ULTRASONIC_MEDIAN_SAMPLES         5
#define EMERGENCY_REVERSE_MS              300UL
