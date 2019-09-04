#include <settingsManager.h>

/*
  This sketch is used to test library crypto capacibilities.
  All printable ASCII input is supported.
  Any whitespace characters are prohibited and will return an error.
*/

/* Basic setup only for purpose of generating test keys */
const char* _name = "Testbench";
const char* _user = "username";
const char* _password = "password";

/*
  Kind of canary. It's intended to use with time, but any value
  that changes is fine.
  In case of using custom generated number set tokenLifespan to 0
*/
const uint32_t validityBase = 100;
const String _chipID = String(ESP.getChipId(), HEX);
String _expiry;

settingsManager settings;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("\r\nEncryption test");

  /* Configuring settings */
#ifdef DEBUG_INSECURE
  settings.serialDebug(&Serial);
#endif
  settings.name(_name);
  settings.configUser(_user, _password);
  settings.tokenLifespan = 10;
  _expiry = String(settings.tokenLifespan);

  /* First, let's check if it en- and decrypts correctly */

  String enc = settings.encryptKey(validityBase);
  Serial.print("Generated key:\t\t");
  Serial.println(enc);

  String dec = settings.decryptKey(enc);
  Serial.print("Valid decrypted key:\t");
  Serial.println(dec);

  Serial.print("Encrypting key:\t\t");
  Serial.println(settings.verifyEncryptedKey(enc, validityBase) == 0 ? "Pass" : "Fail");

  Serial.print("Decrypting key:\t\t");
  performTest(&settings, dec, validityBase, 0);

  String test = dec;
  test[test.length() - 2] = ' ';
  Serial.print("White characters:\t");
  performTest(&settings, test, validityBase, 1);

  Serial.print("No timestamp:\t\t");
  test = _chipID + '=' + _name + String("++") + _expiry;
  performTest(&settings, test, validityBase, 2);

  test = _chipID + '=' + _name + '+' + String(validityBase) + '+' + _expiry;
  test[strlen(_name) + 8] = 'a';
  Serial.print("Conversion error:\t");
  performTest(&settings, test, validityBase, 2);

  test = _chipID + '=' + _name + '+' + String(validityBase) + '+' + ' + ' + _expiry;
  test += 'a';
  Serial.print("Conversion error 2:\t");
  performTest(&settings, test, validityBase, 2);

  test = _chipID + '=' + _name + '+' + String(validityBase) + String("+a") + _expiry;
  test += 'a';
  Serial.print("Conversion error 3:\t");
  performTest(&settings, test, validityBase, 2);

  test = _chipID + '=' + _name + '+' + String(validityBase) + '+';
  Serial.print("No token lifespan:\t");
  performTest(&settings, test, validityBase, 2);

  test = _chipID + '=' + _name + '+' + String(validityBase) + '+' + _expiry;
  test[strlen(_name) - 1] = '!';
  Serial.print("Invalid name:\t\t");
  performTest(&settings, test, validityBase, 3);

  test = _chipID + '=' + String(_name).substring(1) + '+' + String(validityBase) + '+' + _expiry;
  Serial.print("Invalid name length:\t");
  performTest(&settings, test, validityBase, 3);

  test = _chipID + '=' + '+' + String(validityBase) + '+' + _expiry;
  Serial.print("No name:\t\t");
  performTest(&settings, test, validityBase, 3);

  test = _chipID.substring(1) + '=' + _name + '+' + String(validityBase) + '+' + _expiry;
  Serial.print("Invalid chip id len:\t");
  performTest(&settings, test, validityBase, 4);

  test = String("DEADBE=") + _name + '+' + String(validityBase) + '+' + _expiry;
  Serial.print("Invalid chip id:\t");
  performTest(&settings, test, validityBase, 4);

  test = 'z' + _chipID.substring(1) + '=' + _name + '+' + String(validityBase) + String("+z") + '+' + _expiry;
  Serial.print("Invalid chip id 2:\t");
  performTest(&settings, test, validityBase, 4);

  test = _chipID.substring(0, _chipID.length() - 2) + '=' + _name + '+' + String(validityBase) + String("+A+") + _expiry;
  Serial.print("Invalid chip id 3:\t");
  performTest(&settings, test, validityBase, 4);

  test = String("=") + _name + '+' + String(validityBase) + '+' + _expiry;
  Serial.print("No chip id:\t\t");
  performTest(&settings, test, validityBase, 4);

  test = dec;
  Serial.print("Invalid time:\t\t");
  performTest(&settings, test, validityBase + (settings.tokenLifespan * 2), 5);

  Serial.print("Invalid time 2:\t\t");
  performTest(&settings, test, validityBase - (settings.tokenLifespan * 2), 5);
}

/* Try to use pointers whenever you can. It's really memory - consuming */
bool performTest(settingsManager * set, String test, uint32_t t, uint8_t code) {
  uint8_t ret = set->verifyKey(test, t);
  if (ret == code) {
    Serial.println("Pass (" + String(ret) + ")");
    return true;
  }
  Serial.println("Fail (" + String(ret) + ")");
  return false;
}

void loop() {}