#pragma once
#include <Arduino.h>

// Pin Definitions
#define GSM_RX_PIN 41 
#define GSM_TX_PIN 42 
#define SD_CS_PIN 13
#define SD_SCK 11
#define SD_MISO 10
#define SD_MOSI 12
#define BUZZER_PIN 48
#define BUTTON_PIN 47  
#define I2C_SDA 1
#define I2C_SCL 2
#define RTC_SDA 9
#define RTC_SCL 46
#define M_IN1 38
#define M_IN2 37
#define M_IN3 36
#define M_IN4 35

// Motor and slot config
const int STEPS_PER_REV = 2048; 
const int MAX_SLOTS = 21;       
const int STEPS_PER_SLOT = 97;  

// Passwords
const String ADMIN_PASS = "A1234"; 
