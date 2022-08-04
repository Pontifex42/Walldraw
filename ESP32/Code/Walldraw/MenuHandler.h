#pragma once

void SetupMenu();
void HandleMenu();

typedef void (*MenuFct)(int no);

void SetMenuFctButton(MenuFct fct);
void SetMenuFctScrollCW(MenuFct fct);
void SetMenuFctScrollCCW(MenuFct fct);

void ClearMenu();
int AddMenuEntry(String txt);
String GetMenuText(int idx);
void DisplayMenu(int IncSelected, int start = -1);

extern String SelectedFile;
void PopulateBaseMenu();