/*
Arduino USB MIDI FootSwitch v3
by Hecsall (https://github.com/Hecsall)
*/

// https://www.arduino.cc/en/Reference/MIDIUSB
#include "MIDIUSB.h"
#include "variables.h"
#include "functions.h"


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
    setLayer();
}


void loop()
{
    // initBPM(); // BPM LED - commented because needs sync improvements
    setLayer(); // Set which Layer we are using

    byte active_layer = current_layer;

    switch (active_layer) {
        case 0: 
        {
            // statements
            break;
        }

        case 1:
        {
            // statements
            break;
        }
        
        case 2:
        {
            // statements
            break;
        }
    }

    // Button operations based on current_layer
    for (uint8_t i = 0; i < 5; i++)
    {
        if (current_layer < 2) // Only layers 0 and 1 are normal operational layers
        {
            poweroffLeds();
            if (button_modes[current_layer][i] == 0)
            {
                handlePushButton(i);
            }
            else if (button_modes[current_layer][i] == 1)
            {
                handleToggleButton(i);
            }
        }
        else if (current_layer == 2) // Layer 2 is the "settings" layer
        {
            showModeLeds();
            handleChangeMode(i);
        }
    }
}
