#include "PinConfig.h"
#include "OLED_Display.h"
#include "RotaryEncoder.h"
#include "MenuHandler.h"
#include "Sledge.h"
#include "BatteryProtect.h"
#include "SDCardReader.h"
#include "Drawing.h"
#include "OLED_Display.h"
#include "Pen.h"
#include "SysData.h"
#include "FileHandler.h"
#include "Stepper.h"

// #define DEBUG_MENUHANDLER

#ifndef DEBUG_MENUHANDLER
#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#define DEBUG_PRINTLN(a)
#undef DEBUG_PRINT
#define DEBUG_PRINT(a)
#endif
#endif

String SelectedFile;

#pragma region MENU_BASICS
MenuFct currentMenuFctButton = NULL;
MenuFct currentMenuFctScrollCCW = NULL;
MenuFct currentMenuFctScrollCW = NULL;

#define MAX_MENU_ENTRIES 101 // 100 files and the exit selection
String menu[MAX_MENU_ENTRIES];
int menuSize = 0;
int selected = 0;
int currentDisplayStart = 0;

void SetMenuFctButton(MenuFct fct)
{
	currentMenuFctButton = fct;
}

void SetMenuFctScrollCCW(MenuFct fct)
{
	currentMenuFctScrollCCW = fct;
}

void SetMenuFctScrollCW(MenuFct fct)
{
	currentMenuFctScrollCW = fct;
}

void ClearMenu()
{
	while (menuSize >= 0)
	{
		menu[menuSize] = "";
		menuSize--;
	}

	menuSize = 0;
	currentMenuFctButton = NULL;
	currentMenuFctScrollCCW = NULL;
	currentMenuFctScrollCW = NULL;
	selected = 0;
}

int AddMenuEntry(String txt)
{
	if (menuSize >= MAX_MENU_ENTRIES)
		return -1;
	menu[menuSize] = txt;
	menuSize++;
	return menuSize;
}

String GetMenuText(int idx)
{
	if ((idx < 0) || (idx > MAX_MENU_ENTRIES))
		return "";
	return menu[idx];
}

int GetIdxOfText(String text)
{
	for (int i = 0; i < MAX_MENU_ENTRIES; ++i)
	{
		if (text.startsWith(GetMenuText(i)))
		{
			return i;
		}
	}
	return 0;
}

bool IsSelected(String txt)
{
	if ((txt == "") || (menu[selected] == ""))
		return false;
	if (txt.compareTo(menu[selected]) == 0)
		return true;
	return false;
}

String GetSelectedText()
{
	return menu[selected];
}

void DisplayMenu(int IncSelected, int start)
{
	ClearDisplayContent();
	int tmpselected = selected + IncSelected;
	if (tmpselected < 0)
		tmpselected = 0;
	if (tmpselected >= MAX_MENU_ENTRIES)
		tmpselected = MAX_MENU_ENTRIES - 1;
	if (menu[tmpselected] != "")
		selected = tmpselected;

	if (start == -1)
		start = currentDisplayStart;

	if (start > selected) // the selection must be visible
		start = selected;
	if (selected > (start + VISIBLE_MENULINES - 1))
		start = selected - VISIBLE_MENULINES + 1;
	if (start < 0)
		start = 0;
	else if (start >= MAX_MENU_ENTRIES)
		start = MAX_MENU_ENTRIES - 1;

	currentDisplayStart = start;

	for (int i = start; (i < MAX_MENU_ENTRIES) && (i < (start + VISIBLE_MENULINES)); ++i)
	{
		if (menu[i] == "")
			break;
		AddLineToDisplay(menu[i], (i == selected));
	}
	ShowDisplay();
}

void OnButtonPressed()
{
//	DEBUG_PRINTLN("OnButtonPressed");
	if (currentMenuFctButton == NULL)
		return;

	currentMenuFctButton(selected);
}

void OnScrollCCW()
{
	if (currentMenuFctScrollCCW == NULL)
	{
		DisplayMenu(-1);
		return;
	}
	currentMenuFctScrollCCW(selected);
}

void OnScrollCW()
{
	if (currentMenuFctScrollCW == NULL)
	{
		DisplayMenu(1);
		return;
	}
	currentMenuFctScrollCW(selected);
}

void HandleMenu()
{
	if (WasRotaryButtonPressed())
	{
		OnButtonPressed();
		return;
	}
	int inc = GetRotaryIncrement();
	if (inc < 0)
		OnScrollCW();
	else if (inc > 0)
		OnScrollCCW();
}

#pragma endregion

void PopulateBaseMenu();
void PopulateSelectFileMenu();
void PopulateSelectSizeMenu();
void PopulateDrawingMenu();
void PopulateSettingsMenu();
void PopulateMovePenMenu();
void PopulateWidthMenu();
void PopulateHeightMenu();
void PopulateMoveHorizontalMenu();
void PopulateMoveVerticalMenu();
void PopulateSetPenUpPosition();
void PopulateSetPenDownPosition();
void PopulatePreviewMenu();
void PopulateDimensionsMenu();
void PopulatePenAngleMenu();
void PopulateSetOffsetMenu();
void PopulateSetXOffset();
void PopulateSetYOffset();
void PopulateSpeedMenu();	// This one returns to settings menu
void PopulateSpeedMenu2(); // This one returns to drawing menu

const String menuEntryExit = "Go back";

/***************************** Base menu *******************************/

const String menuEntryBaseSelectFile = "Select file";
const String menuEntryBaseStartDrawing = "Start drawing";
const String menuEntryBasePreview = "Preview";
const String menuEntryBaseSettings = "Settings";
const String menuEntryBaseMovePen = "Move pen";

void OnBaseSelected(int idx)
{
	String sel = GetMenuText(idx);
	if (IsSelected(menuEntryBaseSelectFile))
	{
		PopulateSelectFileMenu();
	}
	else if (IsSelected(menuEntryBaseStartDrawing))
	{
		PrepareProgressBar("", "");
		StartDrawing();
		PopulateDrawingMenu();
	}
	else if (IsSelected(menuEntryBasePreview))
	{
		PopulatePreviewMenu();
	}
	else if (IsSelected(menuEntryBaseMovePen))
	{
		PopulateMovePenMenu();
	}
	else if (IsSelected(menuEntryBaseSettings))
	{
		PopulateSettingsMenu();
	}
}

void PopulateBaseMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryBaseSelectFile);
	AddMenuEntry(menuEntryBaseStartDrawing);
	AddMenuEntry(menuEntryBasePreview);
	AddMenuEntry(menuEntryBaseMovePen);
	AddMenuEntry(menuEntryBaseSettings);
	SetMenuFctButton(OnBaseSelected);
	DisplayMenu(0);
}

/***************************** Preview *******************************/

void OnPreviewSelected(int idx)
{
	PopulateBaseMenu();
}

void OnScrollCCWPreview(int no)
{
	// ignore
}

void OnScrollCWPreview(int no)
{
	// ignore
}

void PopulatePreviewMenu()
{
	if (SelectedFile == "")
	{
		PopulateBaseMenu();
		return;
	}
	IsBatteryMeasuring = false;
	ClearMenu();
	DrawFileToDisplay();
	SetMenuFctButton(OnPreviewSelected);
	SetMenuFctScrollCCW(OnScrollCCWPreview);
	SetMenuFctScrollCW(OnScrollCWPreview);
	FlushRotaryEvents(); // if rotary encoder was touched during drawing, ignore this
}


/***************************** Select file menu *******************************/

void OnSelectFileSelected(int idx)
{
	String previousSelectedFile = SelectedFile;

	String text = GetSelectedText();
	if (text == menuEntryExit)
	{
		PopulateBaseMenu();
		return;
	}

	// SelectedFile = '/' + allfiles[selected-1] + ".NC";  ---> no leading '/' in lib of ESP32
	SelectedFile = allfiles[selected - 1];
	DEBUG_PRINTLN("Selected file: " + SelectedFile);

	if (previousSelectedFile != SelectedFile)
		ScanFileContent(SelectedFile);

	PopulateBaseMenu();
}

void PopulateSelectFileMenu()
{
	IsBatteryMeasuring = false;
	ClearMenu();
	ScanDirectory();
	AddMenuEntry(menuEntryExit);

	for (int i = 0; i < filecnt; ++i)
	{
		DEBUG_PRINTLN(allfiles[i]);
		AddMenuEntry(allfiles[i].substring(0, allfiles[i].length() - 3)); // cut ".nc"
	}
	SetMenuFctButton(OnSelectFileSelected);

	if (SelectedFile.length() > 3)
	{
		for (int idx = 0; idx < MAX_FILES; ++idx)
		{
			if (!allfiles[idx].compareTo(SelectedFile))
			{
				DisplayMenu(idx+1);
				return;
			}
		}
	}

	DisplayMenu(0);
}

/***************************** Start drawing *******************************/

const String menuEntryDrawPause("Pause");
const String menuEntryDrawContinue("Continue");
const String menuEntryDrawAbort("Abort");
const String menuEntryDrawSpeed("Speed");

void OnScrollCCWDrawing(int no)
{
	DisplayMenu(-1);
	PaintProgressBar(-1);
}

void OnScrollCWDrawing(int no)
{
	DisplayMenu(1);
	PaintProgressBar(-1);
}

void OnDrawSelected(int idx)
{
	String text = GetSelectedText();
	if (text == menuEntryDrawAbort)
	{
		RestorePenState(); // just to delete a stored state. EndDrawing moves pen up anyway.
		EndDrawing();
		PopulateBaseMenu();
	}
	else if (text == menuEntryDrawContinue)
	{
		DrawingIsRunning = true;
		PopulateDrawingMenu(); // Restores Pen state
	}
	else if (text == menuEntryDrawPause)
	{
		DrawingIsRunning = false;
		StorePenState();
		PopulateDrawingMenu();
	}
	else if(text == menuEntryDrawSpeed)
	{
		StorePenState();
		DrawingIsRunning = false; // User has to press "continue after speed-menu returns
		PopulateSpeedMenu2();
	}
}

void PopulateDrawingMenu()
{
	RestorePenState(); // if it returns from speed setting, continue state
	IsBatteryMeasuring = true;
	ClearMenu();
	DrawingIsRunning ? AddMenuEntry(menuEntryDrawPause) : AddMenuEntry(menuEntryDrawContinue);
	AddMenuEntry(menuEntryDrawAbort);
	AddMenuEntry(menuEntryDrawSpeed);
	SetMenuFctButton(OnDrawSelected);
	DisplayMenu(0);
	SetMenuFctScrollCCW(OnScrollCCWDrawing);
	SetMenuFctScrollCW(OnScrollCWDrawing);
	PaintProgressBar(-1);
}

/***************************** Settings menu *******************************/

const String menuEntrySettingsPaperSize = "Paper size";
const String menuEntrySettingsSpeed = "Speed";
const String menuEntrySettingsDimensions = "Plotter dimensions";
const String menuEntrySettingsPenAngle = "Pen angles";
const String menuEntrySettingsOffsets = "Draw offsets";
const String menuEntrySettingsReverseRotationOn = "Reverse motors on";
const String menuEntrySettingsReverseRotationOff = "Reverse motors off";

void OnSettingsSelected(int idx)
{
	String text = GetSelectedText();
	if (IsSelected(menuEntryExit))
	{
		PopulateBaseMenu();
	}
	else if (IsSelected(menuEntrySettingsPaperSize))
	{
		PopulateSelectSizeMenu();
	}
	else if (IsSelected(menuEntrySettingsSpeed))
	{
		PopulateSpeedMenu();
	}
	else if (IsSelected(menuEntrySettingsDimensions))
	{
		PopulateDimensionsMenu();
	}
	else if (IsSelected(menuEntrySettingsPenAngle))
	{
		PopulatePenAngleMenu();
	}
	else if (IsSelected(menuEntrySettingsOffsets))
	{
		PopulateSetOffsetMenu();
	}
	else if (IsSelected(menuEntrySettingsReverseRotationOn) || IsSelected(menuEntrySettingsReverseRotationOff))
	{
		InvertStepperRotation();
		PopulateSettingsMenu();
	}
}

void PopulateSettingsMenu()
{
	IsBatteryMeasuring = false;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	AddMenuEntry(menuEntrySettingsPaperSize);
	AddMenuEntry(menuEntrySettingsSpeed);
	AddMenuEntry(menuEntrySettingsDimensions);
	AddMenuEntry(menuEntrySettingsPenAngle);
	AddMenuEntry(menuEntrySettingsOffsets);

	int ReelOutL, ReelInL, ReelOutR, ReelInR;
	ReadMotorRotation(ReelOutL, ReelInL, ReelOutR, ReelInR);
	AddMenuEntry(ReelOutL == -1 ? menuEntrySettingsReverseRotationOn : menuEntrySettingsReverseRotationOff);
	SetMenuFctButton(OnSettingsSelected);
	DisplayMenu(0);
}



/*************************** Select speed menu *****************************/

int aValue;
void OnScrollCCWSelectSpeed(int no)
{
	aValue--;
	if (aValue < 1)
		aValue = 1;
	DrawExtraLine("Level " + String(aValue), 32, 42);
}

void OnScrollCWSelectSpeed(int no)
{
	aValue++;
	if (aValue > 10)
		aValue = 10;
	DrawExtraLine("Level " + String(aValue), 32, 42);
}

void OnSelectSpeedMenu(int idx)
{
	SaveSpeed(aValue);
	SetStepperTiming();
	PopulateSettingsMenu();
}

void OnSelectSpeedMenu2(int idx)
{
	SaveSpeed(aValue);
	SetStepperTiming();
	DrawingIsRunning = true;
	PopulateDrawingMenu();
}

void PopulateSpeedMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	aValue = ReadSpeed();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectSpeedMenu);
	SetMenuFctScrollCCW(OnScrollCCWSelectSpeed);
	SetMenuFctScrollCW(OnScrollCWSelectSpeed);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine("Range: 1 - 10", 0, 30);
	DrawExtraLine("Level " + String(aValue), 32, 42);
}

void PopulateSpeedMenu2()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	aValue = ReadSpeed();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectSpeedMenu2);
	SetMenuFctScrollCCW(OnScrollCCWSelectSpeed);
	SetMenuFctScrollCW(OnScrollCWSelectSpeed);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine("Range: 1 - 10", 0, 30);
	DrawExtraLine("Level " + String(aValue), 32, 42);
}



/******************* Setting dimensions menu *******************************/
const String menuEntryConfigWidth = "Set plotter width";
const String menuEntryConfigHeight = "Set plotter height";

void OnScrollCCWDimensions(int no)
{
	DisplayMenu(-1);
	DrawExtraLine(String(ReadPlotterWidth()) + " / " + String(ReadPlotterHeight()) + " mm", 20, 40);
}

void OnScrollCWDimensions(int no)
{
	DisplayMenu(1);
	DrawExtraLine(String(ReadPlotterWidth()) + " / " + String(ReadPlotterHeight()) + " mm", 20, 40);
}


void OnDimensionsSelected(int idx)
{
	String text = GetSelectedText();
	if (IsSelected(menuEntryExit))
	{
		PopulateSettingsMenu();
	}
	else if (IsSelected(menuEntryConfigWidth))
	{
		PopulateWidthMenu();
	}
	else if (IsSelected(menuEntryConfigHeight))
	{
		PopulateHeightMenu();
	}
}

void PopulateDimensionsMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	AddMenuEntry(menuEntryConfigWidth);
	AddMenuEntry(menuEntryConfigHeight);
	SetMenuFctButton(OnDimensionsSelected);
	SetMenuFctScrollCCW(OnScrollCCWDimensions);
	SetMenuFctScrollCW(OnScrollCWDimensions);
	DisplayMenu(0);
	DrawExtraLine(String(ReadPlotterWidth()) + " / " + String(ReadPlotterHeight()) + " mm", 20, 40);
}

/******************* Setting pen angle menu ********************************/
const String menuEntryConfigPenUp = "Set pen-up angle";
const String menuEntryConfigPenDown = "Set pen-down angle";

void OnScrollCCWPenAngle(int no)
{
	DisplayMenu(-1);
	DrawExtraLine(String(ReadPenUpPos()) + " / " + String(ReadPenDownPos()), 30, 40);
}

void OnScrollCWPenAngle(int no)
{
	DisplayMenu(1);
	DrawExtraLine(String(ReadPenUpPos()) + " / " + String(ReadPenDownPos()), 30, 40);
}

void OnPenAngleSelected(int idx)
{
	String text = GetSelectedText();
	if (IsSelected(menuEntryExit))
	{
		PopulateSettingsMenu();
	}
	else if (IsSelected(menuEntryConfigPenUp))
	{
		PopulateSetPenUpPosition();
	}
	else if (IsSelected(menuEntryConfigPenDown))
	{
		PopulateSetPenDownPosition();
	}
}

void PopulatePenAngleMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	AddMenuEntry(menuEntryConfigPenUp);
	AddMenuEntry(menuEntryConfigPenDown);
	SetMenuFctButton(OnPenAngleSelected);
	SetMenuFctScrollCCW(OnScrollCCWPenAngle);
	SetMenuFctScrollCW(OnScrollCWPenAngle);
	DisplayMenu(0);
	DrawExtraLine(String(ReadPenUpPos()) + " / " + String(ReadPenDownPos()), 30, 40);
}

/******************* Setting offset menu ***********************************/
const String menuEntrySetXOffset = "Set X offset";
const String menuEntrySetYOffset = "Set Y offset";

void OnScrollCCWOffset(int no)
{
	DisplayMenu(-1);
	DrawExtraLine(String(ReadXOffset()) + " / " + String(ReadYOffset()) + " mm", 20, 40);
}

void OnScrollCWOffset(int no)
{
	DisplayMenu(1);
	DrawExtraLine(String(ReadXOffset()) + " / " + String(ReadYOffset()) + " mm", 20, 40);
}

void OnSetOffsetSelected(int idx)
{
	String text = GetSelectedText();
	if (IsSelected(menuEntryExit))
	{
		PopulateSettingsMenu();
	}
	else if (IsSelected(menuEntrySetXOffset))
	{
		PopulateSetXOffset();
	}
	else if (IsSelected(menuEntrySetYOffset))
	{
		PopulateSetYOffset();
	}
}

void PopulateSetOffsetMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	AddMenuEntry(menuEntrySetXOffset);
	AddMenuEntry(menuEntrySetYOffset);
	SetMenuFctButton(OnSetOffsetSelected);
	SetMenuFctScrollCCW(OnScrollCCWOffset);
	SetMenuFctScrollCW(OnScrollCWOffset);
	DisplayMenu(0);
	DrawExtraLine(String(ReadXOffset()) + " / " + String(ReadYOffset()) + " mm", 20, 40);
}


#define OFFSET_INCREMENT 10
void OnScrollCCWSetXOffset(int no)
{
	shiftPaperX -= OFFSET_INCREMENT;
	if (shiftPaperX < -maxPlotterX)
	{
		shiftPaperX = -maxPlotterX;
		int x = shiftPaperX / OFFSET_INCREMENT; // round to full offset parts
		shiftPaperX = x * OFFSET_INCREMENT;
	}
	DrawExtraLine(String(shiftPaperX) + " / " + String(shiftPaperY) + " mm", 30, 40);
}

void OnScrollCWSetXOffset(int no)
{
	shiftPaperX += OFFSET_INCREMENT;
	if (shiftPaperX > maxPlotterX)
	{
		shiftPaperX = maxPlotterX;
		int x = shiftPaperX / OFFSET_INCREMENT; // round to full offset parts
		shiftPaperX = x * OFFSET_INCREMENT;
	}
	DrawExtraLine(String(shiftPaperX) + " / " + String(shiftPaperY) + " mm", 30, 40);
}

void OnSelectSetXOffset(int idx)
{
	SaveXOffset(shiftPaperX);
	PopulateSetOffsetMenu();
}

void PopulateSetXOffset()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectSetXOffset);
	SetMenuFctScrollCCW(OnScrollCCWSetXOffset);
	SetMenuFctScrollCW(OnScrollCWSetXOffset);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine(String(shiftPaperX) + " / " + String(shiftPaperY) + " mm", 30, 40);
}

void OnScrollCCWSetYOffset(int no)
{
	shiftPaperY -= OFFSET_INCREMENT;
	if (shiftPaperY < -(upperPlotterY - bottomPlotterY))
	{
		shiftPaperY = -(upperPlotterY - bottomPlotterY);
		int y = shiftPaperY / OFFSET_INCREMENT;
		shiftPaperY = y * OFFSET_INCREMENT;
	}
	DrawExtraLine(String(shiftPaperX) + " / " + String(shiftPaperY) + " mm", 30, 40);
}

void OnScrollCWSetYOffset(int no)
{
	shiftPaperY += OFFSET_INCREMENT;
	if (shiftPaperY > 0)
	{
		shiftPaperY = 0;
	}
	DrawExtraLine(String(shiftPaperX) + " / " + String(shiftPaperY) + " mm", 30, 40);
}

void OnSelectSetYOffset(int idx)
{
	SaveYOffset(shiftPaperY);
	PopulateSetOffsetMenu();
}

void PopulateSetYOffset()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectSetYOffset);
	SetMenuFctScrollCCW(OnScrollCCWSetYOffset);
	SetMenuFctScrollCW(OnScrollCWSetYOffset);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine(String(shiftPaperX) + " / " + String(shiftPaperY) + " mm", 30, 40);
}

/***************************** Move pen menu *******************************/

// const String menuEntrySetInitPos = "Set initial position"; makes no sense
const String menuEntryMovePenUp = "Move pen up";
const String menuEntryMovePenDown = "Move pen down";
const String menuEntryMovePenHorizontal = "Move horizontal";
const String menuEntryMovePenVertical = "Move vertical";
const String menuEntryMovePenInitPos = "Move to 0/0 position";
const String menuEntryMovePenSetInitPos = "Set as 0/0 position";

void OnMovePenSelected(int idx)
{
	String text = GetSelectedText();
	if (IsSelected(menuEntryExit))
	{
		PopulateBaseMenu();
	}
	else if (IsSelected(menuEntryMovePenDown) || IsSelected(menuEntryMovePenUp))
	{
		IsPenUp() ? pen_down() : pen_up();
		PopulateMovePenMenu();
		DisplayMenu(1);
	}
	else if (IsSelected(menuEntryMovePenHorizontal))
	{
		pen_up();
		PopulateMoveHorizontalMenu();
	}
	else if (IsSelected(menuEntryMovePenVertical))
	{
		pen_up();
		PopulateMoveVerticalMenu();
	}
	else if (IsSelected(menuEntryMovePenInitPos))
	{
		pen_up();
		MoveSledgeIgnoreOffset(0, 0);
		PopulateBaseMenu();
	}
	else if (IsSelected(menuEntryMovePenSetInitPos))
	{
		InitZeroPosition();
		PopulateBaseMenu();
	}
}

void PopulateMovePenMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	AddMenuEntry(IsPenUp() ? menuEntryMovePenDown : menuEntryMovePenUp);
	AddMenuEntry(menuEntryMovePenHorizontal);
	AddMenuEntry(menuEntryMovePenVertical);
	AddMenuEntry(menuEntryMovePenInitPos);
	AddMenuEntry(menuEntryMovePenSetInitPos);
	SetMenuFctButton(OnMovePenSelected);
	DisplayMenu(0);
}


/********************** Select pen up position menu ************************/

#define PEN_ANGLE_INCREMENT 1
void OnScrollCCWSetPenUpPosition(int no)
{
	pen_up_angle -= PEN_ANGLE_INCREMENT;
	if (pen_up_angle < 0)
		pen_up_angle = 0;
	DrawExtraLine(String(pen_up_angle), 40, 40);
	pen_up();
}

void OnScrollCWSetPenUpPosition(int no)
{
	pen_up_angle += PEN_ANGLE_INCREMENT;
	if (pen_up_angle > 180)
		pen_up_angle = 180;
	DrawExtraLine(String(pen_up_angle), 40, 40);
	pen_up();
}

void OnSelectSetPenUpPosition(int idx)
{
	SavePenUpPos(pen_up_angle);
	pen_up();
	PopulatePenAngleMenu();
}

void PopulateSetPenUpPosition()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	pen_up_angle = ReadPenUpPos();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectSetPenUpPosition);
	SetMenuFctScrollCCW(OnScrollCCWSetPenUpPosition);
	SetMenuFctScrollCW(OnScrollCWSetPenUpPosition);
	DisplayMenu(0);
	pen_up();
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine(String(pen_up_angle), 40, 40);
}


/********************** Select pen down position menu **********************/
void OnScrollCCWSetPenDownPosition(int no)
{
	pen_down_angle -= PEN_ANGLE_INCREMENT;
	if (pen_down_angle < 0)
		pen_down_angle = 0;
	pen_down();
	DrawExtraLine(String(pen_down_angle), 40, 40);
}

void OnScrollCWSetPenDownPosition(int no)
{
	pen_down_angle += PEN_ANGLE_INCREMENT;
	if (pen_down_angle > 180)
		pen_down_angle = 180;
	DrawExtraLine(String(pen_down_angle), 40, 40);
	pen_down();
}

void OnSelectSetPenDownPosition(int idx)
{
	SavePenDownPos(pen_down_angle);
	pen_up();
	PopulatePenAngleMenu();
}

void PopulateSetPenDownPosition()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	pen_down_angle = ReadPenDownPos();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectSetPenDownPosition);
	SetMenuFctScrollCCW(OnScrollCCWSetPenDownPosition);
	SetMenuFctScrollCW(OnScrollCWSetPenDownPosition);
	DisplayMenu(0);
	pen_down();
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine(String(pen_down_angle), 40, 40);
}


/*************************** Select width menu *****************************/

#define SCROLL_INCREMENT 5
void OnScrollCCWSelectWidth(int no)
{
	aValue -= SCROLL_INCREMENT;
	if (aValue < 100)
		aValue = 100;
	DrawExtraLine(String(aValue) + " mm", 30, 40);
}

void OnScrollCWSelectWidth(int no)
{
	aValue += SCROLL_INCREMENT;
	if (aValue > 2500)
		aValue = 2500;
	DrawExtraLine(String(aValue) + " mm", 30, 40);
}

void OnSelectWidthMenu(int idx)
{
	SavePlatformWidth(aValue);
	InitPlotterRanges();
	PopulateDimensionsMenu();
}

void PopulateWidthMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	aValue = ReadPlotterWidth();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectWidthMenu);
	SetMenuFctScrollCCW(OnScrollCCWSelectWidth);
	SetMenuFctScrollCW(OnScrollCWSelectWidth);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine(String(aValue) + " mm", 30, 40);
}

/***************************** Select height menu *******************************/

void OnScrollCCWSelectHeight(int no)
{
	aValue -= SCROLL_INCREMENT;
	if (aValue < 100)
		aValue = 100;
	DrawExtraLine(String(aValue) + " mm", 30, 40);
}

void OnScrollCWSelectHeight(int no)
{
	aValue += SCROLL_INCREMENT;
	if (aValue > 2000)
		aValue = 2000;
	DrawExtraLine(String(aValue) + " mm", 30, 40);
}

void OnSelectHeightMenu(int idx)
{
	SavePlatformHeight(aValue);
	InitPlotterRanges();
	PopulateDimensionsMenu();
}

void PopulateHeightMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	aValue = ReadPlotterHeight();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnSelectHeightMenu);
	SetMenuFctScrollCCW(OnScrollCCWSelectHeight);
	SetMenuFctScrollCW(OnScrollCWSelectHeight);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to modify", 0, 20);
	DrawExtraLine(String(aValue) + " mm", 30, 40);
}

/***************************** Move horizontal menu *******************************/
#define PEN_MOVE_INCREMENT 5
void OnScrollCCWMoveHorizontal(int no)
{
	double x = GetCurDrawPosX();
	double y = GetCurDrawPosY();
	// round to full increments
	int tmp = x / PEN_MOVE_INCREMENT;
	x = tmp * PEN_MOVE_INCREMENT;
	x -= PEN_MOVE_INCREMENT;
	MoveSledgeIgnoreOffset(x, y);
	DrawExtraLine(String(GetCurDrawPosX()) + " / " + String(GetCurDrawPosY()) + " mm", 30, 40);
}

void OnScrollCWMoveHorizontal(int no)
{
	double x = GetCurDrawPosX();
	double y = GetCurDrawPosY();
	// round to full increments
	int tmp = x / PEN_MOVE_INCREMENT;
	x = tmp * PEN_MOVE_INCREMENT;
	x += PEN_MOVE_INCREMENT;
	MoveSledgeIgnoreOffset(x, y);
	DrawExtraLine(String(GetCurDrawPosX()) + " / " + String(GetCurDrawPosY()) + " mm", 30, 40);
}

void OnMoveHorizontalMenu(int idx)
{
	PopulateMovePenMenu();
}

void PopulateMoveHorizontalMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnMoveHorizontalMenu);
	SetMenuFctScrollCCW(OnScrollCCWMoveHorizontal);
	SetMenuFctScrollCW(OnScrollCWMoveHorizontal);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to move", 0, 20);
	DrawExtraLine(String(GetCurDrawPosX()) + " / " + String(GetCurDrawPosY()) + " mm", 30, 40);
}

/***************************** Move vertical menu *******************************/

void OnScrollCCWMoveVertical(int no)
{
	double x = GetCurDrawPosX();
	double y = GetCurDrawPosY();
	// round to full increments
	int tmp = y / PEN_MOVE_INCREMENT;
	y = tmp * PEN_MOVE_INCREMENT;
	y -= PEN_MOVE_INCREMENT;
	MoveSledgeIgnoreOffset(x, y);
	DrawExtraLine(String(GetCurDrawPosX()) + " / " + String(GetCurDrawPosY()) + " mm", 30, 40);
}

void OnScrollCWMoveVertical(int no)
{
	double x = GetCurDrawPosX();
	double y = GetCurDrawPosY();
	y += PEN_MOVE_INCREMENT;
	// round to full increments
	int tmp = y / PEN_MOVE_INCREMENT;
	y = tmp * PEN_MOVE_INCREMENT;
	MoveSledgeIgnoreOffset(x, y);
	DrawExtraLine(String(GetCurDrawPosX()) + " / " + String(GetCurDrawPosY()) + " mm", 30, 40);
}

void OnMoveVerticalMenu(int idx)
{
	PopulateMovePenMenu();
}

void PopulateMoveVerticalMenu()
{
	IsBatteryMeasuring = true;
	ClearMenu();
	AddMenuEntry(menuEntryExit);
	SetMenuFctButton(OnMoveVerticalMenu);
	SetMenuFctScrollCCW(OnScrollCCWMoveVertical);
	SetMenuFctScrollCW(OnScrollCWMoveVertical);
	DisplayMenu(0);
	DrawExtraLine("Rotate knob to move", 0, 20);
	DrawExtraLine(String(GetCurDrawPosX()) + " / " + String(GetCurDrawPosY()) + " mm", 30, 40);
}

/***************************** Select paper size menu *******************************/

const String menuEntrySizeNative = "Native";
const String menuEntrySizeA5L = "A5 Landscape 210x148";
const String menuEntrySizeA5P = "A5 Portrait 148x210";
const String menuEntrySizeA4L = "A4 Landscape 297x210";
const String menuEntrySizeA4P = "A4 Portrait 210x297";
const String menuEntrySizeA3L = "A3 Landscape 420x297";
const String menuEntrySizeA3P = "A3 Portrait 297x420";
const String menuEntrySizeA2L = "A2 Landscape 594x420";
const String menuEntrySizeA2P = "A2 Portrait 420x594";
const String menuEntrySizeA1L = "A1 Landscape 841x594";
const String menuEntrySizeA1P = "A1 Portrait 594x841";
const String menuEntrySizeA0L = "A0 Landscape 1189x841";
const String menuEntrySizeA0P = "A0 Portrait 841x1189";

void OnSelectSizeSelected(int idx)
{
	String text = GetSelectedText();
	int paperSizeX, paperSizeY;
	if (text == menuEntrySizeNative)
	{
		paperSizeX = 0;
		paperSizeY = 0;
	}
	else if (text == menuEntrySizeA5L)
	{
		paperSizeX = 210;
		paperSizeY = 148;
	}
	else if (text == menuEntrySizeA5P)
	{
		paperSizeX = 148;
		paperSizeY = 210;
	}
	else if (text == menuEntrySizeA4L)
	{
		paperSizeX = 297;
		paperSizeY = 210;
	}
	else if (text == menuEntrySizeA4P)
	{
		paperSizeX = 210;
		paperSizeY = 297;
	}
	else if (text == menuEntrySizeA3L)
	{
		paperSizeX = 420;
		paperSizeY = 297;
	}
	else if (text == menuEntrySizeA3P)
	{
		paperSizeX = 297;
		paperSizeY = 420;
	}
	else if (text == menuEntrySizeA2L)
	{
		paperSizeX = 594;
		paperSizeY = 420;
	}
	else if (text == menuEntrySizeA2P)
	{
		paperSizeX = 420;
		paperSizeY = 594;
	}
	else if (text == menuEntrySizeA1L)
	{
		paperSizeX = 841;
		paperSizeY = 594;
	}
	else if (text == menuEntrySizeA1P)
	{
		paperSizeX = 594;
		paperSizeY = 841;
	}
	else if (text == menuEntrySizeA0L)
	{
		paperSizeX = 1189;
		paperSizeY = 841;
	}
	else if (text == menuEntrySizeA0P)
	{
		paperSizeX = 841;
		paperSizeY = 1189;
	}
	else
	{
		// default: DIN A4 Landscape
		paperSizeX = 297;
		paperSizeY = 210;
	}
	SavePaperSize(paperSizeX, paperSizeY);
	PopulateSettingsMenu();
}

String GetCurrentPaperName()
{
	int paperSizeX, paperSizeY;
	ReadPaperSize(paperSizeX, paperSizeY);

	bool IsLandscape = (paperSizeX > paperSizeY) ? true : false;
	String paperName;
	if (paperSizeX == 0)
		paperName = menuEntrySizeNative;
	else if (paperSizeX == 148)
		paperName = menuEntrySizeA5P;
	else if (paperSizeX == 210)
		paperName = IsLandscape ? menuEntrySizeA5L : menuEntrySizeA4P;
	else if (paperSizeX == 297)
		paperName = IsLandscape ? menuEntrySizeA4L : menuEntrySizeA3P;
	else if (paperSizeX == 420)
		paperName = IsLandscape ? menuEntrySizeA3L : menuEntrySizeA2P;
	else if (paperSizeX == 594)
		paperName = IsLandscape ? menuEntrySizeA2L : menuEntrySizeA1P;
	else if (paperSizeX == 841)
		paperName = IsLandscape ? menuEntrySizeA1L : menuEntrySizeA0P;
	else if (paperSizeX == 1189)
		paperName = menuEntrySizeA0L;
	else
		paperName = menuEntrySizeNative;

	return paperName;
}

void PopulateSelectSizeMenu()
{
	IsBatteryMeasuring = false;
	ClearMenu();

	AddMenuEntry(menuEntrySizeNative);
	AddMenuEntry(menuEntrySizeA5L);
	AddMenuEntry(menuEntrySizeA5P);
	AddMenuEntry(menuEntrySizeA4L);
	AddMenuEntry(menuEntrySizeA4P);
	AddMenuEntry(menuEntrySizeA3L);
	AddMenuEntry(menuEntrySizeA3P);
	AddMenuEntry(menuEntrySizeA2L);
	AddMenuEntry(menuEntrySizeA2P);
	AddMenuEntry(menuEntrySizeA1L);
	AddMenuEntry(menuEntrySizeA1P);
	AddMenuEntry(menuEntrySizeA0L);
	AddMenuEntry(menuEntrySizeA0P);
	SetMenuFctButton(OnSelectSizeSelected);
	DisplayMenu(GetIdxOfText(GetCurrentPaperName()));
}

/***************************** Battery state *******************************/

void SetupMenu()
{
	SelectedFile = "";

	currentMenuFctButton = NULL;
	currentMenuFctScrollCCW = NULL;
	currentMenuFctScrollCW = NULL;
	selected = 0;
	currentDisplayStart = 0;
	for (int i = 0; i < MAX_MENU_ENTRIES; ++i)
		menu[i] = "";

	PopulateBaseMenu();
}
