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
  Serial.begin(115200);
  Serial.println("Encryption test");

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
  Serial.println(performTest(&settings, dec, validityBase, 0) ? "Pass" : "Fail");

  String test = dec;
  test[test.length() - 2] = ' ';
  Serial.print("White characters:\t");
  Serial.println(performTest(&settings, test, validityBase, 1) ? "Pass" : "Fail");

  Serial.print("No timestamp:\t\t");
  test = _name + String("++") + _chipID + String("+") + _expiry;
  Serial.println(performTest(&settings, test, validityBase, 2) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+") + _chipID + String("+") + _expiry;
  test[strlen(_name) + 2] = 'a';
  Serial.print("Conversion error:\t");
  Serial.println(performTest(&settings, test, validityBase, 2) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+") + _chipID + String("+") + _expiry;
  test += 'a';
  Serial.print("Conversion error 2:\t");
  Serial.println(performTest(&settings, test, validityBase, 2) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+") + _chipID + String("+a") + _expiry;
  test += 'a';
  Serial.print("Conversion error 3:\t");
  Serial.println(performTest(&settings, test, validityBase, 2) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+") + _chipID + String("+");
  Serial.print("No token lifespan:\t");
  Serial.println(performTest(&settings, test, validityBase, 2) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+") + _chipID + String("+") + _expiry;
  test[strlen(_name) - 1] = '!';
  Serial.print("Invalid name:\t\t");
  Serial.println(performTest(&settings, test, validityBase, 3) ? "Pass" : "Fail");

  test = String(_name).substring(1) + String("+") + String(validityBase) + String("+") + _chipID + String("+") + _expiry;
  Serial.print("Invalid name length:\t");
  Serial.println(performTest(&settings, test, validityBase, 3) ? "Pass" : "Fail");

  test = String("+") + String(validityBase) + String("+") + _chipID + String("+") + _expiry;
  Serial.print("No name:\t\t");
  Serial.println(performTest(&settings, test, validityBase, 3) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+") + _chipID.substring(1) + String("+") + _expiry;
  Serial.print("Invalid chip id length:\t");
  Serial.println(performTest(&settings, test, validityBase, 4) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+FFFFF+") + _expiry;
  Serial.print("Invalid chip id:\t");
  Serial.println(performTest(&settings, test, validityBase, 4) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+z") + _chipID.substring(1) + String("+") + _expiry;
  Serial.print("Invalid chip id 2:\t");
  Serial.println(performTest(&settings, test, validityBase, 4) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("+") + _chipID.substring(0, _chipID.length() - 2) + String("A+") + _expiry;
  Serial.print("Invalid chip id 3:\t");
  Serial.println(performTest(&settings, test, validityBase, 4) ? "Pass" : "Fail");

  test = _name + String("+") + String(validityBase) + String("++") + _expiry;
  Serial.print("No chip id:\t\t");
  Serial.println(performTest(&settings, test, validityBase, 4) ? "Pass" : "Fail");

  test = dec;
  Serial.print("Invalid time:\t\t");
  Serial.println(performTest(&settings, test, validityBase + (settings.tokenLifespan * 2), 5) ? "Pass" : "Fail");

  Serial.print("Invalid time 2:\t\t");
  Serial.println(performTest(&settings, test, validityBase - (settings.tokenLifespan * 2), 5) ? "Pass" : "Fail");
}

/* Try to use pointers whenever you can. It's really memory-consuming */
bool performTest(settingsManager* set, String test, uint32_t t, uint8_t code) {
  return (set->verifyKey(test, t) == code);
}

void loop() {}