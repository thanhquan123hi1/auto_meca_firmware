#include "motor_control.h"
#include "config.h"

namespace {

constexpr int MOTOR_FORWARD = 1;
constexpr int MOTOR_BACKWARD = -1;
constexpr int MOTOR_STOP = 0;

struct Motor {
  uint8_t pinForward;
  uint8_t pinBackward;
  uint8_t chForward;
  uint8_t chBackward;
  bool reversed;
};

struct WheelState {
  int direction = MOTOR_STOP;
  int appliedDirection = MOTOR_STOP;
  uint8_t targetDuty = 0;
  uint8_t currentDuty = 0;
};

enum WheelIndex {
  WHEEL_FL = 0,
  WHEEL_FR = 1,
  WHEEL_RL = 2,
  WHEEL_RR = 3,
  WHEEL_COUNT = 4,
};

Motor frontLeft  = {FL_IN1, FL_IN2, FL_CH1, FL_CH2, FL_REVERSED};
Motor frontRight = {FR_IN1, FR_IN2, FR_CH1, FR_CH2, FR_REVERSED};
Motor rearLeft   = {RL_IN1, RL_IN2, RL_CH1, RL_CH2, RL_REVERSED};
Motor rearRight  = {RR_IN1, RR_IN2, RR_CH1, RR_CH2, RR_REVERSED};

WheelState wheels[WHEEL_COUNT];
unsigned long lastRampTickAt = 0;

void setupMotor(const Motor& motor) {
  ledcSetup(motor.chForward, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcSetup(motor.chBackward, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(motor.pinForward, motor.chForward);
  ledcAttachPin(motor.pinBackward, motor.chBackward);
  ledcWrite(motor.chForward, 0);
  ledcWrite(motor.chBackward, 0);
}

int correctedDirection(int direction, bool reversed) {
  if (direction == MOTOR_STOP) {
    return MOTOR_STOP;
  }
  return reversed ? -direction : direction;
}

void driveMotor(const Motor& motor, int direction, uint8_t duty) {
  const int corrected = correctedDirection(direction, motor.reversed);

  if (corrected > 0) {
    ledcWrite(motor.chForward, duty);
    ledcWrite(motor.chBackward, 0);
  } else if (corrected < 0) {
    ledcWrite(motor.chForward, 0);
    ledcWrite(motor.chBackward, duty);
  } else {
    ledcWrite(motor.chForward, 0);
    ledcWrite(motor.chBackward, 0);
  }
}

void applyWheelHardware(WheelIndex index) {
  const WheelState& wheel = wheels[index];
  switch (index) {
    case WHEEL_FL:
      driveMotor(frontLeft, wheel.appliedDirection, wheel.currentDuty);
      break;
    case WHEEL_FR:
      driveMotor(frontRight, wheel.appliedDirection, wheel.currentDuty);
      break;
    case WHEEL_RL:
      driveMotor(rearLeft, wheel.appliedDirection, wheel.currentDuty);
      break;
    case WHEEL_RR:
      driveMotor(rearRight, wheel.appliedDirection, wheel.currentDuty);
      break;
    default:
      break;
  }
}

void setWheelTarget(WheelIndex index, int direction, uint8_t duty) {
  wheels[index].direction = direction;
  wheels[index].targetDuty = (direction == MOTOR_STOP) ? 0 : duty;
}

void driveMecanum(int fl, int fr, int rl, int rr, uint8_t duty) {
  setWheelTarget(WHEEL_FL, fl, duty);
  setWheelTarget(WHEEL_FR, fr, duty);
  setWheelTarget(WHEEL_RL, rl, duty);
  setWheelTarget(WHEEL_RR, rr, duty);
}

void stopImmediately() {
  for (int i = 0; i < WHEEL_COUNT; i++) {
    wheels[i].direction = MOTOR_STOP;
    wheels[i].appliedDirection = MOTOR_STOP;
    wheels[i].targetDuty = 0;
    wheels[i].currentDuty = 0;
    applyWheelHardware(static_cast<WheelIndex>(i));
  }
}

void rampTick() {
  const unsigned long now = millis();
  if ((now - lastRampTickAt) < PWM_RAMP_TICK_MS) {
    return;
  }
  lastRampTickAt = now;

  for (int i = 0; i < WHEEL_COUNT; i++) {
    WheelState& wheel = wheels[i];
    const bool needFlip = (wheel.direction != wheel.appliedDirection) && (wheel.direction != MOTOR_STOP);
    const uint8_t goalDuty = needFlip ? 0 : wheel.targetDuty;

    if (wheel.currentDuty < goalDuty) {
      const uint16_t next = static_cast<uint16_t>(wheel.currentDuty) + PWM_RAMP_STEP;
      wheel.currentDuty = (next > goalDuty) ? goalDuty : static_cast<uint8_t>(next);
    } else if (wheel.currentDuty > goalDuty) {
      const int next = static_cast<int>(wheel.currentDuty) - PWM_RAMP_STEP;
      wheel.currentDuty = (next < goalDuty) ? goalDuty : static_cast<uint8_t>(next);
    }

    if (wheel.currentDuty == 0) {
      wheel.appliedDirection = wheel.direction;
    }

    applyWheelHardware(static_cast<WheelIndex>(i));
  }
}

} // namespace

void initMotors() {
  setupMotor(frontLeft);
  setupMotor(frontRight);
  setupMotor(rearLeft);
  setupMotor(rearRight);
  stopImmediately();
}

void updateMotors() {
  rampTick();
}

void stopMotors() {
  stopImmediately();
}

void moveForward() {
  driveMecanum(MOTOR_FORWARD, MOTOR_FORWARD, MOTOR_FORWARD, MOTOR_FORWARD, MOTOR_SPEED_DEFAULT);
}

void moveBackward() {
  driveMecanum(MOTOR_BACKWARD, MOTOR_BACKWARD, MOTOR_BACKWARD, MOTOR_BACKWARD, MOTOR_SPEED_DEFAULT);
}

void rotateLeft() {
  driveMecanum(MOTOR_BACKWARD, MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_FORWARD, ROTATE_SPEED_DEFAULT);
}

void rotateRight() {
  driveMecanum(MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_FORWARD, MOTOR_BACKWARD, ROTATE_SPEED_DEFAULT);
}

void strafeLeft() {
  driveMecanum(MOTOR_BACKWARD, MOTOR_FORWARD, MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_SPEED_DEFAULT);
}

void strafeRight() {
  driveMecanum(MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_BACKWARD, MOTOR_FORWARD, MOTOR_SPEED_DEFAULT);
}

void forwardLeft() {
  driveMecanum(MOTOR_FORWARD, MOTOR_STOP, MOTOR_STOP, MOTOR_FORWARD, FORWARD_DIAGONAL_SPEED);
}

void forwardRight() {
  driveMecanum(MOTOR_STOP, MOTOR_FORWARD, MOTOR_FORWARD, MOTOR_STOP, FORWARD_DIAGONAL_SPEED);
}

void backwardLeft() {
  driveMecanum(MOTOR_STOP, MOTOR_BACKWARD, MOTOR_BACKWARD, MOTOR_STOP, MOTOR_SPEED_DEFAULT);
}

void backwardRight() {
  driveMecanum(MOTOR_BACKWARD, MOTOR_STOP, MOTOR_STOP, MOTOR_BACKWARD, MOTOR_SPEED_DEFAULT);
}

bool handleCommand(char cmd) {
  const char normalized = static_cast<char>(toupper(static_cast<unsigned char>(cmd)));

  switch (normalized) {
    case 'F': moveForward();   return true;
    case 'B': moveBackward();  return true;
    case 'Q': rotateLeft();    return true;
    case 'E': rotateRight();   return true;
    case 'S': stopMotors();    return true;
    case 'L': strafeLeft();    return true;
    case 'R': strafeRight();   return true;
    case 'G': forwardLeft();   return true;
    case 'H': forwardRight();  return true;
    case 'J': backwardLeft();  return true;
    case 'K': backwardRight(); return true;
    default:
      stopMotors();
      return false;
  }
}
