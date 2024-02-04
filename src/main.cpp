#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "gpio_handling.hpp"
#include "mqtt.hpp"
#include "wifi.hpp"

void setup() {
  Serial.begin(115200);
  setup_gpio();
  setup_wifi();
  setup_mqtt();
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

  mqtt_loop();

  check_button_and_publish();
  handle_led_queue();
}
