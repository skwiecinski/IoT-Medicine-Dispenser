#pragma once
#include "Globals.h"
#include "WebPage.h"
#include "MotorControl.h"
#include "Communication.h"

void sendResponse(String message);
void handleRoot();
void handleSave();
void handleDispense();
void handleTestMotor();
void handleTestBuzzer();
void handleTestGSM();
void handleTestEmail();
