#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>      
#include <Wire.h>            
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>         
#include <SPI.h>
#include <SD.h>              
#include <Keypad.h>          
#include <Stepper.h>          
#include <HardwareSerial.h>  
#include <ESP_Mail_Client.h>  
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"  
#include "Config.h"

// System objects
extern WebServer server;
extern Preferences preferences; 
extern LiquidCrystal_I2C lcd; 
extern RTC_DS3231 rtc;
extern HardwareSerial sim800; 
extern SMTPSession smtp;
extern Stepper myStepper;
extern Keypad keypad;

// RTOS
enum TaskType { TASK_SMS, TASK_EMAIL };
struct NotificationData { 
  TaskType type; 
  char subject[64]; 
  char message[160]; 
  bool attachLog; 
};

extern QueueHandle_t notificationQueue; 
extern SemaphoreHandle_t sdMutex;       
extern volatile bool smsDispenseRequest; 

// Configuration state
extern bool config_ap_enabled; 
extern String config_gsm_nr, config_wifi_ssid, config_wifi_pass;
extern String config_email_host, config_email_user, config_email_pass, config_email_recipient;
extern int config_email_port;
extern int config_dose_window; 
extern String web_user, web_pass;

// Application state
extern int doseH[3]; 
extern int doseM[3];  
extern bool doseTaken[3];
extern int currentDay;
extern String keyBuffer; 
extern int currentSlot; 

// General helper
void logToSD(String msg);
String getNextDoseTime();
