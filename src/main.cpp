#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_MCP23X17.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "secrets.h"

#define PIN_BLINK_LED    8
#define PIN_RED1_LED     9
#define PIN_RED2_LED    10
#define PIN_GREEN1_LED  11
#define PIN_GREEN2_LED  12
#define PIN_BTN_1        0
#define PIN_BTN_2        1

int leds[] = {PIN_BLINK_LED, PIN_RED1_LED, PIN_RED2_LED, PIN_GREEN1_LED, PIN_GREEN2_LED};
int btns[] = {PIN_BTN_1, PIN_BTN_2};
Adafruit_MCP23X17 mcp;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
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

void setup_gpio()
{
  unsigned int i;
  Wire.begin(0, 2);

  Serial.println("");
  Serial.println("");
  Serial.println("Setting up MCP23X17 and GPIOs");
  if (!mcp.begin_I2C()) {
    Serial.print("Error in ");
    Serial.println(__func__);
  }

  // turn off all LEDs
  for(i = 0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    mcp.pinMode(leds[i], OUTPUT);
    mcp.digitalWrite(leds[i], HIGH);
  }

  for(i = 0; i<(sizeof(btns)/sizeof(btns[0])); i++) {
    mcp.pinMode(btns[i], INPUT_PULLUP);
  }

  // Test outputs
  for(i = 0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    mcp.digitalWrite(leds[i], LOW);
    delay(100);
    mcp.digitalWrite(leds[i], HIGH);
  }

  Serial.print("Button states: [");
  Serial.print(mcp.digitalRead(PIN_BTN_1));
  Serial.print("] [");
  Serial.print(mcp.digitalRead(PIN_BTN_2));
  Serial.println("]");
}

void setup_wifi() {
  bool res;
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;
  delay(3000);
  Serial.println("\n Starting");

  res = wm.autoConnect("BLINK_Board");
  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }

  Serial.setDebugOutput(false);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // turn on four LEDs
  mcp.digitalWrite(PIN_RED1_LED, LOW);
  mcp.digitalWrite(PIN_RED2_LED, LOW);
  mcp.digitalWrite(PIN_GREEN1_LED, LOW);
  mcp.digitalWrite(PIN_GREEN2_LED, LOW);
  
  four_led_state = true;
  four_led_off = millis() + four_led_period;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("blink", "hello world");
      // ... and resubscribe
      client.subscribe("blink");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
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
  client.setServer(mqtt_domain, 41420);
  client.setCallback(callback);
}

void loop()
{
  unsigned int i;

  // blink LED every 5s for 500ms
  unsigned long now = millis();
  if (!blink_led_state && now > next_blink) {
    // turn led on
    mcp.digitalWrite(PIN_BLINK_LED, LOW);
    blink_led_state = true;
  }

  // turn it off later
  if (blink_led_state && now > (next_blink + blink_on_phase)) {
    next_blink += blink_period;
    mcp.digitalWrite(PIN_BLINK_LED, HIGH);
    blink_led_state = false;
  }

  // turn off other LEDs after callback turned them on
  if (four_led_state && now > four_led_off) {
    mcp.digitalWrite(PIN_RED1_LED, HIGH);
    mcp.digitalWrite(PIN_RED2_LED, HIGH);
    mcp.digitalWrite(PIN_GREEN1_LED, HIGH);
    mcp.digitalWrite(PIN_GREEN2_LED, HIGH);
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  now = millis();
  if (!mcp.digitalRead(PIN_BTN_1) && now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("blink", msg);
  }
}
