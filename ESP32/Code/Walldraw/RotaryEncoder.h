#pragma once

void SetupRotary();
bool WasRotaryButtonPressed(bool reset = true);
int GetRotaryIncrement(bool reset = true);
void FlushRotaryEvents();
