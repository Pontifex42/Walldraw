#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include "PinConfig.h"
#include "OLED_Display.h"
#include "FileHandler.h"
#include "SDCardReader.h"
#include "Drawing.h"
#include "MenuHandler.h"
#include "BatteryProtect.h"

// #define DEBUG_OLED

#ifndef DEBUG_OLED

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif


#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106G display = Adafruit_SH1106G(DISPLAY_WIDTH, DISPLAY_HIGHT, &Wire);

#define FSM_LOGO_WIDTH 128
#define FSM_LOGO_HEIGHT 64

const unsigned char gImage_FSM_Logo[1024] = {
0X00,0X00,0X00,0X00,0X00,0X00,0X1F,0X80,0X01,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X7F,0XC0,0X07,0XFC,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X7F,0XE0,0X07,0X9E,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0XE0,0XE0,0X0E,0X07,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0XC0,0X70,0X0C,0X03,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X01,0XCE,0X30,0X1C,0XF3,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X01,0XDF,0X30,0X1C,0XF3,0X80,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X01,0X9F,0X30,0X1C,0XF3,0X80,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X01,0XDF,0X30,0X1C,0XF3,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X01,0XCE,0X70,0X0C,0X63,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0XE0,0X70,0X0E,0X07,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0XE0,0X07,0X1E,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X7F,0XE0,0X07,0XFC,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X3F,0X80,0X03,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X0F,0X80,0X01,0XC0,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X03,0X80,0X03,0XC0,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X03,0XDF,0XFF,0X80,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X07,0XFF,0XFF,0XF0,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X3F,0XFF,0XFF,0XFE,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X01,0XFF,0XFF,0XFF,0XFF,0XC0,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X07,0XFF,0X81,0XC0,0XFF,0XF0,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X1F,0XF0,0X0F,0XF0,0X07,0XFC,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X7F,0X80,0X1F,0XF8,0X00,0XFF,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X01,0XFE,0X7C,0X3F,0XFC,0X30,0X3F,0X80,0X00,0X00,0X00,0X00,
0X00,0X00,0XFC,0X00,0X07,0XF1,0XFE,0X7F,0XFC,0X78,0X0F,0XE0,0X00,0X08,0X00,0X00,
0X00,0X01,0XFF,0X00,0X0F,0XC7,0XFE,0X7F,0XFC,0X7C,0X73,0XF0,0X00,0XFE,0X00,0X00,
0X00,0X01,0XFF,0X80,0X1F,0X9F,0XFE,0XFE,0X7C,0XFC,0XF8,0XFC,0X03,0XFF,0X00,0X00,
0X00,0X03,0XDF,0XE0,0X7E,0X3F,0XFE,0XFC,0X3C,0XFC,0XF8,0X7E,0X07,0XFF,0X00,0X00,
0X00,0X03,0X83,0XF0,0XFC,0X3F,0X8E,0XFC,0X3C,0XFD,0XFC,0X1F,0X1F,0XC7,0X80,0X00,
0X7C,0X03,0X80,0XFD,0XF0,0X3F,0X86,0X7E,0X1D,0XFF,0XFC,0X0F,0XBF,0X07,0X80,0X00,
0XFF,0X03,0X80,0X7F,0XE0,0X3F,0X80,0X7F,0X81,0XFF,0XFE,0X07,0XFC,0X07,0X00,0XFE,
0XFF,0XC3,0X80,0X1F,0XC0,0X3F,0XF8,0X7F,0XC1,0XFF,0XFE,0X03,0XF8,0X07,0X03,0XFF,
0XFF,0XE3,0XC0,0X0F,0X80,0X1F,0XF8,0X3F,0XF1,0XFF,0XBE,0X01,0XF0,0X07,0X0F,0XFE,
0X03,0XFB,0XC0,0X07,0XC0,0X1F,0XF8,0X1F,0XF1,0XF7,0XBE,0X03,0XE0,0X07,0X1F,0XC0,
0X00,0XFF,0X80,0X03,0XF0,0X1F,0XF8,0X07,0XF1,0XF3,0X1E,0X0F,0XC0,0X07,0XFF,0X00,
0X00,0X3F,0X80,0X00,0XF8,0X1F,0XB8,0X03,0XF9,0XF0,0X1E,0X1F,0X00,0X07,0XFC,0X00,
0X00,0X1F,0X00,0X07,0XFC,0X1F,0X81,0X81,0XF9,0XE0,0X1E,0X3E,0X00,0X03,0XF0,0X00,
0X00,0X00,0X00,0X1F,0XFF,0X1F,0X81,0XE3,0XF9,0XE0,0X1E,0XFF,0XE0,0X00,0XC0,0X00,
0X00,0X00,0X00,0X3F,0XFF,0X9F,0X81,0XFF,0XF1,0XE0,0X01,0XFF,0XF0,0X00,0X00,0X00,
0X00,0X00,0X00,0X7E,0X0F,0XE7,0X01,0XFF,0XF1,0XE0,0X07,0XFF,0XF8,0X00,0X00,0X00,
0X00,0X00,0X00,0X78,0X03,0XF8,0X00,0XFF,0XE1,0XE0,0X1F,0XC0,0X7C,0X00,0X00,0X00,
0X00,0X00,0X00,0XF0,0X00,0XFE,0X00,0XFF,0XE1,0XE0,0X7F,0X00,0X3C,0X00,0X00,0X00,
0X00,0X00,0X00,0XE0,0X00,0X7F,0XC0,0X7F,0X80,0XC3,0XFE,0X00,0X1C,0X00,0X00,0X00,
0X00,0X00,0X00,0XE0,0X00,0XFF,0XF8,0X3F,0X00,0X1F,0XFC,0X00,0X1E,0X00,0X00,0X00,
0X00,0X00,0X00,0XE0,0X01,0XFF,0XFF,0X80,0X01,0XFF,0XFF,0X00,0X0E,0X00,0X00,0X00,
0X00,0X00,0X01,0XE0,0X03,0XE0,0XFF,0XFF,0XFF,0XFF,0XBF,0X80,0X0E,0X00,0X00,0X00,
0X00,0X00,0X01,0XE0,0X03,0XC0,0X1F,0XFF,0XFF,0XFC,0X07,0XC0,0X0E,0X00,0X00,0X00,
0X00,0X00,0X01,0XC0,0X07,0X80,0X03,0XFF,0XFF,0XC0,0X03,0XC0,0X0E,0X00,0X00,0X00,
0X00,0X00,0X01,0XC0,0X07,0X80,0X00,0X07,0XF0,0X00,0X01,0XC0,0X0F,0X00,0X00,0X00,
0X00,0X7E,0X03,0XC0,0X07,0X00,0X00,0X00,0X00,0X00,0X01,0XC0,0X07,0X00,0XF0,0X00,
0X00,0XFF,0XFF,0XC0,0X07,0X00,0X00,0X00,0X00,0X00,0X01,0XC0,0X07,0X87,0XFC,0X00,
0X00,0XFF,0XFF,0X80,0X07,0X80,0X00,0X00,0X00,0X00,0X03,0XC0,0X03,0XFF,0XFE,0X00,
0X01,0XE7,0XFF,0X00,0X03,0X80,0X00,0X00,0X00,0X00,0X03,0XC0,0X03,0XFF,0XFE,0X00,
0X01,0XC0,0XFC,0X00,0X03,0X80,0X00,0X00,0X00,0X00,0X03,0X80,0X01,0XFE,0X0F,0X00,
0X01,0X80,0X00,0X00,0X03,0X80,0X00,0X00,0X00,0X00,0X03,0X80,0X00,0X70,0X07,0X00,
0X00,0X00,0X00,0X00,0X03,0X80,0X00,0X00,0X00,0X00,0X07,0X80,0X00,0X00,0X02,0X00,
0X00,0X00,0X00,0X00,0X03,0X80,0X00,0X00,0X00,0X00,0X07,0X80,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X03,0X80,0X00,0X00,0X00,0X00,0X07,0X80,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0XFF,0X80,0X00,0X00,0X00,0X00,0X03,0XC0,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X07,0XFF,0X00,0X00,0X00,0X00,0X00,0X03,0XFF,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X0F,0XFE,0X00,0X00,0X00,0X00,0X00,0X01,0XFF,0XE0,0X00,0X00,0X00,
0X00,0X00,0X00,0X0F,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XE0,0X00,0X00,0X00,
0X00,0X00,0X00,0X0E,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X1F,0XE0,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X40,0X00,0X00,0X00,
};

#define ICON_WIDTH_BATTERY 16
#define ICON_HEIGHT_BATTERY 8
const unsigned char gImage_Battery100[16] = {
	0xFF, 0xFC,
	0xFF, 0xFC,
	0xFF, 0xFF,
	0xFF, 0xFF,
	0xFF, 0xFF,
	0xFF, 0xFF,
	0xFF, 0xFC,
	0xFF, 0xFC,
};


String lines[VISIBLE_MENULINES];
bool invertedlines[VISIBLE_MENULINES];
int linecnt = 0;
void ClearDisplayContent()
{
	for (int i = 0; i < linecnt; ++i)
		lines[i] = "";
	linecnt = 0;
}

void AddLineToDisplay(String line, bool invers)
{
	if (linecnt >= VISIBLE_MENULINES)
		return;
	lines[linecnt] = line;
	invertedlines[linecnt] = invers;
	linecnt++;
}

void ShowDisplay()
{
	display.clearDisplay();
	for (int i = 0; i < linecnt; ++i)
	{
		if (invertedlines[i])
			display.setTextColor(SH110X_BLACK, SH110X_WHITE);
		else
			display.setTextColor(SH110X_WHITE);
		display.setCursor(0, i*9);
		String line = lines[i].substring(0, 21);
		display.println(line);
	}
	UpdateBatteryLine();
	display.display();
	display.setTextColor(SH110X_WHITE);
}

void DisplayBatteryAlarmState()
{
	display.clearDisplay();
	display.setTextSize(2);      // Normal 1:1 pixel scale
	display.setTextColor(SH110X_WHITE); // Draw white text // display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
	display.setCursor(20, 10);     // Start at top-left corn
	display.println("BATTERY");
	display.setCursor(30, 30);     // Start at top-left corn
	display.println("POWER");
	display.setCursor(40, 50);     // Start at top-left corn
	display.println("LOW");
	display.display();
	display.setTextSize(1);
}

void SetupOled()
{
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SCREEN_ADDRESS))
	{
		DEBUG_PRINTLN("SSD1306 allocation failed");
		return;
	}

	display.clearDisplay();

	// let there be an appearance of the flying spaghetti monsters to the unbelievers!
	display.drawBitmap(
		(display.width() - FSM_LOGO_WIDTH) / 2,
		(display.height() - FSM_LOGO_HEIGHT) / 2,
		gImage_FSM_Logo, FSM_LOGO_WIDTH, FSM_LOGO_HEIGHT, 1);

	display.display();

	// Clear the buffer
	display.clearDisplay();
	display.setTextSize(1);      // Normal 1:1 pixel scale
	display.setTextColor(SH110X_WHITE); // Draw white text // display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
	display.setCursor(0, 0);     // Start at top-left corner
	display.cp437(true);         // Use full 256 char 'Code Page 437' font
	delay(700); // let the fsm logo be visible
}

int curCol = SH110X_BLACK;

void SetCurCol(bool paint)
{
	if (paint)
		curCol = SH110X_WHITE;
	else
		curCol = SH110X_BLACK;
}

int formerX = 0;
int formerY = 0;
void DrawDisplayLine(int drawX, int drawY)
{
	if (drawX < 0)
		drawX = 0;
	else if (drawX > (DISPLAY_WIDTH - 1))
		drawX = (DISPLAY_WIDTH - 1);
	if (drawY > 0)
		drawY = 0;
	else if (drawY < -(DISPLAY_HIGHT - 1))
		drawY = -(DISPLAY_HIGHT - 1);

	DEBUG_PRINTLN("DrawDisplayLine " + String(formerX) + "/" + String(formerY) + " --> " + String(drawX) + "/" + String(drawY) + " col:" + String(curCol));

	drawY *= -1;
	if (curCol == SH110X_WHITE)
		display.drawLine(formerX, formerY, drawX, drawY, SH110X_WHITE);
	formerX = drawX;
	formerY = drawY;
}

String l1, l2;
int lastPaintedProgress;
void PrepareProgressBar(String line1, String line2)
{
	l1 = line1;
	l2 = line2;
	lastPaintedProgress = -1;
	display.setTextSize(1);
}

void SetProgressBarTo(int percentage)
{
	display.clearDisplay();
	display.setCursor(0, 2);
	display.println(l1);
	display.setCursor(0, 14);
	display.println(l2);
	PaintProgressBar(percentage);
}

void EndProgressBar()
{
	display.clearDisplay();
	display.display();
}

void PaintProgressBar(int percentage)
{
	if (percentage > 100)
		percentage = 100;

	if (percentage < 0) // just refresh last value
	{
		percentage = lastPaintedProgress;
		if (percentage < 0)
			percentage = 0;
	}
	else 
	{
		if (lastPaintedProgress == percentage)
			return;
		lastPaintedProgress = percentage;
	}

#define PROGRESSBAR_YPOS 30
	display.fillRect(14, PROGRESSBAR_YPOS + 1, 100, 25, SH110X_BLACK); // delete area of progress bar
	display.drawRect(13, PROGRESSBAR_YPOS, 101, 12, SH110X_WHITE); // a frame around the progress bar
	display.fillRect(14, PROGRESSBAR_YPOS + 1, percentage, 10, SH110X_WHITE);
	DrawExtraLine(String(percentage) + " %", 50, PROGRESSBAR_YPOS + 14);
	display.display();
}

void DrawFileToDisplay()
{
	CalcParamsFileToDisplay();

	OpenFile(SelectedFile);

	display.clearDisplay();
	formerX = 0;
	formerY = 0;

	int line = 0;
	while (!FileReachedEnd())
	{
		String readline = GetNextLine();
		DEBUG_PRINTLN(readline);
		ProcessGcodeLineDisplay(readline);  // execute the instruction of this line
		if (!(++line % 400)) // update display every 400 lines which may be around every 0.4s
			display.display();
	}

	display.display();

	CloseFile();
}

void DrawExtraLine(String line, int x, int y)
{
	display.fillRect(x, y, DISPLAY_WIDTH - x, 8, SH110X_BLACK); // delete area of line
	display.setTextColor(SH110X_WHITE);
	display.setCursor(x, y);
	display.println(line);
	display.display();
}

void DrawBatteryState(int percentage)
{
	display.drawBitmap(0, 64 - ICON_HEIGHT_BATTERY, gImage_Battery100, ICON_WIDTH_BATTERY, ICON_HEIGHT_BATTERY, 1);
	// Got 12 lines inside battery icon to draw in black. That makes 13 levels of battery state.
	// 100/12 = 7,69%
	// 0, 7, 15, 23, 30, 38, 46, 53, 61, 69, 76, 84, 92, 100
	//  0  1   2   3   4   5   6   7   8   9   10  11  12 lines

	int LinesToHide = (100.0 - percentage) / 8;
	if(LinesToHide > 0)
		display.fillRect(1 + 12 - LinesToHide, 64 - ICON_HEIGHT_BATTERY + 1, LinesToHide, ICON_HEIGHT_BATTERY - 2, SH110X_BLACK); // delete area of icon
	DrawExtraLine(String(percentage) + "%", 18, 64 - 8);
}