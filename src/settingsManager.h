#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServerSecure.h>
#include "FS.h"

/*
  Insecure, uncomment only for debugging
  Will show every action in settingsManager including passwords
*/
#define S_DEBUG

class settingsManager {
  public:
    settingsManager();
    settingsManager(const char*);
    ~settingsManager();
    void save();
    bool load();
    void configSTA(const char*, const char* = NULL);
    void configAP(const char*, const char* = NULL);
    void ssid(const char*);
    void ssidAP(const char*);
    void configIP(IPAddress, IPAddress, IPAddress);
    void configIP(const char*, const char*, const char*);
    void useDHCP(bool);
    bool useDHCP();
    void name(const char*);
    void configUser(const char*, const char*);
    bool authenticate(const char*, const char*);
    bool beginWiFi();
    bool beginAP();
    void configUpdateServer(ESP8266WebServer*, ESP8266HTTPUpdateServer*, const char*);
    void beginOTA(uint16_t = 8266);
    bool webAuthenticate(ESP8266WebServer*);
    bool remove(const char*, const char*, const char*);
    IPAddress localIP();
    IPAddress gatewayIP();
    IPAddress subnetMask();
    const char* ssid();
    const char* ssidAP();
    const char* name();
    const char* username();
    String IPtoString(IPAddress);
    IPAddress stringToIP(const char*);
#ifdef S_DEBUG
    void serialDebug(HardwareSerial*);
    void printConfigFile();
    void printConfig();
#endif

  private:
    void _begin(const char*);
    void setField(char*, const char*, uint8_t);
    char* _name;
    char* _ssid;
    char* _pass;
    char* _passap;
    char* _ssidap;
    char* _user;
    char* _pwd;
    char* _file;
    bool _dhcp;
    IPAddress _ip;
    IPAddress _gw;
    IPAddress _mask;
#ifdef S_DEBUG
    void _print(const __FlashStringHelper*, const char* = NULL);
    HardwareSerial *_debug;
#endif
};