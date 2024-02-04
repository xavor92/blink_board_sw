#ifndef MQTT_HPP
#define MQTT_HPP

#include <Arduino.h>
#include <PubSubClient.h>

extern PubSubClient mqtt_client;

void setup_mqtt(void);
void mqtt_loop(void);
void check_button_and_publish(void);
String get_eeprom_name(void);

#endif /* MQTT_HPP */