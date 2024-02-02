#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_MCP23X17.h>

#include "gpio_handling.hpp"

Adafruit_MCP23X17 mcp;

uint8_t leds[] = {PIN_BLINK_LED, PIN_RED1_LED, PIN_RED2_LED, PIN_GREEN1_LED, PIN_GREEN2_LED};
uint8_t btns[] = {PIN_BTN_1, PIN_BTN_2};

void setup_gpio()
{
  uint8_t i;
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
  Serial.print(mcp.digitalRead(PIN_BTN_1));
  Serial.print("] [");
  Serial.print(mcp.digitalRead(PIN_BTN_2));
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
