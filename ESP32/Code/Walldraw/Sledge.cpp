#include "PinConfig.h"
#include "Sledge.h"
#include "SysData.h"
#include "Stepper.h"
#include "Drawing.h"

// #define DEBUG_SLEDGE
// #define VERBOSE

#ifndef DEBUG_SLEDGE

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif

// actual dimensions of walldrawer installation
double maxPlotterX;		// Range of coordinates in X-direction, possible is - (=left) and +  (=right)
double upperPlotterY;	// Upper border coordinates in Y-direction, always negative
double bottomPlotterY;	// Lower border of drawing space, always negative
int shiftPaperX;
int shiftPaperY;

#define STEPS_PER_TURN			2048.0 // 2037.8864 may be the real number of steps per rotation with some gears
#define SPOOL_DIAMETER			36.5 // Spool is 35mm, but has some twine on it
#define SPOOL_CIRCUMFERENCE		(SPOOL_DIAMETER * 3.1416)  // 35*Pi=109.956
#define TWINE_PER_STEP			(SPOOL_CIRCUMFERENCE / STEPS_PER_TURN)  // 0.053689mm per step

#define DISTANCE_Y_FROM_BASELINE (-maxPlotterX / 3) // we could never reach an angle of 0 degrees.
#define DISTANCE_X_FROM_TWINEHOLES 50 // Pen can't be placed vertical under the twineholes

long laststepcnt_l, laststepcnt_r;

// sledge position
double curPlotPosX, curPlotPosY;
// z-coordinate is pen_state

double GetCurDrawPosX() { return curPlotPosX; }
double GetCurDrawPosY() { return curPlotPosY - DISTANCE_Y_FROM_BASELINE; }

#ifdef VERBOSE
double GetHeight(double x, double l, double r)
{
	double s = 0.5 * (x + l + r);
	double h = (2/x) * sqrt(s * (s - x) * (s - l) * (s - r));
	return h;
}

double GetX(double h, double l)
{
	double x = sqrt((l * l) - (h * h));
	return x;
}
#endif

void PaperCoordToPlotterCoord(double paperX, double paperY, double& plotX, double& plotY)
{
	// the upper middle point of paper is placed a) in the middle of the plotter, moved by shiftPaperX, and b) at the upper painting border DISTANCE_Y_FROM_BASELINE, moved by shiftPaperY
	plotX = paperX + shiftPaperX;
	plotY = paperY + shiftPaperY + upperPlotterY;

	// protect sledge from going outside range and protect twine from crashing

// x-coordinate "0" is in the middle of twinehole-distance
	if (plotX < -maxPlotterX)
	{
		DEBUG_PRINTLN("X exceeded left border");
		plotX = -maxPlotterX;
	}
	else if (plotX > maxPlotterX)
	{
		DEBUG_PRINTLN("X exceeded right border");
		plotX = maxPlotterX;
	}

	if (plotY > upperPlotterY)	// y starts a 0 and raises to negative
	{
		DEBUG_PRINTLN("Upper Y-Border exceeded");
		plotY = upperPlotterY;
	}
	else if (plotY < bottomPlotterY) // do not drop sledge under boards bottom end
	{
		DEBUG_PRINTLN("Lower Y-Border exceeded");
		plotY = bottomPlotterY;
	}

	DEBUG_PRINTLN("PaperCoordToPlotterCoord " + String(paperX) + "/" + String(paperY) + " ---> " + String(plotX) + "/" + String(plotY));
}

//------------------------------------------------------------------------------
// in: coords x|y in mm, in plotter-coordinates
// out: stepping count for left and right motor
void Pos2StepCnt(double plotX, double plotY, long& stepcnt_l, long& stepcnt_r)
{
	// kathetes of the triangles
	double lenX_l = maxPlotterX + plotX;
	double lenX_r = maxPlotterX - plotX;
	DEBUG_PRINTLN("Triangle: " + String(plotX) + "/" + String(plotY) + " ---> " + String(lenX_l) + "/" + String(lenX_r) + ", " + String(plotY));

	// get hypothenuses
	double twinelen_l = sqrt((lenX_l * lenX_l) + (plotY * plotY));
	double twinelen_r = sqrt((lenX_r * lenX_r) + (plotY * plotY));
	DEBUG_PRINTLN("  -->  TWINES: " + String(twinelen_l) + "/" + String(twinelen_r));

#ifdef VERBOSE
	// just to be sure calculation is right
	double coordY = -GetHeight(maxPlotterX * 2, twinelen_l, twinelen_r);
	double coordXl = GetX(coordY, twinelen_l);
	double coordXr = GetX(coordY, twinelen_r);
	DEBUG_PRINT("Table coords: " + String(coordXl) + "/ " + String(coordXr) + ", " + String(coordY));
#endif

	// from twinelength to steps
	stepcnt_l = round(twinelen_l / TWINE_PER_STEP);
	stepcnt_r = round(twinelen_r / TWINE_PER_STEP);
	DEBUG_PRINTLN("  -->  Steps: " + String(stepcnt_l) + "/" + String(stepcnt_r));
}

//==========================================================
// in: coords x|y in mm, in paper-coordinates
void MoveSledgeTo(double drawX, double drawY)
{
	// given coords are already resized and moved to the x-y-range of plotter, but replacement by Y-border is not done yet.

	double plotX, plotY;
	PaperCoordToPlotterCoord(drawX, drawY, plotX, plotY);

	long stepcnt_l, stepcnt_r;
	Pos2StepCnt(plotX, plotY, stepcnt_l, stepcnt_r);
	long steps_delta_l = stepcnt_l - laststepcnt_l;
	long steps_delta_r = stepcnt_r - laststepcnt_r;

#ifdef VERBOSE
	DEBUG_PRINT("stepcnt_l:" + String(stepcnt_l));
	DEBUG_PRINTLN(" delta_l:" + String(steps_delta_l));
	DEBUG_PRINT("stepcnt_r:" + String(stepcnt_r));
	DEBUG_PRINTLN(" delta_r:" + String(steps_delta_r));
#endif

	laststepcnt_l = stepcnt_l;
	laststepcnt_r = stepcnt_r;

	ReactivateStepper();

	MoveBothSteppers(steps_delta_l, steps_delta_r);

	curPlotPosX = plotX;
	curPlotPosY = plotY;

	DeactivateStepper();
}

void MoveSledgeIgnoreOffset(double drawX, double drawY)
{	// ignore drawing-offsets, just go to upper middle position
	MoveSledgeTo(drawX - shiftPaperX, drawY - shiftPaperY);
}

//------------------------------------------------------------------------------
// instantly set the virtual plotter position
void InitZeroPosition()
{
	curPlotPosX = 0;
	curPlotPosY = DISTANCE_Y_FROM_BASELINE;
	Pos2StepCnt(curPlotPosX, curPlotPosY, laststepcnt_l, laststepcnt_r);
}

void InitPlotterRanges()
{
	maxPlotterX = (ReadPlotterWidth() / 2) - DISTANCE_X_FROM_TWINEHOLES;
	bottomPlotterY = -ReadPlotterHeight();
	upperPlotterY = DISTANCE_Y_FROM_BASELINE;

	shiftPaperX = ReadXOffset();
	shiftPaperY = ReadYOffset();
	DEBUG_PRINTLN("PlatformRanges: " + String(-maxPlotterX) + "/" + String(maxPlotterX) +", " + String(upperPlotterY) + "/" + String(bottomPlotterY));
}

void SetupSledge()
{
	InitPlotterRanges();
	InitZeroPosition();
}