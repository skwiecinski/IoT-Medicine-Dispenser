#include "Globals.h"
#include "MotorControl.h"
#include "Communication.h"
#include "WebServerManager.h"
#include "AdminMenu.h"

// Define global objects here
WebServer server(80);
Preferences preferences; 
LiquidCrystal_I2C lcd(0x27, 20, 4); 
RTC_DS3231 rtc;
HardwareSerial sim800(1); 
SMTPSession smtp;
Stepper myStepper(STEPS_PER_REV, M_IN1, M_IN3, M_IN2, M_IN4);

char keys[4][4] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
byte rowPins[4] = {5, 6, 7, 15};
byte colPins[4] = {16, 17, 18, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

QueueHandle_t notificationQueue; 
SemaphoreHandle_t sdMutex;       
volatile bool smsDispenseRequest = false; 

// Define Config variables here
bool config_ap_enabled = false; 
String config_gsm_nr = "", config_wifi_ssid = "", config_wifi_pass = "";
String config_email_host = "smtp.gmail.com", config_email_user = "", config_email_pass = "", config_email_recipient = "";
int config_email_port = 465;
int config_dose_window = 15; 
String web_user = "admin", web_pass = "admin";

int doseH[3] = {8, 13, 20}; 
int doseM[3] = {0, 0, 0};  
bool doseTaken[3] = {false, false, false};
int currentDay = -1;
String keyBuffer = ""; 
int currentSlot = 0; 

// General helper
void logToSD(String msg) {
  DateTime now = rtc.now();
  char timestamp[25];
  sprintf(timestamp, "%04d-%02d-%02d %02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute());
  String fullLog = String(timestamp) + " -> " + msg;
  Serial.println("[LOG]: " + fullLog);

  if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    File file = SD.open("/log.txt", FILE_APPEND);
    if(file) { file.println(fullLog); file.close(); }
    xSemaphoreGive(sdMutex); 
  }
}

String getNextDoseTime() {
  DateTime now = rtc.now();
  int nowMins = now.hour() * 60 + now.minute();
  char buff[40];
  for(int i=0; i<3; i++) {
    int doseMins = doseH[i] * 60 + doseM[i];
    if(doseMins > nowMins) { sprintf(buff, "%02d:%02d", doseH[i], doseM[i]); return String(buff); }
  }
  sprintf(buff, "%02d:%02d (Tomorrow)", doseH[0], doseM[0]); 
  return String(buff);
}

void loadSettings() {
  preferences.begin("pillbox", true); 
  config_wifi_ssid = preferences.getString("ssid", ""); config_wifi_pass = preferences.getString("pass", "");
  config_gsm_nr = preferences.getString("gsm", ""); config_email_user = preferences.getString("emu", "");
  config_email_pass = preferences.getString("emp", ""); config_email_recipient = preferences.getString("emr", "");
  config_ap_enabled = preferences.getBool("ap_on", false); 
  config_dose_window = preferences.getInt("window", 15); 
  doseH[0] = preferences.getInt("h1", 8);  doseM[0] = preferences.getInt("m1", 0);
  doseH[1] = preferences.getInt("h2", 13); doseM[1] = preferences.getInt("m2", 0);
  doseH[2] = preferences.getInt("h3", 20); doseM[2] = preferences.getInt("m3", 0);
  currentSlot = preferences.getInt("slot", 0);
  preferences.end();
}

void setup() {
  Serial.begin(115200); 
  delay(2000); 
  
  sdMutex = xSemaphoreCreateMutex();
  notificationQueue = xQueueCreate(10, sizeof(NotificationData));
  
  xTaskCreatePinnedToCore(
    notificationTask, 
    "NotifyTask", 
    131072, 
    NULL, 
    1, 
    NULL, 
    1       
  );

  pinMode(BUZZER_PIN, OUTPUT); digitalWrite(BUZZER_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  loadSettings();

  if (config_ap_enabled == false && config_wifi_ssid == "") {
      Serial.println("EMERGENCY AP START FOR CONFIG");
      config_ap_enabled = true;
  }

  if (config_ap_enabled) {
    WiFi.mode(WIFI_AP_STA); 
    WiFi.softAP("PillBox_Setup", "admin1234");
  } else {
    WiFi.mode(WIFI_STA);
  }

  if(config_wifi_ssid != "") {
    WiFi.begin(config_wifi_ssid.c_str(), config_wifi_pass.c_str());
  }

  server.on("/", handleRoot); 
  server.on("/save", HTTP_POST, handleSave);
  server.on("/dispense", HTTP_POST, handleDispense);
  server.on("/test/motor", HTTP_POST, handleTestMotor); 
  server.on("/test/buzzer", HTTP_POST, handleTestBuzzer);
  server.on("/test/gsm", HTTP_POST, handleTestGSM); 
  server.on("/test/email", HTTP_POST, handleTestEmail);
  server.begin();

  Wire.begin(I2C_SDA, I2C_SCL); 
  lcd.init(); 
  lcd.backlight();
  
  Wire1.begin(RTC_SDA, RTC_SCL); 
  rtc.begin(&Wire1);
  
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS_PIN); 
  if(!SD.begin(SD_CS_PIN, SPI)) Serial.println("SD CARD ERROR");
  
  sim800.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  delay(1000); 
  sim800.println("ATE0"); 
  delay(500); 
  while(sim800.available()) sim800.read();
  
  motorOff();
}

void loop() {
  server.handleClient(); 
  DateTime now = rtc.now();

  if(smsDispenseRequest) { smsDispenseRequest = false; dispensePill(99); }

  char key = keypad.getKey();
  if (key) {
    digitalWrite(BUZZER_PIN, HIGH); delay(20); digitalWrite(BUZZER_PIN, LOW); 
    keyBuffer += key;
    if (keyBuffer.length() > 5) keyBuffer = keyBuffer.substring(keyBuffer.length() - 5);
    if (keyBuffer == ADMIN_PASS) { logToSD("Physical Admin Panel"); physicalAdminPanel(); }
  }

  if (now.day() != currentDay) { for(int i=0; i<3; i++) doseTaken[i] = false; currentDay = now.day(); }
  
  for (int i=0; i<3; i++) { 
    if (!doseTaken[i] && now.hour() == doseH[i] && now.minute() == doseM[i] && now.second() == 0) {
        dispensePill(i); 
    }
  }

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    char line0[21], line1[21], line3[21];
    snprintf(line0, 21, "%02d:%02d %02d.%02d.%04d", now.hour(), now.minute(), now.day(), now.month(), now.year());
    
    if(currentSlot >= MAX_SLOTS) snprintf(line1, 21, "EMPTY! REFILL");
    else snprintf(line1, 21, "Status: %d / 21", currentSlot);
    
    String next = getNextDoseTime();
    snprintf(line3, 21, "%s", next.c_str());
    
    lcd.setCursor(0,0); lcd.print(line0); 
    lcd.setCursor(0,1); lcd.print(line1);
    lcd.setCursor(0,2); lcd.print("Next dose:"); 
    lcd.setCursor(0,3); lcd.print(line3);
  }
}
