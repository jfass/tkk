
## This is the crazy madness I typed while hacking on this thing.
I haven't read through it since then, and can't be sure it makes sense or is useful.

---------------------------------------------

I'm going to make a chorded keyboard using asetniop typing layout, a teensyduino, and cherry mx brown keyswitches.

The process I think I'll be following is:
 - design and print the key switch plate.
 - connect the keys up to the teensyduino.
 - hack on some c code until I can type something.
 - design and print the keyboard case.
 - reverse engineer / design asetniop schema
 - type delightful messages of joy with my new keyboard.

I have a good amount of experience 3d printing on me and my friends printerbot, a good amount of experience modeling objects for 3d printing. I've used blender3d more extensively, but I'm going to use openscad for this project because I want it to be parametric, and opensource. (I'd like to see blender3d expand to have more parametric and cad tools, but that discussion is not something I should document here)

I'm going to start 3d by modeling and 3d printing a board for the keys to fit into.
[this datasheet](http://imgur.com/a/XMoHF) didn't seem too useful. I'll probably measure the keycap unless I can find a useful description of the mounting hole.
[This switch plate](http://www.thingiverse.com/thing:573578/#files) by [rsheldiii](http://www.thingiverse.com/rsheldiii/about) should serve as a good starting point. I should remember to thank them.

Ok, so with two variables changed, I've got a plate with four mx mount holes. That's the power of parametric code. A bunch more hacking and a bit of irriversable damage to rsheldiii's code and I have a switch plate that should be worky enough for the first prototype!


While I'm waiting for the printer to finish up the wallace part it's printing, I'm going to look around for some c code for teensy keyboard to look at and ultimately hack apart and replace the innerd of. As well as making an awesome keyboard I'm hoping to use this project to expand my c coding skills, which right now consist of half of zed's 'learn c the hard way' and the time I hacked on an led blinking script for the teensyduino.

[There's code for keyboarding](http://www.pjrc.com/teensy/usb_keyboard.html) available on the teensyduino site! Seems like a reasonable source.

I've looked into the keyboardcode, it looks pretty straightforward, the main logic is happening in the example.c. I'm going to solder cable to the keys, and then attach them to the teensy and test it out.

Everything is soldered up, and I've managed to hack the teensy code into basic asetniop workyness!

I've added support for modifier keys. (control, shift, alt, and super) And printed the cases and most of the keys.

I'd like to add a system to model keyboard layouts that includes different kinds of keypress events, such as keypress direction and keypress timing. I'm imagining something like a linked list tree with leaf nodes that link back to the root or some higher branch, while triggering some key event.
