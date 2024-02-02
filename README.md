# BLINK Board SW

Software for the [BLINK Board](https://github.com/xavor92/blink_board_hw)

## Requirements

Use VSCode with [PlatformIO](https://platformio.org/) installed.

Should work OOTB.

## secrets.h

See the template. Setup your own MQTT server or check with your team for credentials ;-)

## Programming mode

Keep Boot pressed while power cycling/resetting the board with reset pin.
Refer to hardware repo for button positions.

## Functions

### Bootup LEDs

1. LED Test (All LEDs should flash once)
2. RED1 turns on -> WiFi connection attempt
3. RED1 turns off, GREEN1 turns on -> Wifi connected
3. RED2 will be on if Wifi is not conencted
