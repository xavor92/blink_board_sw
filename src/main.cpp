#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_MCP23X17.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

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

void setup_gpio()
{
  unsigned int i;
  Wire.begin(0, 2);

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
}

void setup_wifi() {
  int attempts;
  for (unsigned int i = 0; i < (sizeof(ssids) / sizeof(*ssids)); i++)
  {
    attempts = 0;
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssids[i]);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssids[i], passwords[i]);

    while (WiFi.status() != WL_CONNECTED && attempts++ < 10) {
      delay(500);
      Serial.print(".");
      Serial.print(attempts);
    }

    if (WiFi.status() != WL_CONNECTED) {
      continue;
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  unsigned int i;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  for(i = 0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    mcp.digitalWrite(leds[i], LOW);
  }
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

    // move me
    Serial.println("Scanning...");
    Serial.print("Btn1: ");
    Serial.print(mcp.digitalRead(PIN_BTN_1));
    Serial.print("Btn2: ");
    Serial.println(mcp.digitalRead(PIN_BTN_2));


  } else if (blink_led_state && now > (next_blink + blink_on_phase)) {
    next_blink += blink_period;
    mcp.digitalWrite(PIN_BLINK_LED, HIGH);
    blink_led_state = false;
  }

  for(i = 0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    mcp.digitalWrite(leds[i], HIGH);
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
