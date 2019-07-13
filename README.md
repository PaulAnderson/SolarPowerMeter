# SolarPowerMeter

Many home solar panel system do not show the amount of power they are generating in a convenient location. It's either on a display on the inverter or accessible using an app. I wanted to see how much power my system was generating at all times, so I created a wireless display for my inverter.

This is an Arduino sketch to get the current power from a Fronius inverter using the [Solar API](https://www.fronius.com/en/photovoltaics/products/all-products/system-monitoring/open-interfaces/fronius-solar-api-json-), and display the power in watts on an LED Display.

![](https://raw.githubusercontent.com/PaulAnderson/SolarPowerMeter/master/images/solarmetercomplete.jpg)

![](https://raw.githubusercontent.com/PaulAnderson/SolarPowerMeter/master/images/solarmeterconnections.jpg)

It uses a low cost TMS1637-based display module which uses a simple serial interface. Any ESP8266 board can be used.

The last known WiFi settings are used by default. If the connection fails, WiFiManager sets up a temporary access point with SSID Solar-Meter and password password, with a captive portal that allows you to set up a new connection.

Libraries used:
ArduinoJson to get the value from the json data returned from the solar API.
ESP8266WiFi, ESP8266WiFiMulti and WiFiManager for WiFi connectivity.
