#include "Arduino.h"
#include "BatteryProtect.h"
#include "PinConfig.h"
#include "OLED_Display.h"
#include "MenuHandler.h"
#include "Pen.h"
#include "Sledge.h"
#include "Buzzer.h"

//#define DEBUG_BATTERYPROTECT

#ifndef DEBUG_BATTERYPROTECT

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif

String BatteryLine = "";
int BatteryPercentage = 0;

portMUX_TYPE muxBattery = portMUX_INITIALIZER_UNLOCKED;

// Voltage divider is 100k and 22k, that is a factor of 22/122 = 0,18
// at 3.3V, there should be 4096 digits measured, which is 0,8mV per digit
// 0,8/0,18 is 4,48mV per digit
// measured readings at precise voltage:
// 12,28V measured 2559 digits -> 4,8 mV per digit
// 9,60V measured 1968 digits -> 4,88mV per digit

#define BATTERY_CHECK_INTERVALL 500000  // 2 times a second will do nicely
volatile bool BatteryAlarmState = false;
int batteryLowCounter = 0;
// // Switch off at 3*3.27V = 9.8V. cause in a serial connection one cell might be deep discharged as other cells are still working at up to 3.8V
const double FreshVoltage = 3 * 4.19; // three full batterys, 4,19V each, add to 12,57V
const double EmptyVoltage = 3 * 3.4; // = 10.2V. Below 3.4 V, LiIo does not have reliable content. 
const double ThresholdVoltage = 9.8; // here, we give a warning and shut motors down
const double VoltageRange = FreshVoltage - EmptyVoltage;
#define THRESHOLD_VALUE 2024 // = 9,8V
#define HYSTERESIS_VALUE 2231 // =10,8V. Come back at 10,8V
#define VOLT_PER_DIGIT 0.00484

bool IsBatteryMeasuring = false;
uint16_t measuredBattery = THRESHOLD_VALUE;

bool GetBatteryAlarmState()
{
	bool ret;
	portENTER_CRITICAL(&muxBattery);
	ret = BatteryAlarmState;
	portEXIT_CRITICAL(&muxBattery);
	return ret;
}

void SetBatteryAlarmState(bool state)
{
	portENTER_CRITICAL(&muxBattery);
	BatteryAlarmState = state;
	portEXIT_CRITICAL(&muxBattery);
}

void OnTimerBattery(void* arg)
{
	measuredBattery = analogRead(PIN_CHECK_BATT);
	DEBUG_PRINT(measuredBattery);
	DEBUG_PRINT("   ");
	if (GetBatteryAlarmState())
	{	// needs to be a certain time above a higher voltage to return to non-alarm-state
		if (measuredBattery > HYSTERESIS_VALUE)
			batteryLowCounter--;
		else
			batteryLowCounter = 10;

		if (batteryLowCounter <= 0)
		{
			batteryLowCounter = 0;
			SetBatteryAlarmState(false);
		}
	}
	else
	{
		if (measuredBattery < THRESHOLD_VALUE)
			batteryLowCounter++;
		else
			batteryLowCounter = 0;

		if (batteryLowCounter >= 10) // one second with undervoltage
		{
			batteryLowCounter = 10;
			SetBatteryAlarmState(true);
		}
	}
	DEBUG_PRINTLN(batteryLowCounter);
}

// this is on main-loop level
long lastTimeBatteryDisplayed = 0;
#define DISPLAY_BATTERY_PAUSE	1000 // ms, display only once per second
bool HandleBatteryAlarm()
{
	static bool AlarmWasActive = false;

	bool AlarmIsActive = GetBatteryAlarmState();
	if (AlarmWasActive && !AlarmIsActive)
	{	// return to normal state
		DEBUG_PRINTLN("Battery alarm ends.");
		AlarmWasActive = false;
		ReactivateServo();
		DisplayMenu(0);
	}
	else if (!AlarmWasActive && AlarmIsActive)
	{	// Go to power save mode. Steppers are shut down after every move anyway.
		IsBatteryMeasuring = false;
		DEBUG_PRINTLN("Battery alarm!");
		BuzzerOn(200);
		AlarmWasActive = true;
		DeactivateServo();
		DisplayBatteryAlarmState();
	}

	if (IsBatteryMeasuring)
	{
		if ((millis() - lastTimeBatteryDisplayed) > DISPLAY_BATTERY_PAUSE)
		{
			lastTimeBatteryDisplayed = millis();

			// IsBatteryMeasuring = false; // just display once
			double volts = measuredBattery * VOLT_PER_DIGIT;
			double remaining = volts - EmptyVoltage;
			BatteryPercentage = ((remaining / VoltageRange) * 100.0);
			if (BatteryPercentage < 0)
				BatteryPercentage = 0;
			else if (BatteryPercentage > 100)
				BatteryPercentage = 100;

			BatteryLine = "Battery: " + String((int)BatteryPercentage) + " %";
			UpdateBatteryLine();
		}
	}

	return AlarmIsActive;
}

void UpdateBatteryLine()
{
	if (IsBatteryMeasuring)
		//DrawExtraLine(BatteryLine, 20, 63 - 7);
		DrawBatteryState(BatteryPercentage);
}

esp_timer_handle_t timerBattery;
void SetupBatteryProtect()
{
	pinMode(PIN_CHECK_BATT, ANALOG);

	esp_timer_create_args_t timerConfigBattery;
	timerConfigBattery.arg = NULL;
	timerConfigBattery.callback = OnTimerBattery;
	timerConfigBattery.dispatch_method = ESP_TIMER_TASK;
	timerConfigBattery.name = "BatteryChecker";

	if (esp_timer_create(&timerConfigBattery, &timerBattery) != ESP_OK)
	{
		DEBUG_PRINTLN("timerButton create failed.");
	}
	esp_timer_stop(timerBattery);
	esp_timer_start_periodic(timerBattery, BATTERY_CHECK_INTERVALL);
}
