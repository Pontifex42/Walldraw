#include "PinConfig.h"
#include "Drawing.h"
#include "Pen.h"
#include "Sledge.h"
#include "SDCardReader.h"
#include "math.h"
#include "OLED_Display.h"
#include "MenuHandler.h"
#include "SysData.h"
#include "FileHandler.h"

// #define DEBUG_DRAWING

#ifndef DEBUG_DRAWING

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif

double resizeFactor;
double offX;
double offY;

// =============================================

void FileCoordToPaperCoord(double fileX, double fileY, double& paperX, double& paperY)
{
	paperX = fileX * resizeFactor;
	paperX += offX;

	paperY = fileY * resizeFactor;
	paperY += offY;

	DEBUG_PRINTLN("FileCoordToPaperCoord " + String(fileX) + "/" + String(fileY) + " ---> " + String(paperX) + "/" + String(paperY));
}

void FileCoordToDisplayCoord(double fileX, double fileY, double& displayX, double& displayY)
{
	displayX = fileX * resizeFactor;
	displayX += offX;

	displayY = fileY * resizeFactor;
	displayY += offY;
	DEBUG_PRINTLN("FileCoordToDisplayCoord " + String(fileX) + "/" + String(fileY) + " ---> " + String(displayX) + "/" + String(displayY));
}

#define PAPER_BORDER 10 // leave an empty frame around the drawing
void CalcParamsFileToPaper()
{
	int paperSizeX, paperSizeY;
	ReadPaperSize(paperSizeX, paperSizeY);

	if ((paperSizeX == 0) || (paperSizeY == 0)) // Native size as given in file
	{
		resizeFactor = 1.0;
		offX = (fileDetails.left + fileDetails.right) / -2.0;
		offY = -fileDetails.upper;
		return;
	}

	// leave a margin to paper edges
	double drawSpaceX = paperSizeX - (2*PAPER_BORDER);
	double drawSpaceY = paperSizeY - (2*PAPER_BORDER);

	double scaleX = drawSpaceX / fileDetails.width;
	double scaleY = drawSpaceY / fileDetails.height;
	resizeFactor = (scaleX < scaleY) ? scaleX : scaleY;

	offX = (fileDetails.left + fileDetails.right) / -2.0; // Center image
	offX *= resizeFactor;
	offY = -fileDetails.upper;	// upper painting border is on upper edge of paper
	offY *= resizeFactor;
	offY -= PAPER_BORDER;

	DEBUG_PRINTLN("Paper size: " + String(paperSizeX) + "/" + String(paperSizeY));
	DEBUG_PRINTLN("Paper space: " + String(drawSpaceX) + "/" + String(drawSpaceY));
	DEBUG_PRINTLN("Sizing params: " + String(resizeFactor) + "/  " + String(offX) + ", " + String(offY));
	DEBUG_PRINTLN("Drawing size: " + String(fileDetails.width * resizeFactor) + "/" + String(fileDetails.height * resizeFactor));
}

void CalcParamsFileToDisplay()
{
	double scaleX = DISPLAY_WIDTH / fileDetails.width;
	double scaleY = DISPLAY_HIGHT / fileDetails.height;

	resizeFactor = (scaleX < scaleY) ? scaleX : scaleY;

	offX = (fileDetails.left + fileDetails.right) / -2.0; // middle of drawing is in the middle of display
	offX *= resizeFactor;
	offX += DISPLAY_WIDTH / 2;
	offY = (fileDetails.upper + fileDetails.lower) / -2.0; // middle of drawing is in the middle of display
	offY *= resizeFactor;
	offY -= DISPLAY_HIGHT / 2;
	DEBUG_PRINTLN("Sizing params: " + String(resizeFactor) + "/  " + String(offX) + ", " + String(offY));
	DEBUG_PRINTLN("Size on display: " + String(fileDetails.width * resizeFactor) + "/" + String(fileDetails.height * resizeFactor));
}

bool DrawingIsRunning = false;

int currentDrawline;

int lastX = 0;
int lastY = 0;
void ParseGcodeLine(String aLine, double& fileX, double& fileY, double& fileZ)
{
	aLine.toUpperCase();

	fileX = fileY = fileZ = NOT_A_COORD;
	int pz = aLine.indexOf('Z');
	if (pz != -1)
	{	// Z-coords given in this line
		fileZ = aLine.substring(pz + 1).toDouble();
	}

	int px = aLine.indexOf('X');
	int py = aLine.indexOf('Y');
	if ((px == -1) && (py == -1)) // No X/Y coords given in this line. If at least one is given, this will work
		return;

	if (px == -1)
		fileX = lastX;
	else
	{
		fileX = aLine.substring(px + 1).toDouble();
		lastX = fileX;
	}

	if(py == -1)
		fileY = lastY;
	else
	{
		fileY = aLine.substring(py + 1).toDouble();
		lastY = fileY;
	}
}

void ProcessGcodeLinePlotter(String aLine)
{
	double fileX, fileY, fileZ;
	ParseGcodeLine(aLine, fileX, fileY, fileZ);

	if (fileZ != NOT_A_COORD)
	{	// Z-coords given in this line
		if (fileZ > 0)
			pen_up();
		else
			pen_down();
	}

	if ((fileX == NOT_A_COORD) || (fileY == NOT_A_COORD))// No X/Y coords given in this line
		return;

	double paperX, paperY;
	FileCoordToPaperCoord(fileX, fileY, paperX, paperY);
	MoveSledgeTo(paperX, paperY);
}

void ProcessGcodeLineDisplay(String aLine)
{
	double fileX, fileY, fileZ;
	ParseGcodeLine(aLine, fileX, fileY, fileZ);

	if (fileZ != NOT_A_COORD)
	{	// Z-coords given in this line
		SetCurCol((fileZ > 0) ? false : true);
	}

	if ((fileX == NOT_A_COORD) || (fileY == NOT_A_COORD))// No X/Y coords given in this line
		return;

	double displayX, displayY;
	FileCoordToDisplayCoord(fileX, fileY, displayX, displayY);
	DrawDisplayLine(displayX, displayY);
	DEBUG_PRINTLN("ProcessGcodeLineDisplay " + String(fileX) + "/" + String(fileY) + "  Resized:" + String(displayX) + "/" + String(displayY));
}


void EndDrawing()
{
	PaintProgressBar(100);
	pen_up();
	MoveSledgeIgnoreOffset(0, 0);
	DrawingIsRunning = false;
	CloseFile();
	PopulateBaseMenu();
}

void StartDrawing()
{
	DrawingIsRunning = true;
	currentDrawline = 0;

	OpenFile(SelectedFile);
	if (FileReachedEnd() || (fileDetails.linesInFile < 1))
	{
		DrawingIsRunning = false;
		DEBUG_PRINTLN("Open file error.");
		EndDrawing();
		return;
	}
	CalcParamsFileToPaper();
	DEBUG_PRINTLN("Drawing started.");
}

void HandleDrawing()
{
	// If drawing is active, we process one line of the file on every call
	if (!DrawingIsRunning)
		return;

	if (FileReachedEnd())
	{
		DEBUG_PRINTLN("Drawing finished");
		EndDrawing();
		return;
	}

	String readline = GetNextLine();

	ProcessGcodeLinePlotter(readline);  // execute the instruction of this line

	if (!(currentDrawline++ % 10)) // update progress bar every 10 lines
		PaintProgressBar(GetReadPercentage());
}

void SetupDrawing()
{
	resizeFactor = 1.0;
	offX = 0.0;
	offY = 0.0;
}