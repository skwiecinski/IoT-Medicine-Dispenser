#include "MotorControl.h"

void motorOff() {
  digitalWrite(M_IN1, LOW); digitalWrite(M_IN2, LOW);
  digitalWrite(M_IN3, LOW); digitalWrite(M_IN4, LOW);
}

void dispensePill(int doseIndex) {
  if (currentSlot >= MAX_SLOTS) {
    logToSD("ALARM: Empty magazine!");
    sendSMSAsync("ALARM: Box empty!");
    lcd.clear(); lcd.print("EMPTY BOX!");
    for(int i=0; i<5; i++) { digitalWrite(BUZZER_PIN, HIGH); delay(500); digitalWrite(BUZZER_PIN, LOW); delay(500); }
    return; 
  }
  
  logToSD("Dispensing dose " + String(doseIndex));
  lcd.clear(); lcd.print("TIME FOR PILL!"); lcd.setCursor(0,1); lcd.print("Press BUTTON");
  
  myStepper.setSpeed(10); myStepper.step(STEPS_PER_SLOT); motorOff(); 
  currentSlot++; 
  preferences.begin("pillbox", false); preferences.putInt("slot", currentSlot); preferences.end();

  unsigned long startTime = millis();
  bool taken = false;
  unsigned long windowMillis = (unsigned long)config_dose_window * 60000UL; 
  
  while(millis() - startTime < windowMillis) {
    server.handleClient(); 
    digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW); delay(100);
    digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW); 
    for(int k=0; k<20; k++) { delay(100); if(digitalRead(BUTTON_PIN) == LOW) { taken = true; break; } }
    if(taken) break; 
  }

  if(taken) {
    lcd.clear(); lcd.print("THANK YOU!"); logToSD("Pill taken");
    sendEmailAsync("Confirmation", "Patient took pill on time.", false); 
    for(int i=0; i<3; i++) { digitalWrite(BUZZER_PIN, HIGH); delay(50); digitalWrite(BUZZER_PIN, LOW); delay(50); }
  } else {
    lcd.clear(); lcd.print("NO REACTION!"); logToSD("ALARM: PILL MISSED");
    sendSMSAsync("WARNING! Patient did NOT take pill!");
    sendEmailAsync("ALARM: PILL MISSED", "No button confirmation.", false);
    digitalWrite(BUZZER_PIN, HIGH); delay(2000); digitalWrite(BUZZER_PIN, LOW); 
    lcd.clear(); lcd.print("PILL MISSED!");
    
    // Indefinite alarm loop until acknowledged
    while(true) {
      server.handleClient(); 
      digitalWrite(BUZZER_PIN, HIGH); delay(200); digitalWrite(BUZZER_PIN, LOW);
      for(int i=0; i<30; i++) { delay(100); if(digitalRead(BUTTON_PIN) == LOW) { taken = true; break; } }
      if(taken) break; 
    }
    lcd.clear(); lcd.print("Phew... THANK YOU");
    sendEmailAsync("Info", "Patient took pill after alarm.", false); 
  }
  
  if(doseIndex != 99) doseTaken[doseIndex] = true; 
  lcd.clear();
}
