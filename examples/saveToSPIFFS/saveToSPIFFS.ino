#include <settingsManager.h>
#include "FS.h"

/*
  Simple example of saving/loading config
  What is saved:
    - Name
    - Login and password
    - AP ssid and password
    - WiFi ssid and password
    - IP settings
    - DHCP settings
    - NTP settings
    - Auth token lifespan
    - 'Some' interval (generic purpose)
    - Time of last NTP update (set manually)

*/

/* Remember name shouldn't contain space */
const char* _name = "DemoName";

void setup() {
  Serial.begin(115200);
  Serial.println();
  SPIFFS.begin();

  /* Creating an object with filename allows it to be saved */
  Serial.println("Creating settings object");
  settingsManager settings("/testFile.txt");
  Serial.print("Setting name ");
  settings.name(_name);
  Serial.println(settings.name());
  settings.save();

  /* Load new settings */
  Serial.println("Creating new settings object");
  settingsManager settings2("/testFile.txt");
  Serial.println("Loading settings...");
  bool loadStatus = settings2.load();
  if (loadStatus) {
    Serial.print("Device name is ");
    Serial.println(settings2.name());
  } else {
    Serial.println("Unable to load settings");
  }

  SPIFFS.end();
}

void loop() {

}