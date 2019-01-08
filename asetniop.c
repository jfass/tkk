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
void empty(void);
void normal(void);
void meta(void);
void arrow(void);
void page(void);

typedef void (*func)(void);
func keymap_func[5] = {zero, empty, normal, meta, arrow, page};
#define z 0
#define e 1
#define n 2
#define m 3
#define a 4
#define p 5


//128 64 32 16  8 4 2 1
uint8_t modifier_keys[4] =
    {KEY_CTRL,       KEY_SHIFT,       KEY_ALT,       KEY_GUI,       };
uint8_t modifier_key_numbers[4] =
    {128+64+32+16+1, 128+64+32+16+2, 128+64+32+16+4, 128+64+32+16+8,};
uint8_t modifier_chord_id;

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


uint8_t key_map[256][2] = {
// .. ..    0
    {z,0            },{n,KEY_P        },{n,KEY_O        },{n,KEY_SEMICOLON},    // 0   1   2   3
    {n,KEY_I        },{n,KEY_K        },{n,KEY_L        },{e,0            },    // 4   5   6   7
    {n,KEY_N        },{n,KEY_M        },{n,KEY_U        },{n,KEY_RIGHT_BRACE},    // 8   9   10   11
    {n,KEY_H        },{n,KEY_LEFT_BRACE},{e,0           },{n,KEY_SPACE    },    // 12   13   14   15
// .. .#    16
    {n,KEY_T        },{n,KEY_BACKSPACE},{n,KEY_G        },{e,0            },    // 16   17   18   19
    {n,KEY_V        },{e,0            },{e,0            },{e,0            },    // 20   21   22   23
    {n,KEY_B        },{e,0            },{e,0            },{e,0            },    // 24   25   26   27
    {e,0            },{e,0            },{e,0            },{n,KEY_ENTER    },    // 28   29   30   31
// .. #.    32
    {n,KEY_E        },{n,KEY_QUOTE    },{n,KEY_MINUS    },{n,KEY_EQUAL    },    // 32   33   34   35
    {n,KEY_COMMA    },{e,0            },{e,0            },{e,0            },    // 36   37   38   39
    {n,KEY_Y        },{e,0            },{e,0            },{e,0            },    // 40   41   42   43
    {e,0            },{e,0            },{e,0            },{e,0            },    // 44   45   46   47
// .. ##    48
    {n,KEY_R        },{e,0            },{e,0            },{e,0            },    // 48   49   50   51
    {e,0            },{e,0            },{e,0            },{e,0            },    // 52   53   54   55
    {e,0            },{e,0            },{e,0            },{e,0            },    // 56   57   58   59
    {e,0            },{e,0            },{e,0            },{e,0            },    // 60   61   62   63
// .# ..    64
    {n,KEY_S        },{n,KEY_SPACE    },{n,KEY_PERIOD   },{e,0            },    // 64   65   66   67
    {n,KEY_Z        },{e,0            },{e,0            },{e,0            },    // 68   69   70   71
    {n,KEY_J        },{e,0            },{e,0            },{e,0            },    // 72   73   74   75
    {e,0            },{e,0            },{e,0            },{e,0            },    // 76   77   78   79
// .# .#    80
    {n,KEY_C        },{e,0            },{e,0            },{e,0            },    // 80   81   82   83
    {e,0            },{e,0            },{e,0            },{e,0            },    // 84   85   86   87
    {e,0            },{e,0            },{m, 0           },{e,0            },    // 88   89   90   91
    {e,0            },{e,0            },{e,0            },{e,0            },    // 92   93   94   95
// .# #.    96
    {n,KEY_D        },{e,0            },{e,0            },{e,0            },    // 96   97   98   99
    {e,0            },{e,0            },{e,0            },{e,0            },    // 100   101   102   103
    {e,0            },{e,0            },{e,0            },{e,0            },    // 104   105   106   107
    {e,0            },{e,0            },{e,0            },{e,0            },    // 108   109   110   111
// .# ##    112
    {n,KEY_0        },{n,KEY_1        },{n,KEY_2        },{n,KEY_3        },    // 112   113   114   115
    {n,KEY_4        },{n,KEY_5        },{n,KEY_6        },{n,KEY_7        },    // 116   117   118   119
    {n,KEY_8        },{n,KEY_9        },{e,0            },{e,0            },    // 120   121   122   123
    {e,0            },{e,0            },{e,0            },{e,0            },    // 124   125   126   127
// #. ..    128
    {n,KEY_A        },{n,KEY_SLASH    },{n,KEY_BACKSLASH},{e,0            },    // 128   129   130   131
    {e,0            },{e,0            },{e,0            },{e,0            },    // 132   133   134   135
    {n,KEY_Q        },{e,0            },{e,0            },{e,0            },    // 136   137   138   139
    {e,0            },{e,0            },{e,0            },{n,KEY_TILDE    },    // 140   141   142   143
// #. .#    144
    {n,KEY_F        },{e,0            },{e,0            },{e,0            },    // 144   145   146   147
    {e,0            },{e,0            },{e,0            },{e,0            },    // 148   149   150   151
    {e,0            },{e,0            },{e,0            },{e,0            },    // 152   153   154   155
    {e,0            },{e,0            },{e,0            },{e,0            },    // 156   157   158   159
// #. #.    160
    {n,KEY_X        },{e,0            },{e,0            },{e,0            },    // 160   161   162   163
    {e,0            },{e,0            },{e,0            },{e,0            },    // 164   165   166   167
    {e,0            },{e,0            },{e,0            },{e,0            },    // 168   169   170   171
    {e,0            },{e,0            },{e,0            },{e,0            },    // 172   173   174   175
// #. ##    176
    {e,0            },{e,0            },{e,0            },{e,0            },    // 176   177   178   179
    {e,0            },{e,0            },{e,0            },{e,0            },    // 180   181   182   183
    {e,0            },{e,0            },{e,0            },{e,0            },    // 184   185   186   187
    {e,0            },{e,0            },{e,0            },{e,0            },    // 188   189   190   191
// ## ..    192
    {n,KEY_W        },{a,KEY_RIGHT    },{a,KEY_UP       },{e,0            },    // 192   193   194   195
    {a,KEY_DOWN     },{e,0            },{e,0            },{e,0            },    // 196   197   198   199
    {a,KEY_LEFT     },{e,0            },{e,0            },{e,0            },    // 200   201   202   203
    {e,0            },{e,0            },{e,0            },{e,0            },    // 204   205   206   207
// ## .#    208
    {e,0            },{e,0            },{e,0            },{e,0            },    // 208   209   210   211
    {e,0            },{e,0            },{e,0            },{e,0            },    // 212   213   214   215
    {e,0            },{e,0            },{e,0            },{e,0            },    // 216   217   218   219
    {e,0            },{e,0            },{e,0            },{e,0            },    // 220   221   222   223
// ## #.    224
    {n,KEY_TAB      },{p,KEY_END      },{p,KEY_PAGE_UP  },{e,0            },    // 224   225   226   227
    {p,KEY_PAGE_DOWN},{e,0            },{e,0            },{e,0            },    // 228   229   230   231
    {p,KEY_HOME     },{e,0            },{e,0            },{e,0            },    // 232   233   234   235
    {e,0            },{e,0            },{e,0            },{e,0            },    // 236   237   238   239
// ## ##     240  These are mostly all modifier keys. They are handled elsewhere, but still called with n
    {n,KEY_ESC      },{n,0            },{n,0            },{n,0            },    // 240   241   242   243
    {n,0            },{n,0            },{n,0            },{n,0            },    // 244   245   246   247
    {n,0            },{n,0            },{n,0            },{n,0            },    // 248   249   250   251
    {n,0            },{n,0            },{n,0            },{e,0            }     // 252   253   254   255
};

uint8_t realtime_index;
uint8_t realtime_map[2][4] =
{
    {KEY_LEFT       ,KEY_DOWN        ,KEY_UP          ,KEY_RIGHT       },
    {KEY_END        ,KEY_PAGE_DOWN   ,KEY_PAGE_UP     ,KEY_HOME        }
};

void read(void);
void keydown(void);
void keyup(void);
void keysend(void);

void dump_fifo(void);
void debug_dump(void);
void realtime(void);
void keyhold(void);
void modpress(uint8_t modchord);
void send(int send_id);

long i;
uint8_t d, prev_d, mask, chord_id, prev_chord_id, pressed, modifier_pressed,timer;
#define timeout 5000
#define timebetween 100
uint8_t chord_keys[8];//={0,0,0,0,0,0,0,0};

uint8_t fifo_index;
uint16_t fifo[64][2];
uint16_t longest;
int chord_accept_numer = 1;
int chord_accept_denom = 3;

int debug_mode = 0;

uint8_t modkeys;
uint8_t modlocks;

#define keydown_delay  1500
#define min_press_detect  0

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
    keymap_func[  key_map[chord_id][0]  ]();
  }
}
///////////////////// we have lost cabin pressure.
//
//    if (((chord_id == 0) && (last_chord_id != 0)) || timer == timeout ) {
//        timer=timeout-timebetween;
//
//        mask = 1;
//        for (i=0; i<4; i++) {
//            if (modifier_key_numbers[i] == last_chord_id) {
//                modifier_pressed = 1;
//                if ((modifier_chord_id & mask) == 0 ) {
//                    modifier_chord_id += 1<<i;
//                }
//            }
//            mask = mask << 1;
//        }
//        if (modifier_pressed == 0) {
//            usb_keyboard_press(key_map[last_chord_id][1],modifier_chord_id);
//            modifier_chord_id = 0;
//        }
//        last_chord_id = 0;
//        modifier_pressed = 0;
//    }
//    if (chord_id > last_chord_id) {
//        last_chord_id = chord_id;
//    }
//
//    // now the current pins will be the previous, and
//    // wait a short delay so we're not highly sensitive
//    // to mechanical "bounce".
//    _delay_ms(2);
////////////////////////////// cabin pressure stabalized.


// keymap functions:

void zero(void){
  // When keys have been pressed and we are at zero
  // we know it's time to send those pressed keys!
  if(pressed){
    if(debug_mode){
      debug_dump();
    }
    dump_fifo();
  }
}
void empty(void){
  // As if we even care what happens when undefined
  // keys are pressed!
}
void normal(void){
  if(!pressed){
    fifo[fifo_index][0] = chord_id;
    pressed=1;
  }else{
    if(fifo[fifo_index][0] == chord_id){
      if(fifo[fifo_index][1] > keydown_delay){
        keyhold();
      }else{
        if(++fifo[fifo_index][1] > longest)
          longest = fifo[fifo_index][1];
      }
    }else{
      fifo_index++;
      fifo[fifo_index][0] = chord_id;
    }
  }
}
void meta(void){
  while(1){
    read();
    if(chord_id != 90)
      break;
  }
  debug_mode = !debug_mode;
  if(debug_mode){
    usb_keyboard_press(KEY_D,2);
    usb_keyboard_press(KEY_E,2);
    usb_keyboard_press(KEY_B,2);
    usb_keyboard_press(KEY_U,2);
    usb_keyboard_press(KEY_G,2);
    usb_keyboard_press(KEY_ENTER,0);
  }else{
    usb_keyboard_press(KEY_B,2);
    usb_keyboard_press(KEY_A,2);
    usb_keyboard_press(KEY_C,2);
    usb_keyboard_press(KEY_K,2);
    usb_keyboard_press(KEY_SPACE,2);
    usb_keyboard_press(KEY_T,2);
    usb_keyboard_press(KEY_O,2);
    usb_keyboard_press(KEY_SPACE,2);
    usb_keyboard_press(KEY_N,2);
    usb_keyboard_press(KEY_O,2);
    usb_keyboard_press(KEY_R,2);
    usb_keyboard_press(KEY_M,2);
    usb_keyboard_press(KEY_A,2);
    usb_keyboard_press(KEY_L,2);
    usb_keyboard_press(KEY_SPACE,2);
    usb_keyboard_press(KEY_MINUS ,0);
    usb_keyboard_press(KEY_SPACE,2);
    usb_keyboard_press(KEY_N,2);
    usb_keyboard_press(KEY_O,2);
    usb_keyboard_press(KEY_T,2);
    usb_keyboard_press(KEY_H,2);
    usb_keyboard_press(KEY_I,2);
    usb_keyboard_press(KEY_N,2);
    usb_keyboard_press(KEY_G,2);
    usb_keyboard_press(KEY_SPACE,2);
    usb_keyboard_press(KEY_T,2);
    usb_keyboard_press(KEY_O,2);
    usb_keyboard_press(KEY_SPACE,2);
    usb_keyboard_press(KEY_S,2);
    usb_keyboard_press(KEY_E,2);
    usb_keyboard_press(KEY_E,2);
    usb_keyboard_press(KEY_SPACE,2);
    usb_keyboard_press(KEY_H,2);
    usb_keyboard_press(KEY_E,2);
    usb_keyboard_press(KEY_R,2);
    usb_keyboard_press(KEY_E,2);
    usb_keyboard_press(KEY_ENTER,0);
  }
}
void arrow(void){
  realtime_index = 0;
  realtime();
}
void page(void){
  realtime_index = 1;
  realtime();
}

// hardware functions:

void read(void){
  // read all port D pins
  d = PIND;
  i=0;
  while(1){// debouncing loop
    prev_d = d;
    d = PIND;
    if(d==prev_d)
      i++;
    else
      i=0;
    if(i==100)
      break;
  } 
  chord_id = ~d;
  mask = 1;// sweepy sweepy sweep my byte!
  for (i=0; i<8; i++) {
    if ((d & mask) == 0)  {
              chord_keys[i] = 1;
          }
    mask = mask << 1;
  }
}
void keydown(void){
}
void keyup(void){
}
void keysend(void){
  usb_keyboard_press(key_map[chord_id][1],modifier_chord_id);
}

// cool functions that I like:
void debug_dump(void){
  usb_keyboard_press(KEY_C,1);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_C,1);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_ENTER,0);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_SPACE,0);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_COMMA,2);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_3,0);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_SPACE,0);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  int accept_len = longest*chord_accept_numer/chord_accept_denom;
  for(i=0; i<=accept_len/20; i++)
    usb_keyboard_press(KEY_EQUAL,0);      /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  for(i=0; i<=(longest-accept_len)/20; i++)
    usb_keyboard_press(KEY_MINUS,0);      /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_C,1);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  usb_keyboard_press(KEY_ENTER,0);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  for(i=0; i<=fifo_index; i++){
    if(fifo[i][1] > accept_len){// YES! SEND IT
      usb_keyboard_press(KEY_1,2);
      usb_keyboard_press(KEY_1,2);
      usb_keyboard_press(KEY_1,2);
    }else{  /// NOT ENOUGH SENDAGE
      usb_keyboard_press(KEY_SPACE,0);
      usb_keyboard_press(KEY_SPACE,0);
      usb_keyboard_press(KEY_SPACE,0);
    }
    send(fifo[i][0]);//// ACTUALLY SEND THE DAMNED KEYPRESS
    for(int j =0; j<fifo[i][1]/20;j++)  /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
      usb_keyboard_press(KEY_MINUS,0);      /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
    usb_keyboard_press(KEY_C,1);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
    usb_keyboard_press(KEY_ENTER,0);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  }
  usb_keyboard_press(KEY_ENTER,0);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
}
void dump_fifo(void){
  int accept_len = longest*chord_accept_numer/chord_accept_denom;
  for(i=0; i<=fifo_index; i++){
    if(fifo[i][1] > accept_len){// YES! SEND IT
      send(fifo[i][0]);//// ACTUALLY SEND THE DAMNED KEYPRESS
      //usb_keyboard_press(KEY_1,2);
      //usb_keyboard_press(KEY_1,2);
      //usb_keyboard_press(KEY_1,2);
    }//else{  /// NOT ENOUGH SENDAGE
    //  usb_keyboard_press(KEY_SPACE,0);
    //  usb_keyboard_press(KEY_SPACE,0);
    //  usb_keyboard_press(KEY_SPACE,0);
    //}
    //for(int j =0; j<fifo[i][1]/10;j++)  /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
    //  usb_keyboard_press(KEY_COMMA,0);      /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
    //usb_keyboard_press(KEY_C,1);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
    fifo[i][1]=0;
  }
    //usb_keyboard_press(KEY_C,1);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
    //usb_keyboard_press(KEY_C,1);            /////////// DEBUGING TOOL, PRINT BASED ON HOW MUCH IT"S PRESSED
  longest = min_press_detect*2;
  pressed = 0;
  fifo_index = 0;
}
  // send_id is the id of the chord we're gonna send.
void send(int send_id){
  if(send_id>240&&send_id!=255){ // If it's a modifier chord
    modpress(send_id);
  }else if(send_id == 165){// 165 is #.#. .#.# and the clear mod key.
    modlocks =  0;
    modkeys = 0;
  }else{
    usb_keyboard_press(key_map[send_id][1], 0);//modkeys|modlocks);//////////////////////////FIX THIS
    modkeys = 0;
  }
}
void modpress(uint8_t modchord){
  uint8_t mod = modchord & 15;
  modlocks |= mod & modkeys;
  modkeys |= mod;
}
void realtime(void){
}
//    dump_fifo();
//    prev_chord_id = chord_id;
//    mask = 8;
//    for(i=0;i<4;i++){
//      if(chord_id & mask)
//        keyboard_keys[0] = realtime_map[realtime_index][i];
//        usb_keyboard_send();
//    }
//  }

void keyhold(void){
}
