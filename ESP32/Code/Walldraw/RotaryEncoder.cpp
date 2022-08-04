#include "PinConfig.h"
#include "RotaryEncoder.h"
#include "Arduino.h"

// #define DEBUG_ROTARYENCODER

#ifndef DEBUG_ROTARYENCODER
#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif
#endif


int currentcount = 0;
portMUX_TYPE muxRotary = portMUX_INITIALIZER_UNLOCKED;
#define DEBOUNCE_BUTTON 10000
esp_timer_handle_t timerRotary; // for debouncing of rotary encoder button

#define IL	42	// illegal
#define CW	1	// Clockwise
#define CCW	-1	// Counter clockwise
#define CCW	-1	// Counter clockwise
#define NO	0	// No Move
//   old	 new
//	A	B	A	B	change
//	0	0	0	0	NO
//	0	0	0	1	CW
//	0	0	1	0	CCW
//	0	0	1	1	IL
//	0	1	0	0	CCW
//	0	1	0	1	NO
//	0	1	1	0	IL
//	0	1	1	1	CW
//	1	0	0	0	CW
//	1	0	0	1	IL
//	1	0	1	0	NO
//	1	0	1	1	CCW
//	1	1	0	0	IL
//	1	1	0	1	CCW
//	1	1	1	0	CW
//	1	1	1	1	NO
const int matrix[2][2][2][2] = { NO, CW, CCW, IL,
							CCW, NO, IL, CW,
							CW, IL, NO, CCW,
							IL, CCW, CW, NO };


volatile long posCnt = 0;
volatile int lastA;
volatile int lastB;
volatile int newA;
volatile int newB;
volatile bool buttonpress = false;

void ISR_OnRotaryA()
{
	newA = digitalRead(PIN_ROT_A);
	int inc = matrix[lastA][lastB][newA][newB];
	lastA = newA;
	if ((inc == CW) || (inc == CCW))
	{
		portENTER_CRITICAL(&muxRotary);
		posCnt += inc;
		portEXIT_CRITICAL(&muxRotary);
	}
}

void ISR_OnRotaryB()
{
	newB = digitalRead(PIN_ROT_B);
	int inc = matrix[lastA][lastB][newA][newB];
	lastB = newB;
	if ((inc == CW) || (inc == CCW))
	{
		portENTER_CRITICAL(&muxRotary);
		posCnt += inc;
		portEXIT_CRITICAL(&muxRotary);
	}
}

void ISR_OnRotarySwitch()
{
	esp_timer_stop(timerRotary);
	esp_timer_start_once(timerRotary, DEBOUNCE_BUTTON);
}

void OnTimerButton(void* arg)
{
	int tmp = digitalRead(PIN_ROT_S);
	portENTER_CRITICAL(&muxRotary);
	if (tmp == LOW) // level is low on button pressed
		buttonpress = true;
	portEXIT_CRITICAL(&muxRotary);
}

void SetupRotary()
{
	pinMode(PIN_ROT_A, INPUT);
	pinMode(PIN_ROT_B, INPUT);
	pinMode(PIN_ROT_S, INPUT);

	newA = lastA = digitalRead(PIN_ROT_A);
	newB = lastB = digitalRead(PIN_ROT_B);

	esp_timer_create_args_t timerConfigButton;
	timerConfigButton.arg = NULL;
	timerConfigButton.callback = OnTimerButton;
	timerConfigButton.dispatch_method = ESP_TIMER_TASK;
	timerConfigButton.name = "RotaryButton";

	if (esp_timer_create(&timerConfigButton, &timerRotary) != ESP_OK)
	{
		DEBUG_PRINTLN("timerButton create failed.");
	}

	attachInterrupt(digitalPinToInterrupt(PIN_ROT_A), ISR_OnRotaryA, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_ROT_B), ISR_OnRotaryB, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_ROT_S), ISR_OnRotarySwitch, CHANGE);
}

bool WasRotaryButtonPressed(bool reset)
{
	bool ret;
	portENTER_CRITICAL(&muxRotary);
	{
		ret = buttonpress;
		if (reset)
			buttonpress = false;
	}
	portEXIT_CRITICAL(&muxRotary);
	return ret;
}

#define INC_PER_STEP 2 // some encoders have 4 increments per step
int GetRotaryIncrement(bool reset)
{
	int diff;
	portENTER_CRITICAL(&muxRotary);
	{
		diff = posCnt - currentcount;
		diff /= INC_PER_STEP;
		if (diff && reset)
		{
			posCnt %= INC_PER_STEP;
			currentcount = 0;
		}
	}
	portEXIT_CRITICAL(&muxRotary);
	return diff;
}

void FlushRotaryEvents()
{
	GetRotaryIncrement(true);
	WasRotaryButtonPressed(true);
}
