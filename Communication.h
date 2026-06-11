#pragma once
#include "Globals.h"

void connectWiFi();
void checkIncomingSMS();
void processSMS(String text);
void processEmail(String subject, String message, bool attachLog);
void notificationTask(void * parameter);
void sendSMSAsync(String text);
void sendEmailAsync(String subject, String message, bool attachLog);
