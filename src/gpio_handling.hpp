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

typedef enum led_state {
  OFF,
  ON,
} led_state_t;

typedef enum button_state {
  UNPRESSED,
  PRESSED,
} button_state_t;

typedef struct {
  uint8_t led_pin;
  led_state_t led_state;
  unsigned long timestamp;
} queued_led_change_t;

extern uint8_t leds[];
extern uint8_t btns[];

void setup_gpio(void);

void gpio_all_led_on();
void gpio_all_led_off();
void gpio_set_led(uint8_t led, led_state_t state);
int queue_led_change(queued_led_change_t statechange);
void handle_led_queue(void);

button_state_t gpio_get_button(uint8_t button);

#endif /* GPIO_HANDLING_HPP */