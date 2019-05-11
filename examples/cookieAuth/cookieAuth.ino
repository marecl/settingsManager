#include <ESP8266WebServer.h>
#include <settingsManager.h>

/*
  Cookie based authentication mechanism
  Handles timeouts and login page
*/

settingsManager settings;
ESP8266WebServer server;

/* Settings */
const char* _name = "Workbench";
const char* _login = "admin";
const char* _pass = "admin";

/* WiFi */
const char* _ssid = "";
const char* _stapass = "";

/* Server must collect cookies */
const char* headerkeys[] = {"Cookie"};

/* Valid init timebase */
uint32_t timebase;

void setup() {
  Serial.begin(115200);
  settings.name(_name);
  settings.configUser(_login, _pass);
  settings.configSTA(_ssid, _stapass);
  /* Token will be valid for 3 minutes */
  /* Depends what is used as counter */
  settings.tokenLifespan = 180;

  /* Configure server */
  server.collectHeaders(headerkeys, sizeof(headerkeys) / sizeof(char*));
  server.on("/login", handleLogin);
  server.on("/logout", handleLogout);
  server.on("/", handleRoot);
  server.begin();

  if (settings.beginSTA()) {
    Serial.print("Connected to ");
    Serial.println(settings.ssid());
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Unable to connect to WiFi\r\nRebooting");
    delay(1000);
    ESP.reset();
  }
}

void loop() {
  server.handleClient();
  timebase = millis() / 1000;
}

void handleLogin() {
  Serial.println("Log in");
  if (server.method() == HTTP_GET) {
    String content = "<html><body><form action='/login' method='POST'>";
    content += "User:<input type='text' name='USERNAME' placeholder='username'><br>";
    content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
    content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>";
    content += "</body></html>";
    server.send(200, "text/html", content);
  } else if (server.method() == HTTP_POST) {
    if (!verifyLogin()) {
      redirectToLogin();
      return;
    }
    Serial.println("Everything is correct");
    /* Redirect to root */
    server.sendHeader("Location", "/");
    server.send(301);
  }
}

void handleRoot() {
  /* Must be checked on every page */
  if (!verifyLogin())
    redirectToLogin();

  server.sendContent(F("<html><head><title>ESP8266 Server</title></he"));
  server.sendContent(F("ad><body><a href=\"/logout\"><input type=\"bu"));
  server.sendContent(F("tton\" value=\"Logout\" /></a><h1>You have su"));
  server.sendContent(F("ccessfully logged in!</h1></body></html>"));
}

void handleLogout() {
  /* Send incorrect cookie to avoid restoring session */
  server.sendHeader("Set-Cookie", settings.encryptKey(1));
  redirectToLogin();
}

bool verifyLogin() {
  String toVerify = server.header("Cookie");
  uint8_t status = settings.verifyEncryptedKey(toVerify, timebase);
  Serial.print("Cookie verification status: ");
  Serial.println(status);
  /* No cookie or it's expired */
  if (status >= 5) {
    /* Check if clients wants to authenticate */
    if (!server.hasArg("USERNAME") || !server.hasArg("PASSWORD"))
      return false;
    if (!settings.authenticate(server.arg("USERNAME").c_str(), server.arg("PASSWORD").c_str()))
      return false;
  } else if (status > 0)
    return false;
  /* Codes 1-4 are invalid */
  /* Clear cookies if you can't log in */

  /* Sending a cookie anyway to hold up the session */
  server.sendHeader("Set-Cookie", settings.encryptKey(timebase));
  return true;
}

void redirectToLogin() {
  server.sendHeader("Location", "/login");
  server.send(301);
  return void();
}
