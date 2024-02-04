#include <Arduino.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "wifi.hpp"
#include "gpio_handling.hpp"

WiFiClient wifi_client;

void setup_wifi() {
  bool res;
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  delay(3000);
  Serial.println("\n Starting");

  String ap_name = "BLINK_Board_" + WiFi.macAddress();
  gpio_set_led(PIN_RED1_LED, ON);
  res = wm.autoConnect(ap_name.c_str());
  if(!res) {
    Serial.println("Failed to connect or hit timeout");
  } else { 
    Serial.println("connected...yeey :)");
    gpio_set_led(PIN_RED1_LED, OFF);
    gpio_set_led(PIN_GREEN1_LED, ON);
    delay(500);
    gpio_set_led(PIN_GREEN1_LED, OFF);
  }

  Serial.setDebugOutput(false);
}


void delete_wifi_data() {
  WiFi.disconnect(true);
  ESP.eraseConfig();
  Serial.println(
    "aaaand its gone"
  );
}