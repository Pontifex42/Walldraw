// Wall Drawing Machine
// Thorsten Hartwig, (c) March 2021
// Taken from github and modified for readability, speed, safety and resizing
//Github https://github.com/shihaipeng03/Walldraw

#include <TinyStepper_28BYJ_48.h>
#include <Servo.h>
#include <SD.h>

// #define VERBOSE 1

#define STEPS_PER_TURN			2048.0
#define SPOOL_DIAMETER			35.0
#define SPOOL_CIRCUMFERENCE		(SPOOL_DIAMETER * 3.1416)  // 35*Pi=109.956
#define TWINE_PER_STEP			(SPOOL_CIRCUMFERENCE / STEPS_PER_TURN)  // 0.053689mm per step

#define M_L_REEL_OUT     1
#define M_L_REEL_IN      -1
#define M_R_REEL_OUT     -1
#define M_R_REEL_IN      1

static long laststepcnt_l, laststepcnt_r;

#define X_SEPARATION	1000        // distance between twineholes at base in mm 
#define X_TWINEHOLE_R   ( X_SEPARATION*0.5)
#define X_TWINEHOLE_L   (-X_SEPARATION*0.5)
#define Y_TWINEHOLES    422	// mm y-distance to baseline at the initial point 0|0

// protect sledge from going outside range and protect twine from crashing
#define MIN_X X_TWINEHOLE_L
#define MAX_X X_TWINEHOLE_R
#define MAX_Y (Y_TWINEHOLES - 80) // 80 mm under base-level of twineholes
#define MIN_Y (-Y_TWINEHOLES) // do not drop sledge under boards bottom end

#define PEN_UP_ANGLE    86
#define PEN_DOWN_ANGLE  46

float resizeFactor = 1.0;
float offX = 0.0;
float offY = 0.0;

// sledge position
static float posx;
static float posy;
// z-coordinate is pen_state
static int pen_state;

Servo pen;

TinyStepper_28BYJ_48 M_L; // Left motor UNO pins 7 8 9 10
TinyStepper_28BYJ_48 M_R; // Right motor UNO pins 2 3 5 6

//------------------------------------------------------------------------------
// in: coords x|y in mm
// out: stepping count for left and right motor
void Pos2StepCnt(float x, float y, long& stepcnt_l, long& stepcnt_r)
{
	float dy = y - Y_TWINEHOLES;
	float dx_l = x - X_TWINEHOLE_L;
	float dx_r = x - X_TWINEHOLE_R;
	stepcnt_l = round(sqrt(dx_l * dx_l + dy * dy) / TWINE_PER_STEP);
	stepcnt_r = round(sqrt(dx_r * dx_r + dy * dy) / TWINE_PER_STEP);
}

//------------------------------------------------------------------------------
void pen_down()
{
	if (pen_state == PEN_DOWN_ANGLE)
		return; // Already down

// SG90 runs 120 deg./s = 0,12 deg/ms.
// 5 deg / 0.12 deg/ms = 42 ms
// hopefully, using 60 ms makes movement slower
// should be a multiply of 20, cause stepper PWM signal works at 50Hz
#define ANGLE_PER_STEP 5
#define SG90_DELAY 20

	const int steps = (PEN_UP_ANGLE - PEN_DOWN_ANGLE) / ANGLE_PER_STEP;
	int currentangle = PEN_UP_ANGLE;
	for (int i = 0; i < steps; ++i) // smooth movement
	{
		currentangle -= ANGLE_PER_STEP;
		pen.write(currentangle);
		delay(SG90_DELAY);
	}

	pen_state = PEN_DOWN_ANGLE;
	pen.write(pen_state);
	delay(SG90_DELAY * 3); // Time to end vibration
}

void pen_up()
{
	if (pen_state == PEN_UP_ANGLE)
		return; // Already up

	pen_state = PEN_UP_ANGLE;
	pen.write(pen_state);
	delay(40); // Time to get distance from paper
}

//==========================================================
void moveto(float x, float y)
{
	if ((x < MIN_X) || (x > MAX_X))
	{
		Serial.println("X-Border exceeded");
		return;
	}
	if ((y < MIN_Y) || (y > MAX_Y))
	{
		Serial.println("Y-Border exceeded");
		return;
	}

#ifdef VERBOSE
	Serial.println("go " + String(x) + "|" + String(y));
#endif

	long stepcnt_l, stepcnt_r;
	Pos2StepCnt(x, y, stepcnt_l, stepcnt_r);
	long delta_l = stepcnt_l - laststepcnt_l;
	long delta_r = stepcnt_r - laststepcnt_r;

#ifdef VERBOSE
	Serial.print("stepcnt_l:" + String(stepcnt_l));
	Serial.print(" laststepcnt_l " + String(laststepcnt_l));
	Serial.print(" delta_l:" + String(delta_l));
	Serial.print("stepcnt_r:" + String(stepcnt_r));
	Serial.print(" laststepcnt_r " + String(laststepcnt_r));
	Serial.println(" delta_r:" + String(delta_r));
#endif

	long add_l = abs(delta_l);
	long add_r = abs(delta_r);
	int dir_l = delta_l > 0 ? M_L_REEL_IN : M_L_REEL_OUT;
	int dir_r = delta_r > 0 ? M_R_REEL_IN : M_R_REEL_OUT;
	long over = 0;

	if (add_l > add_r)
	{
		for (long i = 0; i < add_l; ++i)
		{
			M_L.moveRelativeInSteps(dir_l);
			over += add_r;
			if (over >= add_l)
			{
				over -= add_l;
				M_R.moveRelativeInSteps(dir_r);
			}
		}
	}
	else
	{
		for (long i = 0; i < add_r; ++i)
		{
			M_R.moveRelativeInSteps(dir_r);
			over += add_l;
			if (over >= add_r)
			{
				over -= add_r;
				M_L.moveRelativeInSteps(dir_l);
			}
		}
	}

	laststepcnt_l = stepcnt_l;
	laststepcnt_r = stepcnt_r;
	posx = x;
	posy = y;
}

//------------------------------------------------------------------------------
static void line(float x, float y)
{
	// split up long lines to make them straighter?
	float dx = x - posx;
	float dy = y - posy;

	float delta = sqrt(dx * dx + dy * dy);

	if (delta <= TWINE_PER_STEP)
	{
		moveto(x, y);
		return;
	}

	// too long for a single step
	long pieces = floor(delta / TWINE_PER_STEP);
	float x0 = posx;
	float y0 = posy;
	float a;
	for (long j = 0; j <= pieces; ++j)
	{
		a = (float)j / (float)pieces;
		moveto((x - x0) * a + x0, (y - y0) * a + y0);
	}
	moveto(x, y);
}

inline void line_direct(float x, float y) { moveto(x, y); }

void nc(String st)
{
	st.toUpperCase();

	int px = st.indexOf('X');
	int py = st.indexOf('Y');
	int pz = st.indexOf('Z');

	bool hasXY = (px == -1 || py == -1) ? false : true;

	if (pz == -1)
	{	// no Z-coords given in this line
		pz = st.length();
	}
	else
	{
		String zz = st.substring(pz + 1, st.length());
		float z = zz.toFloat();
		if (z > 0)
			pen_up();
		else
			pen_down();
	}

	if (!hasXY) // No X/Y coords given in this line
		return;

	String xx = st.substring(px + 1, py);
	String yy = st.substring(py + 1, pz);

	xx.trim();
	yy.trim();

	line_direct(xx.toFloat() * resizeFactor + offX, yy.toFloat() * resizeFactor + offY);
}

//------------------------------------------------------------------------------
// Open nc-file and execute instructions line by line
void drawfile(String filename)
{
	File myFile = SD.open(filename);

	if (!myFile)
	{
		Serial.println("Open file error.");
		return;
	}

	Serial.println("File 1.nc opened");

	String readline = "";
#ifdef VERBOSE
	int line = 0;
#endif

	while (myFile.available())
	{
		char readchar = myFile.read();
		if (readchar != '\n')	// Read until end of line
		{
			readline += readchar;
			continue;
		}

#ifdef VERBOSE
		line++;
		Serial.println("Run nc #" + String(line)) + " : " + String(readline));
#endif
		nc(readline);  // execute the instruction of this line
		readline = "";
	}

	myFile.close();
}

// =============================================
// typedef enum PaperFormat { DIN_A0, DIN_A1, DIN_A2, DIN_A3, DIN_A4, DIN_A5, DIN_A6 };
// typedef enum PaperOrientation { LANDSCAPE, PORTRAIT, AUTO };
// bool GetDimensionsOfDrawing(float& minX, float& maxX, float& minY, float& maxY, String filename)
// {
// 	File myFile = SD.open(filename);
// 
// 	if (!myFile)
// 	{
// 		Serial.println("Open file error.");
// 		return false;
// 	}
// 
// 	Serial.println("File 1.nc opened");
// 
// 	String readline = "";
// 
// 	minX = minY = 99999.9;
// 	maxX = maxY = -99999.9;
// 
// #ifdef VERBOSE
// 	int line = 0;
// #endif
// 
// 	while (myFile.available())
// 	{
// 		char readchar = myFile.read();
// 		if (readchar != char(10))	// Read until end of line
// 		{
// 			readline += readchar;
// 			continue;
// 		}
// 
// 		readline.toUpperCase();
// 
// 		int px = readline.indexOf('X');
// 		int py = readline.indexOf('Y');
// 
// 		if (px == -1 || py == -1)
// 			continue;
// 
// #ifdef VERBOSE
// 		++line;
// #endif
// 
// 		int pz = readline.indexOf('Z');
// 		if (pz == -1)
// 		{	// no Z-coords given in this line
// 			pz = readline.length();
// 		}
// 
// 		String xx = readline.substring(px + 1, py);
// 		xx.trim();
// 		float tmp = xx.toFloat();
// 		if (tmp > maxX)
// 			maxX = tmp;
// 		if (tmp < minX)
// 			minX = tmp;
// 
// 		String yy = readline.substring(py + 1, pz);
// 		yy.trim();
// 		tmp = yy.toFloat();
// 		if (tmp > maxY)
// 			maxY = tmp;
// 		if (tmp < minY)
// 			minY = tmp;
// 
// 		readline = "";
// 	}
// 
// 	myFile.close();
// 
// #ifdef VERBOSE
// 	Serial.print("Lines in file:");
// 	Serial.println(line);
// 
// 	Serial.print("X-Dimensions :");
// 	Serial.print(minX);
// 	Serial.print(" | ");
// 	Serial.println(maxX);
// 
// 	Serial.print("Y-Dimensions :");
// 	Serial.print(minY);
// 	Serial.print(" | ");
// 	Serial.println(maxY);
// #endif
// 	return true;
// }
// 
// float GetSizeFactor(String filename, PaperFormat pap, PaperOrientation orientation)
// {
// 	float minX, maxX, minY, maxY;
// 	if (!GetDimensionsOfDrawing(minX, maxX, minY, maxY, filename))
// 		return 1.0;
// 
// 	float width = maxX - minX;
// 	float height = maxY - minY;
// 
// #ifdef VERBOSE
// 	Serial.print("width: ");
// 	Serial.print(width);
// 	Serial.print("  height: ");
// 	Serial.println(height);
// #endif
// 
// 	int papX, papY;
// 	// All paper formats given in PORTRAIT-orientation
// 	switch (pap)
// 	{
// 	case DIN_A0:
// 		papX = 841;
// 		papY = 1189;
// 		break;
// 	case DIN_A1:
// 		papX = 594;
// 		papY = 841;
// 		break;
// 	case DIN_A2:
// 		papX = 420;
// 		papY = 594;
// 		break;
// 	case DIN_A3:
// 		papX = 297;
// 		papY = 420;
// 		break;
// 	case DIN_A4:
// 		papX = 210;
// 		papY = 297;
// 		break;
// 	case DIN_A5:
// 		papX = 148;
// 		papY = 210;
// 		break;
// 	case DIN_A6:
// 	default:
// 		papX = 105;
// 		papY = 148;
// 		break;
// 	}
// 
// 	if ((orientation == LANDSCAPE) ||
// 		((orientation == AUTO) && (width > height)))
// 	{
// 		// swap height/width
// 		int tmp = papX;
// 		papX = papY;
// 		papY = tmp;
// 	}
// 
// 	// consider limits of plotter here (and only here)
// 	if (papX > X_SEPARATION)
// 		papX = X_SEPARATION;
// 	if (papY > ((Y_TWINEHOLES - 60) * 2)) // -60 cause we could never reach an angle of 0 degrees. Adopt to your plotter physics.
// 		papY = (Y_TWINEHOLES - 60) * 2;
// 
// 	papX -= 20; // need space for pen-sledge at the edges of paper 
// 	papY -= 20;
// 
// 	// consider a safety margin for displaced paper here.
// 	papX -= 20;
// 	papY -= 20;
// 
// 	int halfX = papX / 2;
// 	int halfY = papY / 2;
// 
// 	float scaleX = papX / width;
// 	float scaleY = papY / height;
// 	float retVal = (scaleX < scaleY) ? scaleX : scaleY;
// 
// 	// calculate a horizontal/vertikal offset to center the image exactly
// 	offX = (minX + maxX) / -2.0;
// 	offY = (minY + maxY) / -2.0;
// 
// #ifdef VERBOSE
// 	Serial.print("Paper: ");
// 	Serial.print(papX);
// 	Serial.print(" x ");
// 	Serial.println(papY);
// 	Serial.print("Resize-Factor: ");
// 	Serial.println(retVal);
// 	Serial.print("Drawsize: ");
// 	Serial.print(retVal * width);
// 	Serial.print(" | ");
// 	Serial.println(retVal * height);
// 	Serial.print("Left: ");
// 	Serial.print(minX * retVal);
// 	Serial.print(" Right: ");
// 	Serial.println(maxX * retVal);
// 	Serial.print("Top: ");
// 	Serial.print(maxY * retVal);
// 	Serial.print(" Bottom: ");
// 	Serial.println(minY * retVal);
// 	if (papX > papY)
// 		Serial.println("Landscape");
// 	else
// 		Serial.println("Portrait");
// 	Serial.print("Offsets: ");
// 	Serial.print(offX);
// 	Serial.print(" | ");
// 	Serial.println(offY);
// 	Serial.println();
// #endif
// 
// 	return retVal;
// }
// 

void ReadSizeAdoption(String filename)
{
	offX = 0.0;
	offY = 0.0;
	resizeFactor = 1.0;

	File myFile = SD.open(filename);

	if (!myFile)
	{	// no file present, use defaults
		Serial.println("No resize-parameter file found.");
		return;
	}

	String readline = "";

	int parCnt = 0;
	while (myFile.available())
	{
		char readchar = myFile.read();
		if (readchar != '\n')	// Read until end of line
		{
			readline += readchar;
			continue;
		}

		readline.trim();
		readline.toUpperCase();

		parCnt++;
		if (parCnt == 1)
			resizeFactor = readline.toFloat();
		else if (parCnt == 2)
			offX = readline.toFloat();
		else if (parCnt == 3)
			offY = readline.toFloat();
		else
			break;

		readline = "";
	}

	myFile.close();

#ifdef VERBOSE
	Serial.println("Found sizing:");
	Serial.println("Resize: " + String(resizeFactor));
	Serial.println("Offsets: " + String(offX) + " | " + String(offY));
#endif
}

//------------------------------------------------------------------------------
// instantly move the virtual plotter position
// does not validate if the move is valid
static void init_xy(float x, float y)
{
	posx = x;
	posy = y;
	long stepcnt_l, stepcnt_r;
	Pos2StepCnt(posx, posy, stepcnt_l, stepcnt_r);
	laststepcnt_l = stepcnt_l;
	laststepcnt_r = stepcnt_r;
}

void setup()
{
	Serial.begin(115200);
	M_L.connectToPins(7, 8, 9, 10); //M1 L UNO pins 7 8 9 10
	M_R.connectToPins(2, 3, 5, 6);  //M2 R UNO pins 2 3 5 6
	M_L.setSpeedInStepsPerSecond(10000);
	M_L.setAccelerationInStepsPerSecondPerSecond(100000);
	M_R.setSpeedInStepsPerSecond(10000);
	M_R.setAccelerationInStepsPerSecondPerSecond(100000);

	pen.attach(A0);
	pen_state = PEN_UP_ANGLE;
	pen.write(pen_state);

	init_xy(0, 0);

	if (!SD.begin(4))
	{
		Serial.println("SD init failed!");
		while (1)
			;
	}

	Serial.println("Setup OK");
}


void loop()
{
	ReadSizeAdoption("1.par");
	drawfile("1.nc");  // File 1.nc in Gcode-format from SD-card
	moveto(0.0, 0.0);
	Serial.println("Drawing finished");
	while (1)
		;
}

