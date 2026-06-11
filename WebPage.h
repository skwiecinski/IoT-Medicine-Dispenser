#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>PillBox Panel</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <style>
    body { font-family: Helvetica, sans-serif; text-align: center; margin:0; background-color: #f0f2f5; color: #333; }
    h2 { background-color: #007bff; color: white; padding: 15px; margin: 0; }
    h3 { color: #007bff; border-bottom: 2px solid #007bff; display: inline-block; padding-bottom: 5px; margin-top: 30px; }
    form { display: inline-block; text-align: left; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); margin-top: 20px; max-width: 400px; width: 90%; }
    input { width: 100%; padding: 10px; margin: 5px 0 15px 0; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
    input[type=submit], button { background-color: #28a745; color: white; border: none; padding: 12px 20px; cursor: pointer; width: 100%; font-size: 16px; border-radius: 4px; margin-top: 5px; }
    button.test { background-color: #17a2b8; margin-top: 10px; }
    button.dispense { background-color: #dc3545; margin-top: 30px; font-weight: bold; } 
    label { font-weight: bold; font-size: 14px; color: #555; }
    .group { border-top: 1px solid #eee; margin-top: 10px; padding-top: 10px; }
    .time-inputs { display: flex; gap: 10px; }
    .test-panel { background: #e9ecef; padding: 15px; border-radius: 8px; text-align: center; }
  </style>
</head>
<body>
  <h2>⚙️ PillBox Pro v6.0</h2>
  <form action="/save" method="POST">
    <div class="group"><label>WiFi SSID:</label><input type="text" name="ssid" value="%SSID%"></div>
    <div><label>WiFi Password:</label><input type="password" name="pass" value="%PASS%"></div>
    <div class="group"><label>GSM Number:</label><input type="text" name="gsm" value="%GSM%"></div>
    <div class="group"><label>Sender Email:</label><input type="text" name="em_u" value="%EM_U%"></div>
    <div><label>App Password:</label><input type="password" name="em_p" value="%EM_P%"></div>
    <div><label>Recipient Email:</label><input type="text" name="em_r" value="%EM_R%"></div>
    <div class="group"><label>Time to take (min):</label><input type="number" name="window" value="%WINDOW%" min="1" max="60"></div>
    <div class="group"><label>Dose 1:</label><div class="time-inputs"><input type="number" name="h1" value="%H1%"><input type="number" name="m1" value="%M1%"></div></div>
    <div><label>Dose 2:</label><div class="time-inputs"><input type="number" name="h2" value="%H2%"><input type="number" name="m2" value="%M2%"></div></div>
    <div><label>Dose 3:</label><div class="time-inputs"><input type="number" name="h3" value="%H3%"><input type="number" name="m3" value="%M3%"></div></div>
    <input type="submit" value="💾 SAVE">
  </form>
  <form class="test-panel">
    <h3>🛠 Tests</h3>
    <button class="test" formmethod="POST" formaction="/test/motor">⚙️ Motor</button>
    <button class="test" formmethod="POST" formaction="/test/buzzer">🔊 Buzzer</button>
    <button class="test" formmethod="POST" formaction="/test/gsm">📱 SMS</button>
    <button class="test" formmethod="POST" formaction="/test/email">📧 Email</button>
  </form>
  <form action="/dispense" method="POST"><button class="dispense" onclick="return confirm('Dispense pill?')">💊 DISPENSE NOW</button></form>
  <br><br>
</body></html>
)rawliteral";
