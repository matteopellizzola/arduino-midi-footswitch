/*
Arduino USB MIDI FootSwitch v3
by Hecsall (https://github.com/Hecsall)
*/

// https://www.arduino.cc/en/Reference/MIDIUSB
#include "MIDIUSB.h"
#include "variables.h"


void setup()
{
    Serial.begin(115200);
    // Button Pins
    for (uint8_t i = 0; i < 5; i++)
    {
        pinMode(button_pins[i], INPUT_PULLUP);
    }
    // LED Pins
    for (uint8_t i = 0; i < 5; i++)
    {
        pinMode(led_pins[i], OUTPUT);
    }
    // Switch Pins
    for (uint8_t i = 0; i < 2; i++)
    {
        pinMode(switch_pins[i], INPUT_PULLUP);
    }
    // Set currently active Layer
    initLayer();
}


void loop()
{
    // initBPM(); // BPM LED - commented because needs sync improvements
    byte active_layer = checkLayer(); // Set which Layer we are using
    switch (active_layer) {

        case 0: 
        {
            handle_buttons(0);
            break;
        }

        case 1:
        {
            handle_buttons(1);
            break;
        }
        
        case 2:
        {
            handle_settings_buttons(2);
            break;
        }

    }
}
