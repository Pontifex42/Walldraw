#define _AFXDLL

#include <iostream>
#include <afx.h>
#include <io.h>
#include <cstring>

enum class PaperFormat { DIN_A0, DIN_A1, DIN_A2, DIN_A3, DIN_A4, DIN_A5, DIN_A6 };
enum class PaperOrientation { LANDSCAPE, PORTRAIT, AUTO };

// actual draw-range limits of plotter
#define X_SEPARATION	1000        // base width in mm 
#define X_TWINEHOLE_R   ( X_SEPARATION*0.5)
#define X_TWINEHOLE_L   (-X_SEPARATION*0.5)
#define Y_TWINEHOLES    422	// mm y-distance to baseline at a virtual point 0|0


bool GetDimensionsOfDrawing(double& minX, double& maxX, double& minY, double& maxY, CString filename)
{
	int line = 0;
	minX = minY = 99999.9;
	maxX = maxY = -99999.9;

	CStdioFile aFile;
	if (!aFile.Open(filename, CFile::modeRead))
	{
		std::cout << "File not found" << std::endl;
		return false;
	}
	
	CString aLine;
	while (aFile.ReadString(aLine))
	{
		line++;
		aLine.MakeUpper();

		int px = aLine.Find('X');
		int py = aLine.Find('Y');

		if ((px == -1) || (py == -1)) // No X/Y coords given in this line
			continue;

		int pz = aLine.Find('Z');
		if (pz == -1)
		{	// no Z-coords given in this line
			pz = aLine.GetLength();
		}

		CString xx = aLine.Mid(px + 1, py - (px + 1));
		xx.Trim();

		double tmp = _wtof(xx.GetBuffer());
		if (tmp > maxX)
			maxX = tmp;
		else if (tmp < minX)
			minX = tmp;

		CString yy = aLine.Mid(py + 1, pz - (py + 1));
		yy.Trim();

		tmp = _wtof(yy.GetBuffer());
		if (tmp > maxY)
			maxY = tmp;
		else if (tmp < minY)
			minY = tmp;
	}

	aFile.Close();

	std::cout << "Datei enthaelt " << line << " Zeilen." << std::endl;
	std::cout << "Groesse der Zeichnung in der Datei:\n";
	std::cout << "Links " << minX << "mm  Rechts " << maxX << "mm    Oben " << maxY << "mm  Unten " << minY << "mm " << std::endl;
	std::cout << "Breite " << (maxX - minX) << "mm  Hoehe " << (maxY - minY) << "mm" << std::endl;

	return true;
}

void GetPaperSize(PaperFormat pap, int&papX, int&papY)
{
	// All paper formats given in PORTRAIT-orientation
	switch (pap)
	{
	case PaperFormat::DIN_A0:
		papX = 841;
		papY = 1189;
		std::cout << "Papier Format ist DIN A0" << std::endl;
		break;
	case PaperFormat::DIN_A1:
		papX = 594;
		papY = 841;
		std::cout << "Papier Format ist DIN A1" << std::endl;
		break;
	case PaperFormat::DIN_A2:
		papX = 420;
		papY = 594;
		std::cout << "Papier Format ist DIN A2" << std::endl;
		break;
	case PaperFormat::DIN_A3:
		papX = 297;
		papY = 420;
		std::cout << "Papier Format ist DIN A3" << std::endl;
		break;
	case PaperFormat::DIN_A4:
		papX = 210;
		papY = 297;
		std::cout << "Papier Format ist DIN A4" << std::endl;
		break;
	case PaperFormat::DIN_A5:
		papX = 148;
		papY = 210;
		std::cout << "Papier Format ist DIN A5" << std::endl;
		break;
	case PaperFormat::DIN_A6:
		papX = 105;
		papY = 148;
		std::cout << "Papier Format ist DIN A6" << std::endl;
	default:
		papX = 210;
		papY = 297;
		std::cout << "Papier Format ist unbekannt, defaulting to DIN A4" << std::endl;
		break;
	}
	
}

CString GetPaperSizeName(PaperFormat pap)
{
	CString retVal;
	switch (pap)
	{
	case PaperFormat::DIN_A0:
		retVal = "DIN A0";
		break;
	case PaperFormat::DIN_A1:
		retVal = "DIN A1";
		break;
	case PaperFormat::DIN_A2:
		retVal = "DIN A2";
		break;
	case PaperFormat::DIN_A3:
		retVal = "DIN A3";
		break;
	case PaperFormat::DIN_A4:
		retVal = "DIN A4";
		break;
	case PaperFormat::DIN_A5:
		retVal = "DIN A5";
		break;
	case PaperFormat::DIN_A6:
		retVal = "DIN A6";
	default:
		retVal = "unknown";
		break;
	}
	return retVal;
}


void GetSizeFactor(CString filename, int papX, int papY, PaperOrientation orientation, double &scale, double &offX, double &offY)
{
	scale = 1.0;
	offX = 0.0;
	offY = 0.0;

	double minX, maxX, minY, maxY;
	if (!GetDimensionsOfDrawing(minX, maxX, minY, maxY, filename))
		return;

	double width = maxX - minX;
	double height = maxY - minY;

	if ((orientation == PaperOrientation::LANDSCAPE) ||
		((orientation == PaperOrientation::AUTO) && (width > height)))
	{
		// swap height/width
		int tmp = papX;
		papX = papY;
		papY = tmp;
	}

	std::cout << "Papiergroesse: " << papX << "mm x " << papY << "mm" << std::endl;

	// consider limits of plotter here (and only here)
	if (papX > X_SEPARATION)
		papX = X_SEPARATION;
	if (papY > ((Y_TWINEHOLES - 60) * 2)) // -60 cause we could never reach an angle of 0 degrees. Adopt to your plotter physics.
		papY = (Y_TWINEHOLES - 60) * 2;

	papX -= 20; // need space for pen-sledge at the edges of paper 
	papY -= 20;

	// consider a safety margin for displaced paper here.
	papX -= 10;
	papY -= 10;

	int halfX = papX / 2;
	int halfY = papY / 2;

	double scaleX = papX / width;
	double scaleY = papY / height;
	scale = (scaleX < scaleY) ? scaleX : scaleY;

	// calculate a horizontal/vertikal offset to center the image exactly
	offX = (minX + maxX) / -2.0;
	offY = (minY + maxY) / -2.0;

	std::cout << "Skalierung: " << scale << "  X-Offset " << offX << "  Y-Offset " << offY << std::endl;
	std::cout << "Resultat auf dem Papier: " << width * scale << "mm x " << height * scale << "mm" << std::endl;
	std::cout << "Ausrichtung des Papiers: " << ((papX > papY) ? "Landscape" : "Portrait") << std::endl;
}

void WriteToParamFile(CString filename, double scale, double offX, double offY)
{
	filename += ".par";
	CStdioFile aFile;
	if (!aFile.Open(filename, CFile::modeWrite | CFile::modeCreate))
	{
		std::cout << "Could not open parameter file for writing: <" << filename << ">" << std::endl;
		return;
	}
	CString line;
	CString format = "%f\n";
	line.Format(format, scale);
	aFile.WriteString(line);
	line.Format(format, offX);
	aFile.WriteString(line);
	line.Format(format, offY);
	aFile.WriteString(line);
	aFile.Close();
}

void PrintSyntax()
{
	std::cout << "Syntax: nc_range.exe <filename.nc> [<paper-size> [<orientation>]]" << std::endl;
	std::cout << " - <filename.nc>: the name of your nc-file" << std::endl;
	std::cout << " - <paper-size>: either A0, A1, ...,A6 for DIN A0, DIN A1, ...DIN A6. Default is DIN A4" << std::endl;
	std::cout << " -               or two integers xxx xxx for size in mm" << std::endl;
	std::cout << " - <orientation>: P for portrait or L for landscape. Default is automatic-best-choice." << std::endl;
}
int main(int argc, char* argv[])
{
	// argv[0] is the programms name
	// argv[1] is the filename of a nc file
	// argv[2] is either the din-size id oder the width in mm
	// argv[3] is either the height in mm or the orientation
	// argv[4] is the orientation

	if ((argc < 2) || (argc > 5) || (argc >= 3 && (strlen(argv[2]) < 2)))
	{
		PrintSyntax();
		return 0;
	}
	CString filename = argv[1];

	CString cstrOri = "A";
	int sizeX = 0;
	int sizeY = 0;
	if (argc <= 2)
		GetPaperSize(PaperFormat::DIN_A4, sizeX, sizeY);
	else
	{
		CString din = argv[2];
		if (din[0] == 'A')
		{
			CString din = argv[2][1];
			switch (atoi((const char *)din.GetBuffer()))
			{
			case 0:
				GetPaperSize(PaperFormat::DIN_A0, sizeX, sizeY);
				break;
			case 1:
				GetPaperSize(PaperFormat::DIN_A1, sizeX, sizeY);
				break;
			case 2:
				GetPaperSize(PaperFormat::DIN_A2, sizeX, sizeY);
				break;
			case 3:
				GetPaperSize(PaperFormat::DIN_A3, sizeX, sizeY);
				break;
			case 4:
				GetPaperSize(PaperFormat::DIN_A4, sizeX, sizeY);
				break;
			case 5:
				GetPaperSize(PaperFormat::DIN_A5, sizeX, sizeY);
				break;
			case 6:
				GetPaperSize(PaperFormat::DIN_A6, sizeX, sizeY);
				break;
			default:
				GetPaperSize(PaperFormat::DIN_A4, sizeX, sizeY);
				break;
			}

			if (argc == 4)
				cstrOri = argv[3];
		}
		else
		{
			if (argc < 4)
			{
				PrintSyntax();
				return 0;
			}
			sizeX = atoi(argv[2]);
			sizeY = atoi(argv[3]);
			// must be portrait format here, will be switched later
			if (sizeX > sizeY)
			{
				sizeX = atoi(argv[3]);
				sizeY = atoi(argv[2]);
			}
			if (argc == 5)
				cstrOri = argv[4];
		}
	}

	PaperOrientation ori;

		cstrOri.MakeUpper();
		if (cstrOri == "P")
			ori = PaperOrientation::PORTRAIT;
		else if (cstrOri == "L")
			ori = PaperOrientation::LANDSCAPE;
		else
			ori = PaperOrientation::AUTO;

	double scale;
	double offX;
	double offY;
	GetSizeFactor(filename, sizeX, sizeY, ori, scale, offX, offY);
	WriteToParamFile(filename, scale, offX, offY);
	return 0;
}
