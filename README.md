# asetniop-keyboard

This is my chorded keyboard project. If you don't already know, a chorded keyboard is a keyboard that makes use of combinations of keys pressed at the same time (a chords) rather than individual keypresses for typing. (It could be said that qwerty is chorded: Using chords like 'shift'+'A' for a capital 'A' or 'ctrl'+'V' for commonly found paste functions, but this misses the heart of the thing.)

![keyboard detail](../master/images/keyboard-1.jpg)

It works pretty good for the hacky experiment it is, and is rather comfortable to type on. I'm still not up to my qwerty typing speed, but it took me a long time to develop that, so I'm estimating that the time to develop typing speed on qwerty vs asetniop are about the same. Asetniop may have the difference that it will force you to touch type right away, which could be seen as good or bad.

You can watch a short typing demo [here](https://youtu.be/0AZoeCmnyow).

Inspired by [asetniop](http://asetniop.com/) and built using [teensy](http://www.pjrc.com/teensy/usb_keyboard.html).

![keyboard detail](../master/images/keyboard-2.jpg)

## Installation
Keys are wired up directly to the pins D0 - D7. There are ways to do this that would save pins, but I like this method because it's easy, and I don't need the extra pins for anything.

![wiring diagram](../master/images/wiring-diagram.png)

If you're on linux like me you may want to use the [bootloader](https://github.com/PaulStoffregen/teensy_loader_cli)

    sudo ./teensy_loader_cli/teensy_loader_cli --mcu=atmega32u4 asetniop.hex -w

Otherwise you'll have to flash the firmware some other way. Message me if you don't know how and I'll try to help you out.

## Bill of materials

 - [A teensyduino 2.0 board.](https://www.pjrc.com/store/teensy.html)
 - A 3D printed copy of each of the [.stl files.](https://github.com/TristanTrim/asetniop-keyboard/tree/master/printed_parts)
 - [8 Keyswitches](https://mechanicalkeyboards.com/shop/index.php?l=product_detail&p=708)
 - [8 Keycaps](http://www.wasdkeyboards.com/index.php/products/blank-keycap-singles/row-2-size-1x1-cherry-mx-keycap.html) (if you're not printing them)
 - wire and solder tools.
 - M3 nuts and bolts for the case.

## Typing guide

If you want to start learning about typing with this style of chorded keyboard, you can use the asetniop [keyboard tutorial](http://asetniop.com/keyboardTutorial.html) and [keyboard racer](http://asetniop.com/keyboardRacer.html) to start learning asetniop in your web browser using your qwerty keyboard. This way you can find out what its like to type with a chorded keyboard without needing to do anything radical like building a keyboard.

There are a number of additions to the asetniop chords to expand this project into a more general keyboard. Also my keyboard only has eight keys, so doesn't use the thumbs at all. I'll explain below.

For the purpose of this guide I'm representing chords with a series of '.'s and '#'s. The '#'s represent a key being pressed down, and the '.'s a key left unpressed. Read from left to right the keys represented are the left pinky key through to the right pinky key.

### Numbers
The numbers all use .### on the left as somewhat of a prefix, and then follows a binary sequence from 0 to 9.

    .### .... : 0
    .### ...# : 1
    .### ..#. : 2
    .### ..## : 3
    .### .#.. : 4
    .### .#.# : 5
    .### .##. : 6
    .### .### : 7
    .### #... : 8
    .### #..# : 9

### Arrow keys
Like with the numbers the arrow keys use a 'prefix'. They all start with the 'w' key; ##..

    ##.. ...# : up
    ##.. ..## : down
    ##.. ..#. : left
    ##.. .#.. : right

### Non-alphanumeric
The non-alphanumeric keys were some of the first I wanted for interacting with my computer. The way the 'ctrl', 'shift', 'alt', and 'mod/window' keys work is that you press the chord, and then it is applied to the next chord that is pressed. For example, to get a '?' you would press #### ..#. and then press #... ...#

    #### ...# : ctrl
    #### ..#. : shift
    #### .#.. : alt
    #### #... : mod/window
    .... #### : space
    ...# #### : enter
    ###. .... : tab
    #### .... : esc

The rest of the keys may be a bit confusing, because I didn't add chords for anything that would normally be accessed by pressing shift+'some key'. So, for example, to use a '!' you need to press the chord for shift followed by the key for 1. because of this it's important to remember where all the symbols are on the qwerty keyboard. (assuming you want to use them.)

Of the rest of the keys, some of them may be in the original asetniop, and others I may have added or changed. I don't really remember which is which, sorry.

    .... ##.# : [
    .... #.## : ]
    #... ...# : /
    #... ..#. : \
    #... #### : `
    ..#. ..#. : -
    ..#. ..## : =
    
## Random nonsense

The next thing on my todo list with this project is adding functionality for seperating chords when the typist accidentally starts a second chord before fully releasing the first.

I'd like to add the rest of the keys on a standard keyboard, and maybe some special function keys, but I'm not sure when I'll get around to it.

Further down the line I would like to experiment with adding joystick / mouse functionality, and building a wireless pair.

Or abandoning this and making a pair that maps the fingers onto a ternary, rather than binary, system. You know, for the 3 positions ^ 10 fingers = 59049 chords, rather than the 2 positions ^ 8 fingers = 256 chords availiable on my current system. If I assume 12 axes because the thumbs move in two axes so nicely that gives me: 3 positions ^ 12 axes = 531441. That's 3 positions ^ 6 axes = 729 chords on one hand! The trouble then would never be space, but figuring out how to organise all the chords. Ok, it would probably still be space at some point, because who wants to be stuck with a vocabulary of just over half a million. So lame right?

