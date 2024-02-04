#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "gpio_handling.hpp"
#include "secrets.hpp"

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

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

PubSubClient mqtt_client(wifi_client);
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // do stuff with leds
  const unsigned long four_led_period = 1000;
  uint8_t four_leds[4] ={PIN_RED1_LED, PIN_RED2_LED, PIN_GREEN1_LED, PIN_GREEN2_LED};

  for(unsigned int i = 0; i < sizeof(four_leds)/sizeof(four_leds[0]); i++) {
    gpio_set_led(four_leds[i], ON);
  }

  queued_led_change_t four_led_off = {
    .led_pin = 0,
    .led_state = OFF,
    .timestamp = millis() + four_led_period
  };
  for(unsigned int i = 0; i < sizeof(four_leds)/sizeof(four_leds[0]); i++) {
    four_led_off.led_pin = four_leds[i];
    queue_led_change(four_led_off);
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
      mqtt_client.publish("blink", "hello world");
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
  static unsigned int value = 0;
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (gpio_get_button(PIN_BTN_1) == PRESSED && now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "ButtonPressed #%d", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqtt_client.publish("blink", msg);
  }
}

void delete_wifi_data() {
  WiFi.disconnect(true);
  ESP.eraseConfig();
  Serial.println(
    "aaaand its gone"
  );
}

void setup() {
  Serial.begin(115200);
  setup_gpio();
  setup_wifi();
  mqtt_client.setServer(mqtt_domain, 41420);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_reconnect();
}

void loop()
{
  static unsigned long next_blink;
  const unsigned long blink_period = 1000;
  const unsigned long blink_on_phase = 500;

  // blink LED every 5s for 500ms
  unsigned long now = millis();
  if (now > next_blink) {
    next_blink = now + blink_period;
    gpio_set_led(PIN_BLINK_LED, ON);
    queued_led_change_t blink_led_off = {
      .led_pin = PIN_BLINK_LED,
      .led_state = OFF,
      .timestamp = now + blink_on_phase
    };
    queue_led_change(blink_led_off);
  }

  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();

  check_button_and_publish();
  handle_led_queue();
}
