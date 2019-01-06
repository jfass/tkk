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

#define LED_CONFIG  (DDRD |= (1<<6))
#define LED_ON    (PORTD &= ~(1<<6))
#define LED_OFF    (PORTD |= (1<<6))
#define CPU_PRESCALE(n)  (CLKPR = 0x80, CLKPR = (n))


void zero(void);
void arrow(void);
void page(void);
void normal(void);
void empty(void);


//128 64 32 16  8 4 2 1
uint8_t modifier_keys[4] =
    {KEY_CTRL,       KEY_SHIFT,       KEY_ALT,       KEY_GUI,       };
uint8_t modifier_key_numbers[4] =
    {128+64+32+16+1, 128+64+32+16+2, 128+64+32+16+4, 128+64+32+16+8,};
uint8_t modifier_chord_id = 0;

// So I'm trying to make this array of chords kinda readable
// It's broken up into sixteen sections, each one has a set of all the
// chords formed with the same pattern on the left hand, and goes through
// the binary sequency of patterns for the right hand.
// That is to say, for each set the chord pattern goes like:
//    xx xx  .. .. ,  xx xx  .. .# ,  xx xx  .. #. ,  xx xx  .. ## ,
//    xx xx  .# .. ,  xx xx  .# .# ,  xx xx  .# #. ,  xx xx  .# ##  ,
//    xx xx  #. .. ,  xx xx  #. .# ,  xx xx  #. #. ,  xx xx  #. ## ,
//    xx xx  ## .. ,  xx xx  ## .# ,  xx xx  ## #. ,  xx xx  ## ##  ,
// Each entry consists of a function for how that chord should
// be interpreted, and a chord value that represents what that
// chord should send as a key to the computer.
// Like:    [function, value]
// So for example, the chord '.. ..  .# .#' represents the letter 'k'
// when that key is pressed, it is interpretted as a regular keypress
// so it gets the function 'normal' shorthanded to 'n'.
// Like:    [n,KEY_K        ] 
// All the empty space is to maintain a nice readable grid.

void (*z)(void) = zero;
void (*a)(void) = arrow;
void (*p)(void) = page;
void (*n)(void) = normal;
void (*e)(void) = empty;


uint8_t key_map[1024] = {
// .. ..
    [z,0            ],[n,KEY_P        ],[n,KEY_O        ],[n,KEY_SEMICOLON],
    [n,KEY_I        ],[n,KEY_K        ],[n,KEY_L        ],[e,0            ],
    [n,KEY_N        ],[n,KEY_M        ],[n,KEY_U        ],[n,KEY_RIGHT_BRACE],
    [n,KEY_H        ],[n,KEY_LEFT_BRACE],[e,0           ],[n,KEY_SPACE    ],
// .. .#
    [n,KEY_T        ],[n,KEY_BACKSPACE],[n,KEY_G        ],[e,0            ],
    [n,KEY_V        ],[e,0            ],[e,0            ],[e,0            ],
    [n,KEY_B        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[n,KEY_ENTER    ],
// .. #.
    [n,KEY_E        ],[n,KEY_QUOTE    ],[n,KEY_MINUS    ],[n,KEY_EQUAL    ],
    [n,KEY_COMMA    ],[e,0            ],[e,0            ],[e,0            ],
    [n,KEY_Y        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// .. ##
    [n,KEY_R        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// .# ..
    [n,KEY_S        ],[n,KEY_SPACE    ],[n,KEY_PERIOD   ],[e,0            ],
    [n,KEY_Z        ],[e,0            ],[e,0            ],[e,0            ],
    [n,KEY_J        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// .# .#
    [n,KEY_C        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// .# #.
    [n,KEY_D        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// .# ##
    [n,KEY_0        ],[n,KEY_1        ],[n,KEY_2        ],[n,KEY_3        ],
    [n,KEY_4        ],[n,KEY_5        ],[n,KEY_6        ],[n,KEY_7        ],
    [n,KEY_8        ],[n,KEY_9        ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// #. ..
    [n,KEY_A        ],[n,KEY_SLASH    ],[n,KEY_BACKSLASH],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [n,KEY_Q        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[n,KEY_TILDE    ],
// #. .#
    [n,KEY_F        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// #. #.
    [n,KEY_X        ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// #. ##
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// ## ..
    [n,KEY_W        ],[n,KEY_UP       ],[n,KEY_LEFT     ],[n,KEY_DOWN     ],
    [n,KEY_RIGHT    ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// ## .#
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// ## #.
    [n,KEY_TAB      ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
// ## ##
    [n,KEY_ESC      ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ],
    [e,0            ],[e,0            ],[e,0            ],[e,0            ]
       };
uint16_t idle_count=0;

void read(void);
void keydown(void);
void keyup(void);
void keysend(void);

void dump_fifo(void);
void realtime(void);
void keyhold(void);

uint8_t d, d_prev, mask, i, chord_id, last_chord_id=0, modifier_pressed=0,timer=0,timeout=1500,timebetween=100;
uint8_t chord_keys[8]={0,0,0,0,0,0,0,0};

int main(void)
{
  // Stuff from the original keyboard code for, like, being a keyboard and stuff.

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


  // And now for the main event!

  while (1) {
    read();

    if (chord_id != 0 )
            timer++;

    if (((chord_id == 0) && (last_chord_id != 0)) || timer == timeout ) {
        timer=timeout-timebetween;
        mask = 1;
        for (i=0; i<4; i++) {
            if (modifier_key_numbers[i] == last_chord_id) {
                modifier_pressed = 1;
                if ((modifier_chord_id & mask) == 0 ) {
                    modifier_chord_id += 1<<i;
                }
            }
            mask = mask << 1;
        }
        if (modifier_pressed == 0) {
            usb_keyboard_press(key_map[last_chord_id][1],modifier_chord_id);
            modifier_chord_id = 0;
        }
        last_chord_id = 0;
        modifier_pressed = 0;
    }
    if (chord_id > last_chord_id) {
        last_chord_id = chord_id;
    }

    // now the current pins will be the previous, and
    // wait a short delay so we're not highly sensitive
    // to mechanical "bounce".
    d_prev = d;
    _delay_ms(2);
  }
}


void read(void){
  // read all port D pins
  d = PIND;
  chord_id = ~d;
  mask = 1;// sweepy sweepy sweep my byte!
  for (i=0; i<8; i++) {
    if ((d & mask) == 0)  { // && (d_prev & mask) != 0) {
              chord_keys[i] = 1;
          }
    mask = mask << 1;
  }
}
