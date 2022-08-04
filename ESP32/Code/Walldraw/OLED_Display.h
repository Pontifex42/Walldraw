#pragma once
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>

#define DISPLAY_WIDTH 128 // OLED display width, in pixels
#define DISPLAY_HIGHT 64 // OLED display height, in pixels

void SetupOled();

// for menu handling
#define VISIBLE_MENULINES 7
void ClearDisplayContent();
void AddLineToDisplay(String line, bool invers);
void ShowDisplay();
void DisplayBatteryAlarmState();

void SetCurCol(bool paint);

void PrepareProgressBar(String line1, String line2);
void SetProgressBarTo(int percentage);
void EndProgressBar();
void PaintProgressBar(int percentage);

void DrawDisplayLine(int drawX, int drawY);
void DrawExtraLine(String line, int x, int y);

void DrawFileToDisplay();
void DrawBatteryState(int percentage);