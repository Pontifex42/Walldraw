#include "PinConfig.h"
#include "FileHandler.h"
#include "SD.h"
#include "OLED_Display.h"

// #define DEBUG_FILEHANDLER

#ifndef DEBUG_FILEHANDLER

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif

FileDetails fileDetails;

SDFile myFile;

bool FileReachedEnd()
{
	if (!myFile)
		return true;
	if (!myFile.available())
		return true;
	return false;
}

bool OpenFile(String filename)
{
	if (myFile)
		CloseFile();

	myFile = SD.open('/' + filename);
	if (!myFile)
	{
		DEBUG_PRINTLN("File open failed: " + filename);
		return false;
	}
	return true;
}

String GetNextLine()
{
#define BUF_SIZE 256
	char buffer[BUF_SIZE];

	while (!FileReachedEnd())
	{
		int ret = myFile.readBytesUntil('\n', buffer, BUF_SIZE);
		if (ret <= 0)	// allow empty lines to be skipped
			continue;
		if (ret >= BUF_SIZE)
			ret = BUF_SIZE - 1;
		buffer[ret] = '\0';

		fileDetails.linesInFile++;
		String aLine = buffer;
		DEBUG_PRINTLN(aLine);
		return aLine;
	}
	return "";
}

void CloseFile()
{
	if (myFile)
		myFile.close();
}

double GetReadPercentage()
{
	if (!myFile)
		return 0.0;
	double size = myFile.size();
	double pos = myFile.position();
	double percentage = (pos / size) * 100.0;
	return percentage;
}

bool ScanFileContent(String filename)
{
	fileDetails.linesInFile = 0;
	fileDetails.left = 99999.9;
	fileDetails.right = -99999.9;
	fileDetails.upper = -99999.9;
	fileDetails.lower = 99999.9;
	fileDetails.width = 0.0;
	fileDetails.height = 0.0;

	// String name = filename.substring(1, filename.length() - 3); // cut first letter '/' onot on ESP32 
	String name = filename.substring(0, filename.length() - 3); // cut file ending ".nc"
	PrepareProgressBar("Scanning file", name);

	OpenFile(filename);

#ifdef DEBUG_FILEHANDLER
	unsigned long starttime = millis();
#endif

	int lines = 0;
	while (!FileReachedEnd())
	{
		String aLine = GetNextLine();

		int px = aLine.indexOf('X');
		if (px == -1)	// No X/Y coords given in this line
			continue;
		int py = aLine.indexOf('Y');
		if (py == -1)
			continue;

		double tmpx = aLine.substring(px + 1).toDouble();
		double tmpy = aLine.substring(py + 1).toDouble();
		if ((tmpx == 0.0) && (tmpy == 0.0))  // Gcode files from inkscape have a "move to coords 0/0" at start and beginning, but the drawing is anywhere else in coordinate system. Ignore this.
			continue;

		if (tmpx > fileDetails.right)
			fileDetails.right = tmpx;
		else if (tmpx < fileDetails.left)
			fileDetails.left = tmpx;
		//DEBUG_PRINTLN("Dim X: " + String(tmpx) + " ---> " + String(fileMinX) + "/" + String(fileMaxX));

		if (tmpy > fileDetails.upper)
			fileDetails.upper = tmpy;
		else if (tmpy < fileDetails.lower)
			fileDetails.lower = tmpy;
		//DEBUG_PRINTLN("Dim Y: " + String(tmpy) + " ---> " + String(fileMinY) + "/" + String(fileMaxY));

		lines++;
		if (!(lines % 200)) // update progress bar only every 200 lines
			SetProgressBarTo(GetReadPercentage());
	}
#ifdef DEBUG_FILEHANDLER
	unsigned long duration = millis() - starttime;
	DEBUG_PRINTLN("Scanning file took " + String(duration) + " ms");
#endif

	CloseFile();
	fileDetails.width = fileDetails.right - fileDetails.left;
	fileDetails.height = fileDetails.upper - fileDetails.lower;

	DEBUG_PRINTLN("File contains " + String(fileDetails.linesInFile) + " lines.");
	DEBUG_PRINTLN("Left " + String(fileDetails.left) + "mm  Right " + String(fileDetails.right) + "mm    Up " + String(fileDetails.upper) + "mm  Down " + String(fileDetails.lower) + "mm ");
	DEBUG_PRINTLN("Width " + String(fileDetails.width) + "mm  Height " + String(fileDetails.height) + "mm Lines: " + String(fileDetails.linesInFile));
	EndProgressBar();
	return true;
}

void SetupFileHandler()
{
	fileDetails.left = 0.0;
	fileDetails.right = 0.0;
	fileDetails.upper = 0.0;
	fileDetails.lower = 0.0;
	fileDetails.width = 0.0;
	fileDetails.height = 0.0;
	fileDetails.linesInFile = 0;
}
