#include <settingsManager.h>

settingsManager settings;

const char* ssid = "AccessPointDemo";
const char* wifipass = "password";

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Configuring device...");
  settings.configAP(ssid, wifipass);

  if (settings.beginAP()) {
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("Gateway: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("Could not start Access Point");
    Serial.println(ssid);
    delay(1000);
    ESP.reset();
  }
}

void loop() {
}