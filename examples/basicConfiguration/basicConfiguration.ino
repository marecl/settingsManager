#include <settingsManager.h>

/* Saving/loading disabled */
//settingsManager settings;

/* Saving/loading enabled */
settingsManager settings("/testdata.txt");

void setup() {
  Serial.begin(115200);
  Serial.println("settingsManager basic demo");

  /* Blinking LED */
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);

  /* Remember that SPIFFS must be initialized by you */
  SPIFFS.begin();
  if (!settings.load()) {
    Serial.println("Failed to load settings");
    settings.name("Workbench");
    settings.configSTA("", "");
    settings.configAP("Workbench", "password");
    settings.configUser("admin", "admin");
    settings.useDHCP(true);
    if (!settings.useDHCP())
      /* ip, gateway, mask */
      settings.configIP("192.168.2.55", "192.168.2.1", "255.255.255.0");
    settings.useNTP = true;
    
    /* Single server, not a pool */
    settings.ntpServer("tempus1.gum.gov.pl");
    
    /* Remember that DLS is not supported */
    settings.timezone = 1;
    
    /* Tracking changes in settings */
    settings.lastUpdate = 0;
    
    /* Interval of anything you want [ms] */
    settings.readInterval = 1000;

    /* won't actually save if no file is speecified or SPIFFS is disabled */
    settings.save();
  } else Serial.println("Settings loaded");
  SPIFFS.end();
  if (settings.beginSTA()) {
    Serial.print("Connected to ");
    Serial.println(settings.ssid());
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Unable to connect to WiFi\r\nRebooting");
    delay(1000);
    ESP.reset();
  }

  Serial.print("Name:\t\t");
  Serial.println(settings.name());
  Serial.print("STA SSID:\t");
  Serial.println(settings.ssid());
  Serial.print("AP SSID:\t");
  Serial.println(settings.ssidAP());
  Serial.print("NTP server:\t");
  Serial.println(settings.ntpServer());
  Serial.print("Last update:");
  Serial.println(settings.lastUpdate);

  Serial.println();
  Serial.println("Beginning flashing LED on D0");
  Serial.println("Time will be synced every ~10 seconds");
  Serial.println("Settings will be saved every ~30 seconds");
  Serial.println();
}

void loop() {
  if (millis() % settings.readInterval == 0) {
    digitalWrite(D0, !digitalRead(D0));
    /* Wait for the condition to become invalid */
    /* Only when event is faster than condition */
    while (millis() % settings.readInterval == 0);
  }

  if (millis() % 10000 == 1) {
    /* Sync every ~10 seconds */
    uint32_t newTime = syncTime();
    Serial.print("Current time:\t");
    Serial.println(newTime);

    /* Save every ~30 seconds */
    if (newTime - settings.lastUpdate >= 30) {
      Serial.println("Saving settings");
      settings.lastUpdate = newTime;
      SPIFFS.begin();
      settings.save();
      SPIFFS.end();
    }
  }
}

uint32_t syncTime() {
  WiFiUDP udp;
  IPAddress timeServerIP;
  udp.begin(2390);
  WiFi.hostByName(settings.ntpServer(), timeServerIP);

  byte packetBuffer[48];
  memset(packetBuffer, 0, 48);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  int cb = udp.parsePacket();

  uint8_t i = 0;
  while (!cb && i++ < 10) {
    udp.beginPacket(timeServerIP, 123);
    udp.write(packetBuffer, 48);
    udp.endPacket();
    udp.flush();
    delay(1000);
    cb = udp.parsePacket();
  }

  if (i < 10 && cb) {
    udp.read(packetBuffer, 48);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    secsSince1900 -= 2208988800UL;
    secsSince1900 += settings.timezone * 3600;
    return secsSince1900;
  }
  return false;
}
