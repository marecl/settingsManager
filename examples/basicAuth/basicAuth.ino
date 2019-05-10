#include <settingsManager.h>
#include <ESP8266WebServer.h>

settingsManager settings;
ESP8266WebServer server;

/* Remember that passwords stored in global variables aren't very safe */
const char* username = "admin";
const char* password = "admin";
const char* ssid = "";
const char* wifipass = "";

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Configuring device...");
  settings.configUser(username, password);
  settings.configSTA(ssid, wifipass);

  server.on("/", handleRoot);
  server.begin();

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

void handleRoot() {
  /* Check if we're logged in */
  if (!settings.webAuthenticate(&server))
    return server.requestAuthentication();

  server.sendContent(F("<html><head><title>ESP8266 Server</title>"));
  server.sendContent(F("</head><body><h1>You have successfully lo"));
  server.sendContent(F("gged in!</h1></body></html>"));
}

void loop() {
  server.handleClient();
}