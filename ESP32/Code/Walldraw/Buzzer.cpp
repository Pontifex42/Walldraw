#include "PinConfig.h"
#include "Buzzer.h"

//#define DEBUG_BUZZER

#ifndef DEBUG_BUZZER

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif

esp_timer_handle_t timerBuzzer;
#define BEEP_TIME 90000

int BeepsToDo = 0;
void OnTimerBuzzer(void* arg)
{
	if (BeepsToDo <= 0)
	{
		BuzzerOff();
		return;
	}

	if (BeepsToDo % 2)
		BuzzerOff();
	else
		BuzzerOn();

	BeepsToDo--;
	if (BeepsToDo <= 0)
		return;

	esp_timer_stop(timerBuzzer);
	esp_timer_start_once(timerBuzzer, BEEP_TIME);
}

void BuzzerOn()
{
	DEBUG_PRINTLN("BuzzerOn");
	digitalWrite(PIN_BUZZER, HIGH);
}

void BuzzerOn(uint32_t ms)
{
	esp_timer_stop(timerBuzzer);
	BeepsToDo = 0;
	BuzzerOn();
	esp_timer_start_once(timerBuzzer, ms*1000);
}

void BuzzerOff()
{
	DEBUG_PRINTLN("BuzzerOff");
	digitalWrite(PIN_BUZZER, LOW);
}

void BuzzerAsyncBeeps(int beeps)
{
	if (beeps < 1)
	{
		BuzzerOff();
		BeepsToDo = 0;
		return;
	}
	esp_timer_stop(timerBuzzer);
	BeepsToDo = beeps * 2;
	BeepsToDo--;
	BuzzerOn();
	esp_timer_start_once(timerBuzzer, BEEP_TIME);
}

void SetupBuzzer()
{
	pinMode(PIN_BUZZER, OUTPUT);
	BeepsToDo = 0;
	BuzzerOff();

	esp_timer_create_args_t timerConfigBuzzer;
	timerConfigBuzzer.arg = NULL;
	timerConfigBuzzer.callback = OnTimerBuzzer;
	timerConfigBuzzer.dispatch_method = ESP_TIMER_TASK;
	timerConfigBuzzer.name = "Buzzer";

	if (esp_timer_create(&timerConfigBuzzer, &timerBuzzer) != ESP_OK)
	{
		DEBUG_PRINTLN("timerBuzzer create failed.");
	}
}
