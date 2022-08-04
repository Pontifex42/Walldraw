#include "Preferences.h"
#include "SysData.h"

Preferences preferences;

#define DEFAULT_WIDTH 1000
#define DEFAULT_HEIGHT 800

#define DEFAULT_PEN_UP_ANGLE 86
#define DEFAULT_PEN_DOWN_ANGLE 46

#define DEFAULT_PAPER_X	297 // Assume DIN A4 Landscape
#define DEFAULT_PAPER_Y 210

void SavePlatformWidth(int w)
{
	preferences.begin("DIMENSION", false);
	preferences.putInt("W", w);
	preferences.end();
}

int ReadPlotterWidth()
{
	preferences.begin("DIMENSION", true);
	int ret = preferences.getInt("W", DEFAULT_WIDTH);
	preferences.end();
	return ret;
}


void SavePlatformHeight(int h)
{
	preferences.begin("DIMENSION", false);
	preferences.putInt("H", h);
	preferences.end();
}

int ReadPlotterHeight()
{
	preferences.begin("DIMENSION", true);
	int ret = preferences.getInt("H", DEFAULT_HEIGHT);
	preferences.end();
	return ret;
}


void SaveXOffset(int shiftPaperX)
{
	preferences.begin("OFFSET", false);
	preferences.putInt("X", shiftPaperX);
	preferences.end();
}

int ReadXOffset()
{
	preferences.begin("OFFSET", true);
	int ret = preferences.getInt("X", 0);
	preferences.end();
	return ret;
}

void SaveYOffset(int shiftPaperY)
{
	preferences.begin("OFFSET", false);
	preferences.putInt("Y", shiftPaperY);
	preferences.end();
}

int ReadYOffset()
{
	preferences.begin("OFFSET", true);
	int ret = preferences.getInt("Y", 0);
	preferences.end();
	return ret;
}

void SavePenUpPos(int angle)
{
	preferences.begin("SERVO", false);
	preferences.putInt("UP", angle);
	preferences.end();
}

int ReadPenUpPos()
{
	preferences.begin("SERVO", true);
	int ret = preferences.getInt("UP", DEFAULT_PEN_UP_ANGLE);
	preferences.end();
	return ret;
}


void SavePenDownPos(int angle)
{
	preferences.begin("SERVO", false);
	preferences.putInt("DOWN", angle);
	preferences.end();
}

int ReadPenDownPos()
{
	preferences.begin("SERVO", true);
	int ret = preferences.getInt("DOWN", DEFAULT_PEN_DOWN_ANGLE);
	preferences.end();
	return ret;
}

void SavePaperSize(int x, int y)
{
	preferences.begin("PAPER", false);
	preferences.putInt("X", x);
	preferences.putInt("Y", y);
	preferences.end();
}

void ReadPaperSize(int &x, int &y)
{
	preferences.begin("PAPER", true);
	x = preferences.getInt("X", DEFAULT_PAPER_X);
	y = preferences.getInt("Y", DEFAULT_PAPER_Y);
	preferences.end();
}

void SaveMotorRotation(int ReelOutL, int ReelInL, int ReelOutR, int ReelInR)
{
	preferences.begin("ROTATION", false);
	preferences.putInt("OutL", ReelOutL);
	preferences.putInt("InL", ReelInL);
	preferences.putInt("OutR", ReelOutR);
	preferences.putInt("InR", ReelInR);
	preferences.end();
}

#define DEFAULT_REEL_OUT_L -1
#define DEFAULT_REEL_IN_L 1
#define DEFAULT_REEL_OUT_R 1
#define DEFAULT_REEL_IN_R -1

void ReadMotorRotation(int& ReelOutL, int& ReelInL, int& ReelOutR, int& ReelInR)
{
	preferences.begin("ROTATION", true);
	ReelOutL = preferences.getInt("OutL", DEFAULT_REEL_OUT_L);
	ReelInL = preferences.getInt("InL", DEFAULT_REEL_IN_L);
	ReelOutR = preferences.getInt("OutR", DEFAULT_REEL_OUT_R);
	ReelInR = preferences.getInt("InR", DEFAULT_REEL_IN_R);
	preferences.end();
}


#define DEFAULT_SPEEDLEVEL 7
void SaveSpeed(int level)
{
	preferences.begin("SPEED", false);
	preferences.putInt("LEVEL", level);
	preferences.end();
}

int ReadSpeed()
{
	preferences.begin("SPEED", true);
	int ret = preferences.getInt("LEVEL", DEFAULT_SPEEDLEVEL);
	preferences.end();
	return ret;
}