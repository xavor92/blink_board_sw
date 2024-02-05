#include <Arduino.h>
#include <EEPROM.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "wifi.hpp"
#include "gpio_handling.hpp"

WiFiClient wifi_client;
WiFiManagerParameter name("name", "Your Name", "", 16);
const char name_len = 17;
char eeprom_name[name_len];

void setup_wifi() {
  bool res;
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  // Load potentially saved name from EEPROM, save one space for null terminator
  EEPROM.begin(512);
  for (int i = 0; i < name_len - 1; i++) {
    eeprom_name[i] = EEPROM.read(i);
  }
  eeprom_name[name_len - 1] = 0; // null terminator

  // check if eeprom_name contains a valid string
  res = false;
  for (int i = 0; i < name_len; i++) {
    if (eeprom_name[i] == 0) {
      // end of string -> valid
      Serial.println("Found end of string at " + String(i));
      res = true;
      break;
    }
    if (eeprom_name[i] < 32 || eeprom_name[i] > 126) {
      // not a valid char -> invalid
      Serial.println("Not valid at " + String(i) + ": " + String(eeprom_name[i]));
      break;
    }
  }

  if (!res) {
    // if not, set it to an empty string
    Serial.println("Wiping String");
    for (int i = 0; i < name_len; i++) {
      eeprom_name[i] = 0;
    }
  }
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
    Serial.println("Updated custom parameter: " + String(custom_value));

    // Only write to EEPROM if the value has changed
    if (strcmp(custom_value, eeprom_name) != 0) {
      for (int i = 0; i < name_len - 1; i++) {
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
