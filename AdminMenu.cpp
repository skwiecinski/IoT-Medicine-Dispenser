#include "AdminMenu.h"

void adminAccessPointConfig() {
    bool inSubMenu = true;
    while(inSubMenu) {
        server.handleClient();
        lcd.clear(); lcd.setCursor(0,0); lcd.print("1. Enable AP"); lcd.setCursor(0,1); lcd.print("2. Disable AP");
        char k = 0;
        while(!k) { server.handleClient(); k = keypad.getKey(); if(k=='D') { inSubMenu = false; break; } }
        if(!inSubMenu) break;

        digitalWrite(BUZZER_PIN, HIGH); delay(50); digitalWrite(BUZZER_PIN, LOW);

        if(k == '1') { 
            if (config_ap_enabled && (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)) {
                lcd.clear(); lcd.print("ALREADY ENABLED!"); delay(1000);
            } else {
                lcd.clear(); lcd.print("Enabling AP...");
                WiFi.mode(WIFI_AP_STA); WiFi.softAP("PillBox_Setup", "admin1234");
                config_ap_enabled = true;
                preferences.begin("pillbox", false); preferences.putBool("ap_on", true); preferences.end();
                delay(1000); lcd.clear(); lcd.print("AP ACTIVE!"); delay(1000);
            }
        }
        else if(k == '2') {
            if (!config_ap_enabled) {
                lcd.clear(); lcd.print("ALREADY DISABLED!"); delay(1000);
            } else {
                lcd.clear(); lcd.print("Disabling AP...");
                WiFi.softAPdisconnect(true); WiFi.mode(WIFI_STA);
                config_ap_enabled = false;
                preferences.begin("pillbox", false); preferences.putBool("ap_on", false); preferences.end();
                delay(1000); lcd.clear(); lcd.print("AP DISABLED!"); delay(1000);
            }
        }
    }
}

void adminSetDoses() {
  for(int i=0; i<3; i++) {
    lcd.clear(); lcd.print("Dose " + String(i+1) + " (HHMM):"); lcd.setCursor(0,1);
    String buf = "";
    while(buf.length() < 4) {
      server.handleClient(); 
      char k = keypad.getKey();
      if(k) {
        digitalWrite(BUZZER_PIN, HIGH); delay(20); digitalWrite(BUZZER_PIN, LOW);
        if(k == 'D') return; 
        if(k >= '0' && k <= '9') { buf += k; lcd.print(k); }
      }
    }
    int h = buf.substring(0,2).toInt(); int m = buf.substring(2,4).toInt();
    if(h < 24 && m < 60) { doseH[i] = h; doseM[i] = m; lcd.print(" OK"); delay(500); } 
    else { lcd.print(" ERROR"); delay(1000); }
  }
  
  preferences.begin("pillbox", false);
  preferences.putInt("h1", doseH[0]); preferences.putInt("m1", doseM[0]);
  preferences.putInt("h2", doseH[1]); preferences.putInt("m2", doseM[1]);
  preferences.putInt("h3", doseH[2]); preferences.putInt("m3", doseM[2]);
  preferences.end();
  lcd.clear(); lcd.print("Saved!"); delay(1000);
}

void adminRunTests() {
  bool inTest = true;
  while(inTest) {
    server.handleClient();
    lcd.clear(); lcd.print("1.Mot 2.Buz"); lcd.setCursor(0,1); lcd.print("3.GSM 4.Email");
    char k = 0;
    while(!k) { server.handleClient(); k = keypad.getKey(); if(k == 'D') { inTest = false; break; } }
    if(!inTest) break;
    digitalWrite(BUZZER_PIN, HIGH); delay(20); digitalWrite(BUZZER_PIN, LOW);
    
    if(k == '1') { lcd.clear(); lcd.print("Motor Test"); myStepper.setSpeed(10); myStepper.step(100); delay(200); myStepper.step(-100); motorOff(); }
    else if(k == '2') { lcd.clear(); lcd.print("Buzzer Test"); for(int i=0; i<3; i++) { digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW); delay(100); } }
    else if(k == '3') { lcd.clear(); lcd.print("Background SMS..."); sendSMSAsync("SMS Test (Keypad)"); delay(1000); }
    else if(k == '4') { lcd.clear(); lcd.print("Background Email..."); sendEmailAsync("Test Email", "Keypad test", false); delay(1000); }
  }
}

void physicalAdminPanel() {
  lcd.clear(); lcd.print("MAIN MENU:"); delay(1000);
  bool inMenu = true;
  while(inMenu) {
    server.handleClient();
    lcd.clear(); 
    lcd.setCursor(0,0); lcd.print("1.Doses   2.Tests"); 
    lcd.setCursor(0,1); lcd.print("3.Disp    4.WiFi");
    lcd.setCursor(0,2); lcd.print("5.Fill    6.Log");
    lcd.setCursor(0,3); lcd.print("7.AP      8.ClrLog"); 
    
    char key = 0;
    while(!key) { server.handleClient(); key = keypad.getKey(); }
    digitalWrite(BUZZER_PIN, HIGH); delay(50); digitalWrite(BUZZER_PIN, LOW);
    
    if(key == '1') adminSetDoses();
    else if(key == '2') adminRunTests();
    else if(key == '3') dispensePill(99); 
    else if(key == '4') { lcd.clear(); lcd.print("WiFi Reset..."); preferences.begin("pillbox", false); preferences.putString("ssid", ""); preferences.end(); delay(1000); ESP.restart(); }
    else if(key == '5') {
        lcd.clear(); lcd.print("CALIBRATION..."); lcd.setCursor(0,1); lcd.print("Return to base");
        int stepsUsed = STEPS_PER_SLOT * MAX_SLOTS; int stepsMissing = STEPS_PER_REV - stepsUsed; 
        if (stepsMissing > 0) { myStepper.setSpeed(10); myStepper.step(stepsMissing); motorOff(); }
        currentSlot = 0; for(int i=0; i<3; i++) doseTaken[i] = false;
        preferences.begin("pillbox", false); preferences.putInt("slot", 0); preferences.end();
        lcd.clear(); lcd.print("READY! 0/21"); logToSD("RESET AND CALIBRATION"); delay(2000);
    }
    else if(key == '6') { lcd.clear(); lcd.print("Sending Logs..."); sendEmailAsync("MANUAL LOGS", "Task from LCD panel.", true); delay(2000); }
    else if(key == '7') { adminAccessPointConfig(); }
    
    else if(key == '8') {
        lcd.clear(); lcd.print("Deleting logs...");
        if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            SD.remove("/log.txt"); 
            File file = SD.open("/log.txt", FILE_WRITE); 
            if(file) { file.println("--- LOG CLEARED ---"); file.close(); lcd.clear(); lcd.print("DELETED!"); }
            else { lcd.clear(); lcd.print("SD ERROR!"); }
            xSemaphoreGive(sdMutex);
        } else { lcd.print("SD BUSY!"); }
        delay(2000);
    }
    else if(key == 'D') inMenu = false;
  }
  lcd.clear(); keyBuffer = "";
}
