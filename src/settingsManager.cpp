#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServerSecure.h>
#include "FS.h"
#include "settingsManager.h"

settingsManager::settingsManager(const char* _f) {
  this->_name = new char[32];
  this->_ssid = new char[32];
  this->_pass = new char[32];
  this->_ssidap = new char[32];
  this->_passap = new char[32];
  this->_user = new char[16];
  this->_pwd = new char[16];
  this->_ntp = new char[32];
  this->_file = new char[32];
  this->setField(this->_file, _f, 32);
  this->_dhcp = true;
  this->useNTP = false;
  this->lastUpdate = 0;
  this->timezone = 0;
  this->readInterval = 5;
#ifdef DEBUG_INSECURE
  this->_debug = NULL;
#endif
}

settingsManager::~settingsManager() {
  delete[] _name;
  delete[] _ssid;
  delete[] _pass;
  delete[] _ssidap;
  delete[] _passap;
  delete[] _user;
  delete[] _pwd;
  delete[] _ntp;
  delete[] _file;
#ifdef DEBUG_INSECURE
  this->_print(F("End"));
#endif
}

void settingsManager::ntpServer(const char* _ntp) {
  setField(this->_ntp, _ntp, 32);
#ifdef DEBUG_INSECURE
  this->_print(F("Set NTP Server: "), this->_ntp);
#endif
  return void();
}

void settingsManager::save() {
  SPIFFS.remove(this->_file);
  File _toSave = SPIFFS.open(this->_file, "w");
  _toSave.print(F("{\"SN\":\""));
  _toSave.print(this->_name);
  _toSave.print(F("\",\"SL\":\""));
  _toSave.print(this->_user);
  _toSave.print(F("\",\"SPL\":\""));
  _toSave.print(this->_pwd);
  _toSave.print(F("\",\"OS\":\""));
  _toSave.print(this->_ssid);
  _toSave.print(F("\",\"OP\":\""));
  _toSave.print(this->_pass);
  _toSave.print(F("\",\"SS\":\""));
  _toSave.print(this->_ssidap);
  _toSave.print(F("\",\"SPA\":\""));
  _toSave.print(this->_passap);
  _toSave.print(F("\",\"OI\":\""));
  _toSave.print(this->_ip);
  _toSave.print(F("\",\"OG\":\""));
  _toSave.print(this->_gw);
  _toSave.print(F("\",\"OM\":\""));
  _toSave.print(this->_mask);
  _toSave.print(F("\",\"OD\":"));
  _toSave.print(this->_dhcp ? F("true") : F("false"));
  _toSave.print(F(",\"UN\":"));
  _toSave.print(this->useNTP ? F("true") : F("false"));
  _toSave.print(F(",\"NS\":\""));
  _toSave.print(this->_ntp);
  _toSave.print(F("\",\"LU\":"));
  _toSave.print(this->lastUpdate);
  _toSave.print(F(",\"TZ\":"));
  _toSave.print(this->timezone);
  _toSave.print(F(",\"RI\":"));
  _toSave.print(this->readInterval);
  _toSave.print(F("}"));
  _toSave.flush();
  _toSave.close();

#ifdef DEBUG_INSECURE
  this->_print(F("Saved settings"));
#endif
  return void();
}

void settingsManager::beginOTA(uint16_t _p) {
  ArduinoOTA.setHostname(this->_name);
  ArduinoOTA.setPassword(this->_pwd);
  ArduinoOTA.setPort(_p);
#ifdef DEBUG_INSECURE
  this->_print(F("ArduinoOTA Started"));
  ArduinoOTA.onStart([this]() {
    this->_print(F("Begin flash update"));
  });
  ArduinoOTA.onEnd([this]() {
    SPIFFS.end();
    this->_debug->println();
    this->_print(F("Update End"));
  });
  ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
    this->_debug->printf("Settings:\tUpdate Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([this](ota_error_t error) {
    this->_debug->printf("Settings:\tError[%u]: ", error);
    if (error == OTA_AUTH_ERROR) this->_debug->println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) this->_debug->println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) this->_debug->println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) this->_debug->println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) this->_debug->println(F("End Failed"));
    SPIFFS.begin();
  });
#endif
  ArduinoOTA.begin();
}

bool settingsManager::load() {
#ifdef DEBUG_INSECURE
  this->_print(F("Config file: "), this->_file);
#endif
  if (!SPIFFS.exists(this->_file)) {
#ifdef DEBUG_INSECURE
    this->_print(F("No file to load"));
#endif
    return false;
  }
#ifdef DEBUG_INSECURE
  this->_print(F("Found file"));
#endif
  File _toSave = SPIFFS.open(this->_file, "r");
  //Longest config i could create
  StaticJsonBuffer<675> _ld;
  JsonObject& _data = _ld.parseObject(_toSave);
  _toSave.close();
  if (!_data.success()) {
#ifdef DEBUG_INSECURE
    this->_print(F("Load error"));
#endif
    return false;
  }

  this->setField(this->_name, _data["SN"], 32);
  this->setField(this->_user, _data["SL"], 16);
  this->setField(this->_pwd, _data["SPL"], 16);
  this->setField(this->_ssid, _data["OS"], 32);
  this->setField(this->_pass, _data["OP"], 32);
  this->setField(this->_ssidap, _data["SS"], 32);
  this->setField(this->_passap, _data["SPA"], 32);
  this->_ip = stringToIP(_data["OI"]);
  this->_gw = stringToIP(_data["OG"]);
  this->_mask = stringToIP(_data["OM"]);
  this->_dhcp = _data["OD"].as<bool>();
  this->useNTP = _data["UN"].as<bool>();
  this->setField(this->_ntp, _data["NS"], 32);
  this->lastUpdate = _data["LU"].as<uint32_t>();
  this->timezone = _data["TZ"].as<int8_t>();
  this->readInterval = _data["RI"].as<uint16_t>();
#ifdef DEBUG_INSECURE
  this->_print(F("Loaded successfully"));
#endif
  return true;
}

#ifdef DEBUG_INSECURE
void settingsManager::_print(const __FlashStringHelper *_msg, const char* _arg) {
  if (this->_debug == NULL) return void();
  this->_debug->print(F("Settings:\t"));
  this->_debug->print(_msg);
  this->_debug->println(_arg);
  return void();
}
void settingsManager::serialDebug(HardwareSerial *_d) {
  this->_debug = _d;
  this->_print(F("Debug begin"));
  return void();
}
void settingsManager::printConfigFile() {
  File _f = SPIFFS.open(this->_file, "r");
  while (_f.available()) this->_debug->write(_f.read());
  _f.close();
  return void();
}
void settingsManager::printConfig() {
  this->_debug->print(F("{\"SN\":\""));
  this->_debug->print(this->_name);
  this->_debug->print(F("\",\"SL\":\""));
  this->_debug->print(this->_user);
  this->_debug->print(F("\",\"SPL\":\""));
  this->_debug->print(this->_pwd);
  this->_debug->print(F("\",\"OS\":\""));
  this->_debug->print(this->_ssid);
  this->_debug->print(F("\",\"OP\":\""));
  this->_debug->print(this->_pass);
  this->_debug->print(F("\",\"SS\":\""));
  this->_debug->print(this->_ssidap);
  this->_debug->print(F("\",\"SPA\":\""));
  this->_debug->print(this->_passap);
  this->_debug->print(F("\",\"OI\":\""));
  this->_debug->print(this->_ip);
  this->_debug->print(F("\",\"OG\":\""));
  this->_debug->print(this->_gw);
  this->_debug->print(F("\",\"OM\":\""));
  this->_debug->print(this->_mask);
  this->_debug->print(F("\",\"OD\":"));
  this->_debug->print(this->_dhcp ? F("true") : F("false"));
  this->_debug->print(F(",\"UN\":"));
  this->_debug->print(this->useNTP ? F("true") : F("false"));
  this->_debug->print(F(",\"NS\":\""));
  this->_debug->print(this->_ntp);
  this->_debug->print(F("\",\"LU\":"));
  this->_debug->print(this->lastUpdate);
  this->_debug->print(F(",\"TZ\":"));
  this->_debug->print(this->timezone);
  this->_debug->print(F("\",\"RI\":"));
  this->_debug->print(this->readInterval);
  this->_debug->print(F("}\r\n"));
  return void();
}
#endif

bool settingsManager::remove(const char* _l, const char* _p, const char* _f) {
  if (!this->authenticate(_l, _p))
    return false;
  SPIFFS.remove(_f);
#ifdef DEBUG_INSECURE
  this->_print(F("File removed"));
#endif

  return true;
}

void settingsManager::configUpdateServer(ESP8266WebServer *_conf, ESP8266HTTPUpdateServer *_upd, const char* _url) {
  _upd->setup(_conf, _url, this->_user, this->_pwd);
#ifdef DEBUG_INSECURE
  this->_debug->println(F("Settings:\tConfigured update server "));
  this->_debug->print(F("Settings:\tUpdate on "));
  this->_debug->print(_url);
  this->_debug->print(F(" as "));
  this->_debug->print(this->_user);
  this->_debug->print(F("@"));
  this->_debug->println(this->_pwd);
#endif
  return void();
}

bool settingsManager::webAuthenticate(ESP8266WebServer* _s) {
  return _s->authenticate(this->_user, this->_pwd);
}

bool settingsManager::beginWiFi() {
  WiFi.mode(WIFI_STA);
  if (this->_dhcp == false)
    WiFi.config(this->_ip, this->_gw, this->_mask);
  WiFi.setOutputPower(0);
  WiFi.begin(this->_ssid, this->_pass);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) //wait 10 seconds
    delay(500);

  if (WiFi.status() != WL_CONNECTED) {
#ifdef DEBUG_INSECURE
    this->_print(F("Unable to connect to WiFi"));
#endif
    return false;
  }
  //if (this->_dhcp == true)
  //  this->configIP(this->_ip, this->_gw, this->_mask);
#ifdef DEBUG_INSECURE
  this->_print(F("Connected to "), this->_ssid);
#endif
  return true;
}

bool settingsManager::beginAP() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP);
  WiFi.setOutputPower(0);
  delay(10);
  bool _r;
  if (strcmp(this->_passap, "") == 0)
    _r = WiFi.softAP(this->_ssidap);
  else
    _r = WiFi.softAP(this->_ssidap, this->_passap, false);
#ifdef DEBUG_INSECURE
  this->_print(F("AccessPoint Started: "), _r ? "Yes" : "No");
#endif
  return _r;
}

void settingsManager::configUser(const char* _u, const char* _p) {
  this->setField(this->_user, _u, 16);
  this->setField(this->_pwd, _p, 16);
#ifdef DEBUG_INSECURE
  this->_debug->print(F("Settings:\tConfigured user "));
  this->_debug->print(this->_user);
  this->_debug->print(F("@"));
  this->_debug->println(this->_pwd);
#endif
  return void();
}

bool settingsManager::authenticate(const char* _username, const char* _password) {
  if (strcmp(this->_user, _username) == 0)
    if (strcmp(this->_pwd, _password) == 0) {
#ifdef DEBUG_INSECURE
      this->_print(F("Authenticated user "), this->_user);
#endif
      return true;
    }
#ifdef DEBUG_INSECURE
  this->_debug->print(F("Settings:\tInvalid user or password "));
  this->_debug->print(this->_user);
  this->_debug->print(F("@"));
  this->_debug->println(this->_pwd);
#endif
  return false;
}

void settingsManager::configSTA(const char* _s, const char* _p) {
  this->setField(this->_ssid, _s, 32);
  this->setField(this->_pass, _p, 32);
#ifdef DEBUG_INSECURE
  this->_debug->print(F("Settings:\tConfigured WiFi "));
  this->_debug->print(this->_ssid);
  this->_debug->print(F("@"));
  this->_debug->println(this->_pass);
#endif
  return void();
}

void settingsManager::configAP(const char* _ssidap, const char* _passap) {
  this->setField(this->_ssidap, _ssidap, 32);
  this->setField(this->_passap, _passap, 32);
#ifdef DEBUG_INSECURE
  this->_debug->print(F("Settings:\tConfigured AccessPoint "));
  this->_debug->print(this->_ssidap);
  this->_debug->print(F("@"));
  this->_debug->println(this->_passap);
#endif
  return void();
}

void settingsManager::configIP(IPAddress _ip, IPAddress _gw, IPAddress _mask) {
  this->_ip = _ip;
  this->_gw = _gw;
  this->_mask = _mask;
#ifdef DEBUG_INSECURE
  this->_debug->print(F("Settings:\tIP Config:\r\n\t\tIP Address:\t"));
  this->_debug->print(this->_ip);
  this->_debug->print(F("\r\n\t\tGateway:\t"));
  this->_debug->print(this->_gw);
  this->_debug->print(F("\r\n\t\tSubnet:\t\t"));
  this->_debug->println(this->_mask);
#endif
  return void();
}

void settingsManager::configIP(const char* _ip, const char* _gw, const char* _mask) {
  this->configIP(stringToIP(_ip), stringToIP(_gw), stringToIP(_mask));
  return void();
}

void settingsManager::useDHCP(bool _dhcp) {
  this->_dhcp = _dhcp;
#ifdef DEBUG_INSECURE
  this->_print(F("DHCP has been "), this->_dhcp ? "enabled" : "disabled");
#endif
  return void();
}

void settingsManager::ssid(const char* _ssid) {
  setField(this->_ssid, _ssid, 32);
#ifdef DEBUG_INSECURE
  this->_print(F("SSID changed to "), this->_ssid);
#endif
  return void();
}

void settingsManager::ssidAP(const char* _ssidap) {
  setField(this->_ssidap, _ssidap, 32);
#ifdef DEBUG_INSECURE
  this->_print(F("AP SSID changed to "), this->_ssid);
#endif
  return void();
}

const char* settingsManager::ssid() {
  return this->_ssid;
}

const char* settingsManager::ssidAP() {
  return this->_ssidap;
}

const char* settingsManager::username() {
  return this->_user;
}

const char* settingsManager::name() {
  return this->_name;
}

const char* settingsManager::ntpServer() {
  return this->_ntp;
}

bool settingsManager::useDHCP() {
  return this->_dhcp;
}

IPAddress settingsManager::localIP() {
  return this->_ip;
}

IPAddress settingsManager::gatewayIP() {
  return this->_gw;
}

IPAddress settingsManager::subnetMask() {
  return this->_mask;
}

void settingsManager::name(const char* _name) {
  this->setField(this->_name, _name, 32);
#ifdef DEBUG_INSECURE
  this->_print(F("Changed name to "), this->_name);
#endif
  return void();
}

void settingsManager::setField(char* _dest, const char* _src, uint8_t _size) {
  if (_src == NULL) {
    memset(_dest, 0, _size);
#ifdef DEBUG_INSECURE
    this->_print(F("Empty"));
#endif
  } else {
    uint8_t _len = strlen(_src);
#ifdef DEBUG_INSECURE
    this->_print(F("Length: "), String(_len).c_str());
#endif
    if (_len != 0) {
      if (_len > (_size - 1)) _len = (_size - 1);
      memcpy(_dest, _src, _len);
      _dest[_len] = '\x00';
    }
  }
  return void();
}

IPAddress settingsManager::stringToIP(const char* input) {
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

String settingsManager::IPtoString(IPAddress address) {
  String out;
  for (int z = 0; z < 4; z++) {
    out += String(address[z]);
    if (z < 3)out += ".";
  }
  return out;
}
