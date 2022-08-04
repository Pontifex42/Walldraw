#pragma once
#include "Arduino.h"

// Rotary encoder
#define PIN_ROT_A	39 // SVN
#define PIN_ROT_B	36 // SVP 
#define PIN_ROT_S	35

// Display 1302
#define PIN_I2C_SDA		21
#define PIN_I2C_SCL		22

// Servo
#define PIN_SERVO1_PULSE 25

// Overdischarge protection
#define PIN_CHECK_BATT	34

// Stepper
#define PIN_STEPPER_L_IN1 13
#define PIN_STEPPER_L_IN2 14
#define PIN_STEPPER_L_IN3 15
#define PIN_STEPPER_L_IN4 16

#define PIN_STEPPER_R_IN1 26
#define PIN_STEPPER_R_IN2 27
#define PIN_STEPPER_R_IN3 32
#define PIN_STEPPER_R_IN4 33

// Micro SD Card
#define PIN_SD_MOSI	23
#define PIN_SD_MISO	19
#define PIN_SD_CS	5
#define PIN_SD_SCK	18

// Buzzer
#define PIN_BUZZER	4

// Spare pins
#define PIN_SPARE_1	12
#define PIN_SPARE_2	17

#define DEBUG_PRINTLN(a) Serial.println(a)
//#define DEBUG_PRINTLN(a)
#define DEBUG_PRINT(a) Serial.print(a)
//#define DEBUG_PRINT(a)
