#include <Arduino.h>

#include "mqtt.hpp"

#include "wifi.hpp"
#include "gpio_handling.hpp"

#include "secrets.hpp"

PubSubClient mqtt_client(wifi_client);
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  const unsigned long four_led_period = 1000;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  queued_led_change_t led_off_queue_elem = {
    .led_pin = 0,
    .led_state = OFF,
    .timestamp = millis() + four_led_period
  };

  // check if string SW1=1 is contained in the payload
  if (strstr((char*)payload, "SW1=1") != NULL) {
    gpio_set_led(PIN_RED1_LED, ON);
    led_off_queue_elem.led_pin = PIN_RED1_LED;
    queue_led_change(led_off_queue_elem);

    gpio_set_led(PIN_RED2_LED, ON);
    led_off_queue_elem.led_pin = PIN_RED2_LED;
    queue_led_change(led_off_queue_elem);
  }

  if (strstr((char*)payload, "SW2=1") != NULL) {
    gpio_set_led(PIN_GREEN1_LED, ON);
    led_off_queue_elem.led_pin = PIN_GREEN1_LED;
    queue_led_change(led_off_queue_elem);

    gpio_set_led(PIN_GREEN2_LED, ON);
    led_off_queue_elem.led_pin = PIN_GREEN2_LED;
    queue_led_change(led_off_queue_elem);
  }
}

unsigned long next_mqtt_reconnect;
void mqtt_reconnect() {
  // if we end up here, we're not connected -> signal by LED
  gpio_set_led(PIN_RED2_LED, ON);
  // Try to reconnect, but only if we have not tried in the last 5 seconds
  if (millis() > next_mqtt_reconnect) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      gpio_set_led(PIN_RED2_LED, OFF);
      // Once connected, publish an announcement...
      String message = "Online: BLINK Board" + get_eeprom_name();
      mqtt_client.publish("blink", message.c_str());
      // ... and resubscribe
      mqtt_client.subscribe("blink");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      next_mqtt_reconnect = millis() + 5000;
    }
  }
}

void check_button_and_publish() {
  static unsigned int msgs_send = 0;
  static unsigned long lastMsg = 0;
  button_state_t btn1, btn2;
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    btn1 = gpio_get_button(PIN_BTN_1);
    btn2 = gpio_get_button(PIN_BTN_2); 
    if ((btn1 == PRESSED || btn2 == PRESSED)) {
      lastMsg = now;
      ++msgs_send;
      snprintf (msg, MSG_BUFFER_SIZE, "SW1=%d SW2=%d Pressed (#%d) by %s", btn1, btn2, msgs_send, get_eeprom_name().c_str());
      Serial.print("Publish message: ");
      Serial.println(msg);
      mqtt_client.publish("blink", msg);
    }
  }
}

void setup_mqtt(void) {
  mqtt_client.setServer(mqtt_domain, 41420);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_reconnect();
}

void mqtt_loop(void) {
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();
}
