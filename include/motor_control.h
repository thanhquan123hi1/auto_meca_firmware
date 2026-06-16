#pragma once

#include <Arduino.h>

// ============================================================
//  API dieu khien xe Mecanum 4 banh.
//  Tat ca mapping huong nam trong motor_control.cpp.
// ============================================================

// Khoi tao chan + kenh PWM. Goi 1 lan trong setup(). Motor dung sau khi init.
void initMotors();

// Cap nhat ramp PWM. Goi lien tuc trong loop() de soft-start/soft-reverse hoat dong.
void updateMotors();

// Dung toan bo 4 motor ngay lap tuc.
void stopMotors();

// --- Cac lenh chinh (bat buoc chay dung) ---
void moveForward();    // F - tien
void moveBackward();   // B - lui
void rotateLeft();     // Q - xoay trai (tai cho)
void rotateRight();    // E - xoay phai (tai cho)

// --- Cac lenh legacy / mo rong ---
void strafeLeft();     // L - tinh tien sang trai
void strafeRight();    // R - tinh tien sang phai
void forwardLeft();    // G - cheo tien-trai
void forwardRight();   // H - cheo tien-phai
void backwardLeft();   // J - cheo lui-trai
void backwardRight();  // K - cheo lui-phai

// Phan giai 1 ky tu lenh ASCII -> goi ham tuong ung.
// Tra ve true neu lenh hop le, false neu khong hop le.
bool handleCommand(char cmd);
