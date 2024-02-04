#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_MCP23X17.h>

#include "gpio_handling.hpp"

Adafruit_MCP23X17 mcp;

uint8_t leds[] = {PIN_BLINK_LED, PIN_RED1_LED, PIN_RED2_LED, PIN_GREEN1_LED, PIN_GREEN2_LED};
uint8_t btns[] = {PIN_BTN_1, PIN_BTN_2};

queued_led_change_t led_queue[16];

void setup_gpio()
{
  uint8_t i;

  // clear the led queue
  memset(led_queue, 0, sizeof(led_queue));

  Wire.begin(0, 2);

  Serial.println("");
  Serial.println("");
  Serial.println("Setting up MCP23X17 and GPIOs");
  if (!mcp.begin_I2C()) {
    Serial.println("Error in begin_I2C()");
  }

  // turn off all LEDs
  gpio_all_led_off();

  // set buttons to input
  for(i = 0; i<(sizeof(btns)/sizeof(btns[0])); i++) {
    mcp.pinMode(btns[i], INPUT_PULLUP);
  }

  // Test outputs
  for(i = 0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    mcp.pinMode(leds[i], OUTPUT);
    gpio_set_led(leds[i], ON);
    delay(100);
    gpio_set_led(leds[i], OFF);
  }

  Serial.print("Button states: [");
  Serial.print(gpio_get_button(PIN_BTN_1));
  Serial.print("] [");
  Serial.print(gpio_get_button(PIN_BTN_2));
  Serial.println("]");
}

void gpio_all_led_on() {
  unsigned int i;
  for(i = 0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    gpio_set_led(leds[i], ON);
  }
}

void gpio_all_led_off() {
  unsigned int i;
  for(i = 0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    gpio_set_led(leds[i], OFF);
  }
}

void gpio_set_led(uint8_t led, led_state_t state) {
    mcp.digitalWrite(led, state == OFF ? HIGH : LOW);
}

button_state_t gpio_get_button(uint8_t button) {
  uint8_t button_value = mcp.digitalRead(button); //HIGH = OFF
  return button_value == LOW ? PRESSED : UNPRESSED;
}

int queue_led_change(queued_led_change_t statechange) {
  unsigned int i;

  for(i = 0; i < sizeof(led_queue)/sizeof(led_queue[0]); i++) {
    if (led_queue[i].timestamp == 0) {
      led_queue[i] = statechange;
      return 0;
    }
  }

  Serial.println("Queue full");
  return -1;
};

void handle_led_queue(void) {
  unsigned int i;

  for(i = 0; i < sizeof(led_queue)/sizeof(led_queue[0]); i++) {
    if (led_queue[i].timestamp != 0) {
      if (millis() > led_queue[i].timestamp) {
        gpio_set_led(led_queue[i].led_pin, led_queue[i].led_state);
        led_queue[i].timestamp = 0;
      }
    }
  }
}
