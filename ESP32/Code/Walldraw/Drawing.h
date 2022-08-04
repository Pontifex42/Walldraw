#pragma once

void SetupDrawing();

void ProcessGcodeLinePlotter(String aLine);
void ProcessGcodeLineDisplay(String aLine);

void CalcParamsFileToDisplay();

extern bool DrawingIsRunning;

void EndDrawing();
void HandleDrawing();
void StartDrawing();
	
extern double resizeFactor;
extern double offX;
extern double offY;

#define NOT_A_COORD -4242.4242
void ParseGcodeLine(String aLine, double& fileX, double& fileY, double& fileZ);
