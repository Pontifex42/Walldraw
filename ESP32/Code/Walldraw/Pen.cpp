#include "PinConfig.h"
#include <ESP32Servo.h>
#include "Pen.h"
#include "SysData.h"

//#define DEBUG_PEN

#ifndef DEBUG_PEN

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif


#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

Servo pen;
static int pen_state;
int pen_up_angle;
int pen_down_angle;

//------------------------------------------------------------------------------
void MoveSmooth(int current, int target)
{
	// SG90 runs 120 deg./s = 0,12 deg/ms.
	// 5 deg / 0.12 deg/ms = 42 ms
	// hopefully, using 60 ms makes movement slower
	// should be a multiply of 20, cause stepper PWM signal works at 50Hz
#define ANGLE_PER_STEP 5
#define SG90_DELAY 20
	int steps = abs((current - target) / ANGLE_PER_STEP);
	int increment = (current < target) ? ANGLE_PER_STEP : -ANGLE_PER_STEP;
		
	for (int i = 0; i < steps; ++i) // smooth movement
	{
		current += increment;
		pen.write(current);
		delay(SG90_DELAY);
	}
	if (target != current)
	{
		pen.write(target);
		delay(SG90_DELAY);
	}
}

void pen_down()
{
	if (pen_state == pen_down_angle)
		return; // Already down

	MoveSmooth(pen_state, pen_down_angle);

	pen_state = pen_down_angle;
	delay(SG90_DELAY * 5); // Time to end vibration
}

void pen_up()
{
	if (pen_state == pen_up_angle)
		return; // Already up

	MoveSmooth(pen_state, pen_up_angle);

	pen_state = pen_up_angle;
	delay(SG90_DELAY); // Time to end vibration
}

bool IsPenUp()
{
	return (pen_state == pen_up_angle);
}

int storedPenState = -1; // -1 means, not stored, 0 means, pen down, 1 means, pen up

void StorePenState()
{
	storedPenState = IsPenUp() ? 1 : 0;
	pen_up();
}

void RestorePenState()
{
	if (storedPenState == 0)
		pen_down();
	else if (storedPenState == 1)
		pen_up;
	storedPenState = -1;
}

const int minUs = 750;
const int maxUs = 2250;

void ServoGoToAngle(int angle)
{
	DEBUG_PRINTLN("ServoGoToAngle " + String(angle));
	//	Datasheet claims a speed of 60 degrees within 100ms
	pen.write(angle); // not waiting for finishing movement, the table rotation and color reading is slow enough
}

void DeactivateServo()
{
	// pen.detach(); better do not, otherwise servo may rotate to elesewhere
	 pinMode(PIN_SERVO1_PULSE, INPUT); // would that work? I may confuse the underlaying libraries ESP32Servo/ESP32_PWM/esp32-hal-ledc. And there is no pull-down resistor.
}

void ReactivateServo()
{
	pinMode(PIN_SERVO1_PULSE, OUTPUT);
}

void HandlePen()
{
	// Will not move servo async smoothely in this project, just synchrone
}

void SetupPen()
{
	pen_up_angle = ReadPenUpPos();
	pen_down_angle = ReadPenDownPos();
	ESP32PWM::allocateTimer(0);
	pen.setPeriodHertz(50); // standard 50 hz servo
	pen.attach(PIN_SERVO1_PULSE, minUs, maxUs);
	pen_state = pen_up_angle;
	pen.write(pen_state);
}


