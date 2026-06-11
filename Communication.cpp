#include "Communication.h"

void connectWiFi() {
  if(WiFi.status() == WL_CONNECTED) return;
  if(config_wifi_ssid == "") { logToSD("[WIFI] No SSID!"); return; }

  logToSD("[WIFI] Connecting: " + config_wifi_ssid);
  WiFi.disconnect();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  WiFi.begin(config_wifi_ssid.c_str(), config_wifi_pass.c_str());
  
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 40) { 
    vTaskDelay(500 / portTICK_PERIOD_MS); 
    attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) logToSD("[WIFI] SUCCESS! IP: " + WiFi.localIP().toString());
  else logToSD("[WIFI] FAILED.");
}

void checkIncomingSMS() {
  while(sim800.available()) sim800.read(); 
  sim800.println("AT+CMGL=\"REC UNREAD\""); 
  unsigned long waitStart = millis();
  String response = "";
  
  while((millis() - waitStart) < 1000) { while(sim800.available()) response += (char)sim800.read(); }

  if(response.indexOf("+CMGL:") != -1) { 
    Serial.println("[BG] SMS Received");
    if(response.indexOf(config_gsm_nr) != -1) {
      response.toLowerCase();
      if(response.indexOf("dispense") != -1) {
        logToSD("[SMS] DISPENSE command authorized!");
        smsDispenseRequest = true; 
      }
    }
  }
}

void processSMS(String text) {
  if(config_gsm_nr.length() < 3) return; 
  logToSD("[BG] SMS START: " + text);
  sim800.println("AT+CMGF=1"); delay(200); 
  while(sim800.available()) sim800.read();
  sim800.print("AT+CMGS=\""); sim800.print(config_gsm_nr); sim800.println("\"");
  delay(200);
  while(sim800.available()) sim800.read();
  sim800.print(text); delay(200); sim800.write(26); 
  logToSD("[BG] SMS SENT");
}

void processEmail(String subject, String message, bool attachLog) {
  connectWiFi(); 
  if(WiFi.status() != WL_CONNECTED) { logToSD("[BG] Email ERROR: No WiFi"); return; }
  
  ESP_Mail_Session session;
  session.server.host_name = config_email_host; session.server.port = config_email_port;
  session.login.email = config_email_user; session.login.password = config_email_pass;
  session.login.user_domain = "";
  
  SMTP_Message msg;
  msg.sender.name = "Smart Pillbox"; msg.sender.email = config_email_user;
  msg.subject = subject; msg.addRecipient("Caregiver", config_email_recipient);
  msg.text.content = message.c_str();

  if(attachLog) {
    SMTP_Attachment att;
    att.file.path = "/log.txt"; att.file.storage_type = esp_mail_file_storage_type_sd;
    att.descr.mime = "text/plain"; att.descr.filename = "log.txt";
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
    msg.addAttachment(att);
  }

  if(attachLog) xSemaphoreTake(sdMutex, pdMS_TO_TICKS(10000));
  
  if (!smtp.connect(&session)) Serial.println("Mail Connect Error");
  else if (!MailClient.sendMail(&smtp, &msg)) Serial.println("Mail Send Error");
  else Serial.println("Mail SENT OK");
  
  if(attachLog) xSemaphoreGive(sdMutex);
}

void notificationTask(void * parameter) {
  NotificationData req;
  for(;;) {
    if(xQueueReceive(notificationQueue, &req, pdMS_TO_TICKS(3000)) == pdPASS) {
      if(req.type == TASK_SMS) processSMS(String(req.message));
      else if(req.type == TASK_EMAIL) processEmail(String(req.subject), String(req.message), req.attachLog);
    } else {
      if(config_gsm_nr.length() > 3) checkIncomingSMS();
    }
  }
}

void sendSMSAsync(String text) {
  NotificationData req; req.type = TASK_SMS;
  strncpy(req.message, text.c_str(), sizeof(req.message) - 1); req.message[sizeof(req.message) - 1] = 0;
  xQueueSend(notificationQueue, &req, 0);
}

void sendEmailAsync(String subject, String message, bool attachLog) {
  NotificationData req; req.type = TASK_EMAIL;
  strncpy(req.subject, subject.c_str(), sizeof(req.subject) - 1);
  strncpy(req.message, message.c_str(), sizeof(req.message) - 1);
  req.attachLog = attachLog;
  xQueueSend(notificationQueue, &req, 0);
}
