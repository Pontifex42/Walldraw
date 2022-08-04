#pragma once

void SetupSDCard();

String GetFile(int idx);
void ScanDirectory();
#define MAX_FILES 100
extern String allfiles[MAX_FILES];
extern int filecnt;
