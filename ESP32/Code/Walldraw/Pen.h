#pragma once

void SetupPen();

void pen_up();
void pen_down();
bool IsPenUp();
void StorePenState();
void RestorePenState();

void DeactivateServo();
void ReactivateServo();

extern int pen_up_angle;
extern int pen_down_angle;
