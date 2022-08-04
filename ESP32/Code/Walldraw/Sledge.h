#pragma once

void SetupSledge();
void InitZeroPosition();
void InitPlotterRanges();

void MoveSledgeTo(double paperPosX, double paperPosY);
void MoveSledgeIgnoreOffset(double drawX, double drawY);
double GetCurDrawPosX();
double GetCurDrawPosY();

void InvertStepperRotation();
// actual dimensions od walldrawer installation
extern double maxPlotterX;	  // Range of coordinates in X-direction, possible is - (=left) and +  (=right)
extern double upperPlotterY;  // Upper border coordinates in Y-direction, always negative
extern double bottomPlotterY; // Lower border of drawing space, always negative
extern int shiftPaperX;
extern int shiftPaperY;