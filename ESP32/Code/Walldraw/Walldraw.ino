// Wall Drawing Machine V2
// Thorsten Hartwig, (c) July 2022
// Based on an idea found on gitub https://github.com/shihaipeng03/Walldraw

#include "PinConfig.h"
#include <WiFi.h>
#include "Buzzer.h"
#include "BatteryProtect.h"
#include "RotaryEncoder.h"
#include "Sledge.h"
#include "Pen.h"
#include "OLED_Display.h"
#include "MenuHandler.h"
#include "Drawing.h"
#include "SDCardReader.h"
#include "Stepper.h"
#include "FileHandler.h"

void setup()
{
	setCpuFrequencyMhz(240);
	WiFi.mode(WIFI_OFF);
	WiFi.setSleep(true);

	Serial.begin(115200);
	SetupBuzzer();
	SetupOled();
	SetupStepper();
	SetupBatteryProtect();
	SetupMenu();
	SetupRotary();
	SetupSledge();
	SetupPen();
	SetupSDCard();
	SetupFileHandler();
	SetupDrawing();
	DEBUG_PRINTLN("Setup OK");
}


void loop()
{
	HandleBatteryAlarm();
	HandleMenu();
	HandleDrawing();
}

