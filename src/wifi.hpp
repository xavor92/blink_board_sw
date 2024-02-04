#ifndef WIFI_HPP
#define WIFI_HPP

#include <ESP8266WiFi.h>

extern WiFiClient wifi_client;

void setup_wifi(void);
void delete_wifi_data();

#endif /* WIFI_HPP */