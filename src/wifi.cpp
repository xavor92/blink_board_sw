#include <Arduino.h>
#include <EEPROM.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "wifi.hpp"
#include "gpio_handling.hpp"

WiFiClient wifi_client;
WiFiManagerParameter name("name", "Your Name", "", 16);
char eeprom_name[17];

void setup_wifi() {
  bool res;
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  // Load potentially saved name from EEPROM
  EEPROM.begin(512);
  for (int i = 0; i < 16; i++) {
    eeprom_name[i] = EEPROM.read(i);
  }
  for (int i = 0; i < 16; i++) {
    if (eeprom_name[i] < 32 || eeprom_name[i] > 126) {
      eeprom_name[i] = 0;
    }
  }
  eeprom_name[16] = '\0';
  name.setValue(eeprom_name, 16);
  Serial.println("Loaded custom parameter: " + String(eeprom_name));

  // Add the custom parameter to WiFiManager
  wm.addParameter(&name);

  delay(3000);
  Serial.println("\n Starting");

  String ap_name;
  if (eeprom_name[0] != 0) {
    ap_name = "BLINK_Board_" + String(eeprom_name);
  } else {
    ap_name = "BLINK_Board_" + WiFi.macAddress();
  }

  gpio_set_led(PIN_RED1_LED, ON);
  if(gpio_get_button(PIN_BTN_1) == PRESSED && gpio_get_button(PIN_BTN_2) == PRESSED) {
    res = wm.startConfigPortal(ap_name.c_str());
  } else {
    res = wm.autoConnect(ap_name.c_str());
  }
  if(!res) {
    Serial.println("Failed to connect or hit timeout");
  } else { 
    Serial.println("connected...yeey :)");

    // Get the custom text value
    const char* custom_value = name.getValue();

    // Only write to EEPROM if the value has changed
    if (strcmp(custom_value, eeprom_name) != 0) {
      for (int i = 0; i < 16; i++) {
        EEPROM.write(i, custom_value[i]);
      }
      EEPROM.commit(); // Make sure to commit changes
    }

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

String get_eeprom_name() {
  return String(eeprom_name);
}
