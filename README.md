# BLINK Board SW

Software for the [BLINK Board](https://github.com/xavor92/blink_board_hw)

## Requirements

Use VSCode with [PlatformIO](https://platformio.org/) installed.

### secrets.hpp

See the template. Setup your own MQTT server or check with your team for credentials ;-)

# Board usage

## Debugging

UART should be labeled on V1.2 of the board, otherwise it's the three-pin header.

Top to Bottom
* TXD
* RXD
* GND

## Flashing

* Set board into programming mode
  keep *Boot* pressed while power cycling/resetting the board with *Reset*.
* Use flash method from platformIO or esptool.py

# Setup

When the board can not connect to a WiFi on boot, it will start a WiFi with no password and a captive portal at 192.168.4.1 to allow setting up WiFi and the name of the Board owner. The same captive portal can be started by keeping SW1 and SW2 pressed at boot.

## Functions

* Press SW1 to turn on RED LEDs on all boards
* Press SW2 to turn on GREEN LEDs on all boards

### Bootup LEDs

1. LED Test (All LEDs should flash once)
2. RED1 turns on -> WiFi connection attempt
3. RED1 turns off, GREEN1 turns on -> Wifi connected
3. RED2 will be on if MQTT fails to connect/is reconnecting
