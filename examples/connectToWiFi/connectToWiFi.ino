#include <settingsManager.h>

settingsManager settings;

const char* ssid = "";
const char* wifipass = "";

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Configuring device...");
  settings.configSTA(ssid, wifipass);

  if (settings.beginSTA()) {
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("Connect to ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("Could not connect to");
    Serial.println(ssid);
    delay(1000);
    ESP.reset();
  }
}

void loop() {
}
