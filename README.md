# settingsManager
Handler for ESP8266 settings such as WiFi, NTP and update server.  
It's meant to be an universal, secure toolbox.  

## Features

* Handling WiFi connection
* Configuring AP
* Storing your server's login and password
* Cookie-based authentication ready
* Integrated unique key generator:
	* Device-specific key
	* Encrypted with Vigenere's cipher
	* Expiration time checked on server
* Saving and loading from SPIFFS
* Handling OTA and HTTP updates

## Examples
* basicAuth - simple authentication using HTTP server
* basicConfiguration - configuring everything on the device
* connectToWiFi - how to connect to the WiFi
* cookieAuth - cookie based authentication example using
* keyTesting - a bunch of tests for key generator
* OTAUpdateConfiguration - how to configure OTA update service
* saveToSPIFFS - saving and loading settings
* setUpAP - configure Access Point
* updateServerConfiguration - how to configure default HTTP update server

## Known issues
* Cookie auth works best with Firefox
* encryptKey takes values >=1 because of how key verification works
* Using a function without configuring it (or loading settings) may cause crash
* Requires ArduinoJson v5

### ToDo list
1. Update to ArduinoJson v6
2. Encrypt config file
3. Change key structure to [id]=(...) to make cookie auth working on all browsers
4. Rewrite token verification function to consume less memory
