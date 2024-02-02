#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "gpio_handling.hpp"
#include "secrets.hpp"

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
long int value = 0;

const unsigned long blink_period = 5000;
const unsigned long blink_on_phase = 500;
unsigned long next_blink;
bool blink_led_state;  // true = on
bool four_led_state;
const unsigned long four_led_period = 1000;
unsigned long four_led_off;

WiFiClient wifi_client;
void setup_wifi() {
  bool res;
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  delay(3000);
  Serial.println("\n Starting");

  String ap_name = "BLINK_Board_" + WiFi.macAddress();
  res = wm.autoConnect(ap_name.c_str());
  if(!res) {
    Serial.println("Failed to connect or hit timeout");
  } else { 
    Serial.println("connected...yeey :)");
  }

  Serial.setDebugOutput(false);
}

PubSubClient mqtt_client(wifi_client);
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // turn on four LEDs
  gpio_set_led(PIN_RED1_LED, ON);
  gpio_set_led(PIN_RED2_LED, ON);
  gpio_set_led(PIN_GREEN1_LED, ON);
  gpio_set_led(PIN_GREEN2_LED, ON);
  
  four_led_state = true;
  four_led_off = millis() + four_led_period;
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish("blink", "hello world");
      // ... and resubscribe
      mqtt_client.subscribe("blink");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_gpio();
  setup_wifi();
  mqtt_client.setServer(mqtt_domain, 41420);
  mqtt_client.setCallback(callback);
}

void check_button_and_publish() {
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (gpio_get_button(PIN_BTN_1) == PRESSED && now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
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

void loop()
{
  // blink LED every 5s for 500ms
  unsigned long now = millis();
  if (!blink_led_state && now > next_blink) {
    // turn led on
    gpio_set_led(PIN_BLINK_LED, ON);
    blink_led_state = true;
  }

  // turn it off later
  if (blink_led_state && now > (next_blink + blink_on_phase)) {
    next_blink += blink_period;
    gpio_set_led(PIN_BLINK_LED, OFF);
    blink_led_state = false;
  }

  // turn off other LEDs after callback turned them on
  if (four_led_state && now > four_led_off) {
    gpio_set_led(PIN_RED1_LED, OFF);
    gpio_set_led(PIN_RED2_LED, OFF);
    gpio_set_led(PIN_GREEN1_LED, OFF);
    gpio_set_led(PIN_GREEN2_LED, OFF);
  }

  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

  check_button_and_publish();
}
