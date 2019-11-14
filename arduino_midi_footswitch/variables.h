// Arduino Pins
const byte NUM_BUTTONS = 5;
const byte NUM_LEDS = 5;
const byte button_pins[5] = {2, 3, 4, 5, 6};   // Pins where buttons are connected
const byte switch_pins[2] = {8, 9};            // Pins where the switch is connected
const byte led_pins[5] = {10, 16, 14, 15, 18}; // Pins where LEDs are connected

// Layer: this is the current selected layer (changed by the switch)
byte current_layer; // 0 = switch up, 1 = switch off, 2 = switch down

/*
Layer button order
,-----------------,
|  1     |     2  |
|-----------------|
|  3  |  4  |  5  |
`-----------------`
CC values are the numbers in the decimal column of this table
https://www.midi.org/specifications-old/item/table-3-control-change-messages-data-bytes-2
*/
const int button_layers[2][5] = {
    {49, 50, 51, 80, 81}, // Layer 0 (Switch Up)
    {16, 17, 18, 19, 48} // Layer 1 (Switch Off)
};

// Button modes: 
// 0 = Push button
// 1 = Switch button
byte button_modes[2][5] = {
    {0, 0, 0, 0, 0}, // Layer 0 (Switch Up)
    {0, 0, 0, 0, 0} // Layer 1 (Switch Off)
};

// Button states: true = button ON, false = button OFF
int button_states[5] = {0, 0, 0, 0, 0};
int switch_button_states[5] = {0, 0, 0, 0, 0};
int settings_button_states[5] = {0, 0, 0, 0, 0};

long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

// LED states
bool led_states[5] = {false, false, false, false, false};

byte ppqn = 0; // "Pulse Per Quarter Note"





// Utility functions

// Event type is hard-coded (0x09 = note on, 0x08 = note off).
// First parameter is the MIDI channel, combined with the note-on/note-off.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Second parameter (pitch) is the note number (48 = middle C).
// Third parameter is the velocity (0 -> 127, 0 = no_sound, 64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity)
{
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity)
{
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
}


// Event type hard-coded (0x0B = control change, aka "MIDI CC").
// First parameter is the channel (0-15), combined with the event type.
// Second parameter is the "control change" number (0-119, see top link to midi.org).
// Third parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value)
{
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
    MidiUSB.sendMIDI(event);
}


// BPM Counter
// commented because needs sync improvements
// void initBPM()
// {
//     midiEventPacket_t rx;
//     do {
//         rx = MidiUSB.read();
//         //Count pulses and send note
//         if (rx.byte1 == 0xF8)
//         {
//             ++ppqn;
//             if (ppqn == 24)
//             {
//                 digitalWrite(10, HIGH);
//                 delay(80);
//                 digitalWrite(10, LOW);
//                 ppqn = 0;
//             };
//         }
//         //Clock start byte
//     } while (rx.header != 0);
// }


// Turn off all LEDs
void poweroffLeds()
{
    for (uint8_t i = 0; i < 5; i++)
    {
        digitalWrite(led_pins[i], LOW);
        led_states[i] = false;
    }
}

// Turn on all the current mode LEDs
void showModeLeds()
{
    for (uint8_t i = 0; i < 5; i++)
    {
        if (button_modes[1][i] == 1)
        {
            digitalWrite(led_pins[i], HIGH);
            led_states[i] = true;
        }
    }
}


byte readLayerSwitch ()
{
    int switch0 = digitalRead(switch_pins[0]);
    int switch1 = digitalRead(switch_pins[1]);
    if (switch0 == LOW && switch1 == HIGH)
    {
        return 0; // Switch UP
    }
    else if (switch0 == HIGH && switch1 == HIGH)
    {
        return 1; // Switch OFF
    }
    else if (switch0 == HIGH && switch1 == LOW)
    {
        return 2; // Switch DOWN
    }
}


void initLayer()
{
    current_layer = 0; //readLayerSwitch();
}


byte checkLayer()
{
    byte readLayer = readLayerSwitch();
    if (readLayer == 0 && current_layer != 0)
    {
        poweroffLeds();
        current_layer = 0; // Switch UP
        return 0;
    }
    else if (readLayer == 1 && current_layer != 1)
    {
        poweroffLeds();
        current_layer = 1; // Switch OFF
        return 1;
    }
    else if (readLayer == 2 && current_layer != 2)
    {
        poweroffLeds();
        current_layer = 2; // Switch DOWN
        return 2;
    }
}


// Button in "Push" mode
void handle_push_button(byte i)
{
    if (digitalRead(button_pins[i]) == LOW && button_states[i] == 0)
    {
        controlChange(0, button_layers[current_layer][i], 127);
        MidiUSB.flush();
        button_states[i] = 1;
        digitalWrite(led_pins[i], HIGH); // Turn the LED on
        led_states[i] = true;
        delay(15);
    }
    else if (digitalRead(button_pins[i]) == HIGH && button_states[i] == 1)
    {
        controlChange(0, button_layers[current_layer][i], 0);
        MidiUSB.flush();
        button_states[i] = 0;
        digitalWrite(led_pins[i], LOW); // Turn the LED off
        led_states[i] = false;
        delay(15);
    }
}


// Button in "Toggle" mode
void handle_switch_button(byte i)
{
    int buttonState = digitalRead(button_pins[i]);
    if (buttonState != switch_button_states[i]){
        if (buttonState == LOW && button_states[i] == 0)
        {
            controlChange(0, button_layers[current_layer][i], 127);
            MidiUSB.flush();
            button_states[i] = 1;
            digitalWrite(led_pins[i], HIGH); // Turn the LED on
            led_states[i] = true;
        }
        else if (buttonState == LOW && button_states[i] == 1)
        {
            controlChange(0, button_layers[current_layer][i], 0);
            MidiUSB.flush();
            button_states[i] = 0;
            digitalWrite(led_pins[i], LOW); // Turn the LED off
            led_states[i] = false;
        }
        delay(100);
    }
    switch_button_states[i] = buttonState;
}


void handle_buttons (byte layer)
{
    for (byte i = 0; i < NUM_BUTTONS; i++)
    {
        // Reminder: 0 = Push button, 1 = Switch button
        if (button_modes[layer][i] == 0)
        {
            handle_push_button(i);
        }
        else
        {
            handle_switch_button(i);
        }
    }
}


void handle_settings_buttons (byte layer)
{
    showModeLeds(); // Higlight mode LEDs

    for (byte i = 0; i < NUM_BUTTONS; i++)
    {
        int buttonState = digitalRead(button_pins[i]);
        if (buttonState != settings_button_states[i]){
            // Note: only layer 1 (switch OFF) can be customized
            if (buttonState == LOW && button_modes[1][i] == 0 && millis() - time > debounce)
            {
                button_modes[1][i] = 1;
                digitalWrite(led_pins[i], HIGH); // Turn the LED on
                led_states[i] = true;
                time = millis();
            }
            else if (buttonState == LOW && button_modes[1][i] == 1 && millis() - time > debounce)
            {
                button_modes[1][i] = 0;
                digitalWrite(led_pins[i], LOW); // Turn the LED off
                led_states[i] = false;
                time = millis();
            }
            delay(50);
        }
        settings_button_states[i] = buttonState;   
    }
}
