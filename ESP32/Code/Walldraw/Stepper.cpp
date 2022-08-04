#include "Arduino.h"
#include "PinConfig.h"
#include "Stepper.h"
#include "SysData.h"

// #define DEBUG_STEPPER

#ifndef DEBUG_STEPPER
#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif
#endif

// depending on speed: Highest speed 10 leads to 1200 microseconds, which might be way too fast for the motors
// every speed level more increases the value by factor 1.4
//  10	   9     8    7     6     5     4      3      2     1
// 1500, 2100, 2940, 4116, 5762, 8067, 11294, 15812, 22136, 30991
#define FULL_SPEED_TIMING 1500

int MicroSecPerStep = 3292; // usual limit is 2-3ms per step, regarding to datasheet. Mine work still at 1,5ms.

void SetStepperTiming()
{
	int level = ReadSpeed();
	int inverseLevel = 10 - level;
	double factor = pow(1.4, inverseLevel);
	MicroSecPerStep = FULL_SPEED_TIMING * factor;
}

int ReelOutL = -1;
int ReelInL = 1;
int ReelOutR = 1;
int ReelInR = -1;

unsigned long lastStepTime_l = 0;
unsigned long lastStepTime_r = 0;

void ExecuteStep_l(int direction)
{
	static int stepPhase_l = 0;

	// compute the next phase number
	stepPhase_l += direction;

	if (stepPhase_l <= -1)
		stepPhase_l = 3;

	if (stepPhase_l >= 4)
		stepPhase_l = 0;

	while ((micros() - lastStepTime_l) < MicroSecPerStep)
		;

	// set the coils for this phase
	switch (stepPhase_l)
	{
	case 0:
		digitalWrite(PIN_STEPPER_L_IN1, LOW);
		digitalWrite(PIN_STEPPER_L_IN2, LOW);
		digitalWrite(PIN_STEPPER_L_IN3, HIGH);
		digitalWrite(PIN_STEPPER_L_IN4, HIGH);
		break;
	case 1:
		digitalWrite(PIN_STEPPER_L_IN1, LOW);
		digitalWrite(PIN_STEPPER_L_IN2, HIGH);
		digitalWrite(PIN_STEPPER_L_IN3, HIGH);
		digitalWrite(PIN_STEPPER_L_IN4, LOW);
		break;
	case 2:
		digitalWrite(PIN_STEPPER_L_IN1, HIGH);
		digitalWrite(PIN_STEPPER_L_IN2, HIGH);
		digitalWrite(PIN_STEPPER_L_IN3, LOW);
		digitalWrite(PIN_STEPPER_L_IN4, LOW);
		break;
	case 3:
		digitalWrite(PIN_STEPPER_L_IN1, HIGH);
		digitalWrite(PIN_STEPPER_L_IN2, LOW);
		digitalWrite(PIN_STEPPER_L_IN3, LOW);
		digitalWrite(PIN_STEPPER_L_IN4, HIGH);
		break;
	}
	lastStepTime_l = micros();
}

void ExecuteStep_r(int direction)
{
	static int stepPhase_r = 0;

	// compute the next phase number
	stepPhase_r += direction;

	if (stepPhase_r <= -1)
		stepPhase_r = 3;

	if (stepPhase_r >= 4)
		stepPhase_r = 0;

	while ((micros() - lastStepTime_r) < MicroSecPerStep)
		;

	// set the coils for this phase
	switch (stepPhase_r)
	{
	case 0:
		digitalWrite(PIN_STEPPER_R_IN1, LOW);
		digitalWrite(PIN_STEPPER_R_IN2, LOW);
		digitalWrite(PIN_STEPPER_R_IN3, HIGH);
		digitalWrite(PIN_STEPPER_R_IN4, HIGH);
		break;
	case 1:
		digitalWrite(PIN_STEPPER_R_IN1, LOW);
		digitalWrite(PIN_STEPPER_R_IN2, HIGH);
		digitalWrite(PIN_STEPPER_R_IN3, HIGH);
		digitalWrite(PIN_STEPPER_R_IN4, LOW);
		break;
	case 2:
		digitalWrite(PIN_STEPPER_R_IN1, HIGH);
		digitalWrite(PIN_STEPPER_R_IN2, HIGH);
		digitalWrite(PIN_STEPPER_R_IN3, LOW);
		digitalWrite(PIN_STEPPER_R_IN4, LOW);
		break;
	case 3:
		digitalWrite(PIN_STEPPER_R_IN1, HIGH);
		digitalWrite(PIN_STEPPER_R_IN2, LOW);
		digitalWrite(PIN_STEPPER_R_IN3, LOW);
		digitalWrite(PIN_STEPPER_R_IN4, HIGH);
		break;
	}

	lastStepTime_r = micros();
}

void MoveBothSteppers(int l, int r)
{
	long steps_abs_l = abs(l);
	long steps_abs_r = abs(r);
	long over = 0;

	for (long i = 0; i < steps_abs_l; ++i)
	{
		ExecuteStep_l(l < 0 ? ReelInL : ReelOutL);
		over += steps_abs_r;
		while (over >= steps_abs_l)
		{
			over -= steps_abs_l;
			ExecuteStep_r(r < 0 ? ReelInR : ReelOutR);
		}
	}
}

void DeactivateStepper()
{
	while ((micros() - lastStepTime_l) < MicroSecPerStep)
		;// Let motors finish their current step

	while ((micros() - lastStepTime_r) < MicroSecPerStep)
		;
	// Let motors finish their current step

	digitalWrite(PIN_STEPPER_L_IN1, LOW);
	digitalWrite(PIN_STEPPER_L_IN2, LOW);
	digitalWrite(PIN_STEPPER_L_IN3, LOW);
	digitalWrite(PIN_STEPPER_L_IN4, LOW);

	digitalWrite(PIN_STEPPER_R_IN1, LOW);
	digitalWrite(PIN_STEPPER_R_IN2, LOW);
	digitalWrite(PIN_STEPPER_R_IN3, LOW);
	digitalWrite(PIN_STEPPER_R_IN4, LOW);
}

void ReactivateStepper()
{
	ExecuteStep_l(0);
	ExecuteStep_r(0);
}

void InvertStepperRotation()
{
	ReelOutL = ReelOutL == 1 ? -1 : 1;
	ReelInL = ReelInL == 1 ? -1 : 1;
	ReelOutR = ReelOutR == 1 ? -1 : 1;
	ReelInR = ReelInR == 1 ? -1 : 1;
	SaveMotorRotation(ReelOutL, ReelInL, ReelOutR, ReelInR);
}

void SetupStepper()
{
	DEBUG_PRINTLN("SetupStepper");
	SetStepperTiming();

	ReadMotorRotation(ReelOutL, ReelInL, ReelOutR, ReelInR);

	pinMode(PIN_STEPPER_L_IN1, OUTPUT);
	pinMode(PIN_STEPPER_L_IN2, OUTPUT);
	pinMode(PIN_STEPPER_L_IN3, OUTPUT);
	pinMode(PIN_STEPPER_L_IN4, OUTPUT);

	pinMode(PIN_STEPPER_R_IN1, OUTPUT);
	pinMode(PIN_STEPPER_R_IN2, OUTPUT);
	pinMode(PIN_STEPPER_R_IN3, OUTPUT);
	pinMode(PIN_STEPPER_R_IN4, OUTPUT);

	DeactivateStepper();

	lastStepTime_l =
	lastStepTime_r = micros();
}
