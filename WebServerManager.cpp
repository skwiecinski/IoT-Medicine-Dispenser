#include "WebServerManager.h"

void sendResponse(String message) {
  String html = "<!DOCTYPE HTML><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Helvetica;text-align:center;background-color:#f0f2f5;margin-top:50px;}";
  html += "h1{color:#333;} a{display:inline-block;background-color:#007bff;color:white;padding:15px 30px;text-decoration:none;border-radius:5px;margin-top:20px;font-weight:bold;box-shadow:0 4px 6px rgba(0,0,0,0.1);}</style></head>";
  html += "<body><h1>" + message + "</h1>";
  html += "<a href='/'>BACK TO MENU</a></body></html>";
  server.send(200, "text/html", html);
}

void handleRoot() {
  if (!server.authenticate(web_user.c_str(), web_pass.c_str())) return server.requestAuthentication();
  String html = index_html;
  html.replace("%SSID%", config_wifi_ssid); html.replace("%PASS%", config_wifi_pass);
  html.replace("%GSM%", config_gsm_nr); html.replace("%EM_U%", config_email_user);
  html.replace("%EM_P%", config_email_pass); html.replace("%EM_R%", config_email_recipient);
  html.replace("%H1%", String(doseH[0])); html.replace("%M1%", String(doseM[0]));
  html.replace("%H2%", String(doseH[1])); html.replace("%M2%", String(doseM[1]));
  html.replace("%H3%", String(doseH[2])); html.replace("%M3%", String(doseM[2]));
  html.replace("%WINDOW%", String(config_dose_window)); 
  server.send(200, "text/html", html);
}

void handleSave() {
  if (!server.authenticate(web_user.c_str(), web_pass.c_str())) return;
  
  config_wifi_ssid = server.arg("ssid"); config_wifi_pass = server.arg("pass");
  config_gsm_nr = server.arg("gsm"); config_email_user = server.arg("em_u");
  config_email_pass = server.arg("em_p"); config_email_recipient = server.arg("em_r");
  config_dose_window = server.arg("window").toInt();
  doseH[0] = server.arg("h1").toInt(); doseM[0] = server.arg("m1").toInt();
  doseH[1] = server.arg("h2").toInt(); doseM[1] = server.arg("m2").toInt();
  doseH[2] = server.arg("h3").toInt(); doseM[2] = server.arg("m3").toInt();
  
  preferences.begin("pillbox", false);
  preferences.putString("ssid", config_wifi_ssid); preferences.putString("pass", config_wifi_pass);
  preferences.putString("gsm", config_gsm_nr); preferences.putString("emu", config_email_user);
  preferences.putString("emp", config_email_pass); preferences.putString("emr", config_email_recipient);
  preferences.putInt("window", config_dose_window);
  preferences.putInt("h1", doseH[0]); preferences.putInt("m1", doseM[0]);
  preferences.putInt("h2", doseH[1]); preferences.putInt("m2", doseM[1]);
  preferences.putInt("h3", doseH[2]); preferences.putInt("m3", doseM[2]);
  preferences.end();
  
  server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='10;url=/'><style>body{font-family:sans-serif;text-align:center;margin-top:50px;}</style></head><body><h1>Saved!</h1><p>Device is restarting...</p><p>Page will refresh automatically (if WiFi connects).</p></body></html>");
  delay(1000); ESP.restart(); 
}

void handleDispense() { if (server.authenticate(web_user.c_str(), web_pass.c_str())) { sendResponse("Dispensing..."); dispensePill(99); } }
void handleTestMotor() { if (server.authenticate(web_user.c_str(), web_pass.c_str())) { sendResponse("Motor test."); lcd.clear(); lcd.print("MOTOR TEST"); myStepper.setSpeed(10); myStepper.step(100); delay(200); myStepper.step(-100); motorOff(); lcd.clear(); } }
void handleTestBuzzer() { if (server.authenticate(web_user.c_str(), web_pass.c_str())) { sendResponse("Buzzer test."); for(int i=0; i<3; i++) { digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW); delay(100); } } }
void handleTestGSM() { if (server.authenticate(web_user.c_str(), web_pass.c_str())) { sendResponse("Background SMS."); sendSMSAsync("TEST SMS"); } }
void handleTestEmail() { if (server.authenticate(web_user.c_str(), web_pass.c_str())) { sendResponse("Background Email."); sendEmailAsync("Test", "Dual Core OK", false); } }
