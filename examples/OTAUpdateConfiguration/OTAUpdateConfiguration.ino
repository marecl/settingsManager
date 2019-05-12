#include <settingsManager.h>

/* Settings */
const char* _name = "Workbench";
const char* _login = "admin";
const char* _pass = "admin";

/* WiFi */
const char* _ssid = "";
const char* _stapass = "";

settingsManager settings;

void setup() {
  Serial.begin(115200);
  pinMode(D0, OUTPUT);

  settings.name(_name);
  settings.configUser(_login, _pass);
  settings.configSTA(_ssid, _stapass);

  if (settings.beginSTA()) {
    Serial.print("Connected to ");
    Serial.println(settings.ssid());
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Unable to connect to WiFi\r\nRebooting");
    delay(1000);
    ESP.reset();
  }

  settings.beginOTA();
}

void loop() {
  while (millis() % 1000 != 0)
    ArduinoOTA.handle();
  digitalWrite(D0, !digitalRead(D0));
}
