#ifndef SETTINGSMANAHER_H
#define SETTINGSMANAHER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <stdio.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266HTTPUpdateServer.h>
#define FS_NO_GLOBALS
#include "FS.h"

/*
  Library which is made to hold all settings of Temperature Station
  There is no way (I know) to make it return passwords / other sensitive data
*/

/*
  Insecure, uncomment only for debugging
  Will show every action in settingsManager including passwords
*/
//#define DEBUG_INSECURE

class settingsManager {
  public:
    /* File name to store settings */
    settingsManager(const char* = NULL);
    ~settingsManager();
    void save();
    bool load();

    /* SSID, password */
    void configSTA(const char*, const char* = NULL);
    void configAP(const char*, const char* = NULL);

    /* SSID */
    void ssid(const char*);
    void ssidAP(const char*);

    /* IP addresses */
    void configIP(IPAddress, IPAddress, IPAddress);
    void configIP(const char*, const char*, const char*);
    void useDHCP(bool);
    bool useDHCP();
    void name(const char*);
    /* Username, password */
    void configUser(const char*, const char*);
    bool authenticate(const char*, const char*);

    bool beginSTA();
    bool beginAP();

    /* Direct NTP address. Pools are not tested */
    void ntpServer(const char*);

    /* Server, update service, path */
    void configUpdateServer(ESP8266WebServer*, ESP8266HTTPUpdateServer*, const char*);

    void beginOTA(uint16_t = 8266);
    bool webAuthenticate(ESP8266WebServer*);

    /* Username, password, file to remove from SPIFFS */
    bool remove(const char*, const char*, const char*);

    IPAddress localIP();
    IPAddress gatewayIP();
    IPAddress subnetMask();
    bool useNTP;
    const char* ssid();
    const char* ssidAP();
    const char* name();
    const char* username();
    const char* ntpServer();
    uint32_t lastUpdate;
    int8_t timezone;
    uint16_t readInterval;

    /* Decrypted key, canary */
    uint8_t verifyKey(String, uint32_t);

    /* Encrypted key, canary */
    uint8_t verifyEncryptedKey(String, uint32_t);

    /* Canary */
    String encryptKey(uint32_t);

    /* Encrypted key */
    String decryptKey(String);
    uint32_t tokenLifespan;

    static IPAddress stringToIP(const char* input) {
      uint8_t parts[4] = {0, 0, 0, 0};
      uint8_t part = 0;
      for (uint8_t a = 0; a < strlen(input); a++) {
        uint8_t b = input[a];
        if (b == '.') {
          part++;
          continue;
        }
        parts[part] *= 10;
        parts[part] += b - '0';
      }
      return IPAddress(parts[0], parts[1], parts[2], parts[3]);
    }

    static String IPtoString(IPAddress address) {
      String out;
      for (int z = 0; z < 4; z++) {
        out += String(address[z]);
        if (z < 3)out += ".";
      }
      return out;
    }

#ifdef DEBUG_INSECURE
    /* Pointer to Serial */
    void serialDebug(HardwareSerial*);
    void printConfigFile();
    void printConfig();
#endif

  protected:
    String generateKey(uint8_t);
    char encryptChar(char, char);
    char decryptChar(char, char);
    void setField(char*, const char*, uint8_t);
    char* _name;
    char* _ssid;
    char* _pass;
    char* _passap;
    char* _ssidap;
    char* _user;
    char* _pwd;
    char* _ntp;
    char* _file;
    bool _dhcp;
    IPAddress _ip;
    IPAddress _gw;
    IPAddress _mask;
#ifdef DEBUG_INSECURE
    void _print(const __FlashStringHelper*, const char* = NULL);
    HardwareSerial *_debug;
#endif
};
#endif