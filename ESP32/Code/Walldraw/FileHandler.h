#pragma once

typedef struct FileDetails
{
	double left;
	double right;
	double upper;
	double lower;
	double width;
	double height;
	int linesInFile;
};

extern FileDetails fileDetails;

bool ScanFileContent(String filename);
bool FileReachedEnd();
bool OpenFile(String filename);
String GetNextLine();
void CloseFile();
double GetReadPercentage();

void SetupFileHandler();