/* This is the asetniop teensy code.
 * It implements a version of asetniop on a teensy
 * Based on the teensy keyboard example:
 *
 * Keyboard example with debug channel, for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard.h"

#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

//128 64 32 16  8 4 2 1
uint8_t modifier_keys[4] =
    {KEY_CTRL,       KEY_SHIFT,       KEY_ALT,       KEY_GUI,       };
uint8_t modifier_key_numbers[4] =
    {128+64+32+16+1, 128+64+32+16+2, 128+64+32+16+4, 128+64+32+16+8,};
uint8_t modifier_key_state = 0;

uint8_t key_map[1024] =
    {0,KEY_P,KEY_O,KEY_SEMICOLON,KEY_I,KEY_K,KEY_L,0,
       KEY_N,KEY_M,KEY_U,KEY_RIGHT_BRACE,KEY_H,KEY_LEFT_BRACE,0,KEY_SPACE,
       KEY_T,KEY_BACKSPACE,KEY_G,0,KEY_V,0,0,0,
       KEY_B,0,0,0,0,0,0,KEY_ENTER,
       KEY_E,KEY_QUOTE,KEY_MINUS,KEY_EQUAL,KEY_COMMA,0,0,0,
       KEY_Y,0,0,0,0,0,0,0,
       KEY_R,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       KEY_S,0,KEY_PERIOD,0,KEY_Z,0,0,0,
       KEY_J,0,0,0,0,0,0,0,
       KEY_C,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       KEY_D,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,
       KEY_8,KEY_9,0,0,0,0,0,0,
       KEY_A,KEY_SLASH,KEY_BACKSLASH,0,0,0,0,0,
       KEY_Q,0,0,0,0,0,0,KEY_TILDE,
       KEY_F,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       KEY_X,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       KEY_W,KEY_UP,KEY_LEFT,KEY_DOWN,KEY_RIGHT,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       KEY_TAB,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       KEY_ESC,0,0,0,0,0,0,0,
       };
uint16_t idle_count=0;

int main(void)
{
	uint8_t d, mask, i, reset_idle, key_state, last_key_state=0, modifier_pressed=0,timer=0,timeout=1500;
	uint8_t d_prev=0xFF;
    uint8_t pressed_keys[8]={0,0,0,0,0,0,0,0};

	// set for 16 MHz clock
	CPU_PRESCALE(0);

	// Configure all port B and port D pins as inputs with pullup resistors.
	// See the "Using I/O Pins" page for details.
	// http://www.pjrc.com/teensy/pins.html
	DDRD = 0x00;
	PORTD = 0xFF;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	// Configure timer 0 to generate a timer overflow interrupt every
	// 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
	// This demonstrates how to use interrupts to implement a simple
	// inactivity timeout.
	TCCR0A = 0x00;
	TCCR0B = 0x05;
	TIMSK0 = (1<<TOIE0);

	while (1) {
		// read all port D pins
		d = PIND;
		// check if any pins are low, but were high previously
		mask = 1;
		reset_idle = 0;
		for (i=0; i<8; i++) {
			if ((d & mask) == 0)  { // && (d_prev & mask) != 0) {
                pressed_keys[i] = 1;
				reset_idle = 1;
            }
			mask = mask << 1;
		}
        key_state = 0;
		for (i=0; i<8; i++) {
            if (pressed_keys[i]) {
                key_state += 1 << i;
                pressed_keys[i] = 0;
            }
		}

        if (key_state != 0 ) {
            timer++;
        }

        //phex(modifier_key_state);
        if (((key_state == 0) && (last_key_state != 0)) || timer == timeout ) {
            timer=0;
            mask = 1;
            for (i=0; i<4; i++) {
                if (modifier_key_numbers[i] == last_key_state) {
                    modifier_pressed = 1;
                    if ((modifier_key_state & mask) == 0 ) {
                        modifier_key_state += 1<<i;
                    }
                }
                mask = mask << 1;
            }
            if (modifier_pressed == 0) {
                usb_keyboard_press(key_map[last_key_state],modifier_key_state);
                modifier_key_state = 0;
            }
            last_key_state = 0;
            modifier_pressed = 0;
        }
        if (key_state > last_key_state) {
            last_key_state = key_state;
        }

        //phex(pressed_key_value);
        //print("\n");
		// if any keypresses were detected, reset the idle counter
		if (reset_idle) {
			// variables shared with interrupt routines must be
			// accessed carefully so the interrupt routine doesn't
			// try to use the variable in the middle of our access
			cli();
			idle_count = 0;
			sei();
		}
		// now the current pins will be the previous, and
		// wait a short delay so we're not highly sensitive
		// to mechanical "bounce".
		d_prev = d;
		_delay_ms(2);
	}
}

// This interrupt routine is run approx 61 times per second.
// A very simple inactivity timeout is implemented, where we
// will (not send a space character) and print a message to the
// hid_listen debug message window.
ISR(TIMER0_OVF_vect)
{
	idle_count++;
	if (idle_count > 61 * 8) {
		idle_count = 0;
	}
}


