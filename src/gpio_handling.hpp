#ifndef GPIO_HANDLING_HPP
#define GPIO_HANDLING_HPP

#include <Arduino.h>

#define PIN_BLINK_LED    8
#define PIN_RED1_LED     9
#define PIN_RED2_LED    10
#define PIN_GREEN1_LED  11
#define PIN_GREEN2_LED  12
#define PIN_BTN_1        0
#define PIN_BTN_2        1

extern uint8_t leds[];
extern uint8_t btns[];

typedef enum led_state {
  OFF,
  ON,
} led_state_t;

typedef enum button_state {
  PRESSED,
  UNPRESSED
} button_state_t;

void setup_gpio(void);

void gpio_all_led_on();
void gpio_all_led_off();
void gpio_set_led(uint8_t led, led_state_t state);

button_state_t gpio_get_button(uint8_t button);

#endif /* GPIO_HANDLING_HPP */