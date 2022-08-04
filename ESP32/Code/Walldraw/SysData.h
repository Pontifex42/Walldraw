#pragma once

void SavePlatformWidth(int w);
int ReadPlotterWidth();

void SavePlatformHeight(int h);
int ReadPlotterHeight();

void SaveXOffset(int shiftPaperX);
int ReadXOffset();

void SaveYOffset(int shiftPaperY);
int ReadYOffset();

void SavePenUpPos(int angle);
int ReadPenUpPos();

void SavePenDownPos(int angle);
int ReadPenDownPos();

void SavePaperSize(int x, int y);
void ReadPaperSize(int &x, int &y);

void SaveMotorRotation(int ReelOutL, int ReelInL, int ReelOutR, int ReelInR);
void ReadMotorRotation(int &ReelOutL, int& ReelInL, int& ReelOutR, int& ReelInR);

void SaveSpeed(int level);
int ReadSpeed();
