#include "PinConfig.h"
#include <SD.h>
#include "SPI.h"
#include "FS.h"
#include "SDCardReader.h"
#include "MenuHandler.h"

// #define DEBUG_SDCARD

#ifndef DEBUG_SDCARD

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif

#endif

void SortStringList(String* list, int listsize)
{
	int i = 0;
	bool found = false;
	do
	{
		found = false;
		for (int i = 0; i < listsize - 1; ++i)
		{
			int cmp = list[i].compareTo(list[i + 1]);
			if (cmp > 0)
			{
				String tmp = list[i];
				list[i] = list[i + 1];
				list[i + 1] = tmp;
				found = true;
			}
		}
	} while (found);
}


String allfiles[MAX_FILES];
int filecnt = 0;

void ScanDirectory(SDFS& filesys, const char* dirname, uint8_t levels)
{
	filecnt = 0;
	for (int i = 0; i < MAX_FILES; ++i)
		allfiles[i] = "";

	DEBUG_PRINTLN("Scanning directory");
	SDFile root = filesys.open(dirname);
	if (!root)
	{
		DEBUG_PRINTLN("Failed to open directory" + String(dirname));
		return;
	}

	if (!root.isDirectory())
	{
		DEBUG_PRINTLN("Not a directory");
		return;
	}

	SDFile file = root.openNextFile();
	while (file && (filecnt < MAX_FILES))
	{
		if (!file.isDirectory())
		{
			// DEBUG_PRINTLN(file.name());
			String name = file.name();
			DEBUG_PRINTLN(name);
			name.toUpperCase();
			if (!name.endsWith(".NC"))
			{
				//DEBUG_PRINTLN(name + " not ending with .NC");
				file = root.openNextFile();
				continue;
			}
			// name = name.substring(1); // skip leading '/', but only in arduino-lib, not in ESP32
			// name = name.substring(0, name.length() - 3); // cut ".nc"
			allfiles[filecnt] = file.name(); // original non-uppercase
			++filecnt;
		}

		file = root.openNextFile();
	}
	DEBUG_PRINT("Filecount:"); DEBUG_PRINTLN(filecnt);
	SortStringList(allfiles, filecnt);
}

void ScanDirectory()
{
	ScanDirectory(SD, "/", 0);
}

String GetFile(int idx)
{
	if ((idx >= MAX_FILES) || (idx < 0))
		return "";
	return allfiles[idx];
}

void SetupSDCard()
{
	pinMode(PIN_SD_CS, OUTPUT);
	digitalWrite(PIN_SD_CS, HIGH);
	SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI);

	if (!SD.begin(PIN_SD_CS))
	{
		DEBUG_PRINTLN("Card Mount Failed");
		return;
	}

	uint8_t cardType = SD.cardType();
	if (cardType == CARD_NONE)
	{
		DEBUG_PRINTLN("No SD card attached");
		return;
	}

	DEBUG_PRINT("SD Card Type: ");
	if (cardType == CARD_MMC)
	{
		DEBUG_PRINTLN("MMC");
	}
	else if (cardType == CARD_SD)
	{
		DEBUG_PRINTLN("SDSC");
	}
	else if (cardType == CARD_SDHC)
	{
		DEBUG_PRINTLN("SDHC");
	}
	else
	{
		DEBUG_PRINTLN("UNKNOWN CARD TYPE");
	}

	DEBUG_PRINT("SD Card Size:");
	DEBUG_PRINT(SD.cardSize() / (1024 * 1024));
	DEBUG_PRINTLN(" MB");
}
