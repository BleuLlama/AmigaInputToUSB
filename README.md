# Amiga Input To USB

This is a project to connect a Commodore Amiga mouse, Atari joystick
and maybe an Amiga Keyboard as USB HID devices.

# Overview

The purpose of this project is to make and document a small,
inexpensive device that you can use to hook up some Commodore Amiga
and Atari input peripherals to modern computers with USB inputs.
It presents itself to the modern host computer as a HID USB keyboard
and HID USB mouse.

It has the following supported ports:

- D9 Male mouse/joystick port
- 8 pin Amiga 500 keyboard port. (future)
- Adapters can be made for: 
 - 4P4C (RJ-22) Amiga 1000 Keyboard port
 - 5-Pin DIN connector can be used for A2000/3000 keyboard
 - 6-Pin mini-DIN for A4000 keyboard

It supports the following schemes:

- Amiga mouse as HID mouse
- Atari mouse as HID mouse
- Atari Joystick as HID mouse
- Atari Joystick as various HID Keyboard control sets
- Amiga Keyboard as HID Keyboard (future)

This was originally made for the http://retrochallenge.org Retro
Challenge 2015/7.

# Theory of operation 

The entire design is based around the ATmega 32u4 microcontroller
with the Arduino bootloader installed.  This part with support
hardware can be purchased under the name "Arduino Leonardo", or
"Arduino Pro Micro".  The Arduino libraries take care of implementing
the full USB HID stacks necessary for the device to present itself
as a common HID keyboard or common HID mouse.  ("Authentic" boards
can be purchased for $10-$25 each, or through some Chinese production
places at about $2 for a Pro Micro.)

The additional hardware necessary to hook these boards up to a
device is basically a male D9 connector, for the mouse or joystick
to plug into.  For an Amiga keyboard interface, the necessary
hardware would be an 4P4C (RJ-22) jack (telephone handset), 5-pin
DIN jack, or 6-pin mini-DIN Jack for various Amiga keyboard models.
The native interface connection on the prototyped board uses the
Amiga 500 keyboard pinout, so that is also an option. (They all
speak the same protocol.

When plugged in, the device will appear to be a USB Keyboard, USB
Mouse and USB Serial Port.  (The serial port is used for configuration.)

Amiga and Atari mice output quadrature for X and Y using two lines
each in gray code.  As you move positive X or Y, it outputs on two
data lines the gray code sequence over time: 00 01 11 10 00 and so
on.  for negative movement, it outputs the sequence 00 10 11 01 00
etc.  By sitting in a tight loop, we can determine these X and Y
movements, and output them using the Arduino's Mouse library.

Those two function identically, although the Atari mouse swaps the
signals found on the D9 connector's pin 1 and 4.

# Development Phases

## Prototype / Proof of concept (complete)
- Simple implementation of Amiga mouse movement and button clicking

## Amiga Mouse full implementation (complete)
- Left, Right, Middle Mouse button supported
- Movement with reasonable acceleration

## Atari ST Mouse full implementation (complete 7/20)
- Left, Right buttons supported
- Movement with reasonable acceleration
- pin defines operate the same as Amiga mouse but D9.1 and D9.4 are swapped

## Atari Joystick as mouse (complete)
- Joystick movement translated to Mouse movements (like joymouse)
- Joystick button(s) convert to mouse button presses
- Movement with reasonable acceleration

## Serial configuration (complete 7/12)
- simple serial shell to change configuration parameters
- display currently configured modes
- stored in EEPROM

## Atari Joystick as keyboard (complete 7/13)
- Joytick movements mapped as keyboard
- Support via preset key configurations for: FS-UAE, Mame, Stella, Vice, WASD, and vi
 - FS-UAE: Arrow keys for movement, right ctrl, right alt for buttons
 - MAME: Arrow keys for movement, left ctrl, left alt, space for buttons
 - WASD: WASD for movement, space bar, ctrl, shift
 - vi: HJKL for movement, (yes, it's a joke. yes, it's usable!)

## Amiga Keyboard (future)
- Support for using an Amiga keyboard as a HID USB keyboard
 - All Amiga keyboards all use the same protocol, but different connectors 
 - My hardware uses an 8 position header for the interface, wired as an A500 keyboard port
 - Interface cables can be made to connect to 
  - Amiga 1000 keyboards (4P4C (RJ-22) jack)
  - Amiga 2000,3000 keyboards (5-pin DIN jack)
  - Amiga 4000 keyboards (6-pin mini-DIN jack)

# Future possibilities/features
- Autodetect Amiga Mouse / Atari Mouse / Atari Joystick (See below) 
- Pushbuttons with 7 segment display to select mode without serial interface
- N port input (instead of 1 port) via use of a series of 8 bit
parallel in, serial out shift register (74HC165).  This could be a
modular, chainable board, where you plug one into the next.  Make
4 identical boards so that Atari 800 M.U.L.E. would be properly
emulated... or make 3, use two for user joysticks, one for Mouse
input, etc.

# Joystick Support:
## Supported
- Atari 2600 VCS - supported
- Atari 7800 - somewhat supported (uses B2, B3)

## Could be supported with code rework (not currently supported)
- Odyssey - Possible, requires rework of the code

## Require rewiring to work (not supported)
- TI 99/4A - Not possible. Requires rewiring
- Sega Master System - Requires 5v on pin 5
- Sega Genesis - Requires 5v on pin 5, rework
- Sega Saturn - Requires rewiring and code rework

# Current Status Notes

### 2015/07/20

My Atari ST mouse has arrived in the mail today! I have tweaked
the code, cleaned it up a little, and confirmed that it works
with this mouse.  Images have been added to the project to
show the Leonardo hookup diagram, as well as diagrams for
converting between Amiga and Atari mice.

### 2015/07/15

The initial version is done, minus keyboard support.  I've been
having power problems getting the Amiga keyboards to be powerable
off of the Arduino/USB power.

I need to make a small 8 pin adapter to "inject" power into the
cord for the keyboards.  Then I need to figure out how to properly
read from the keyboard and adapt it appropriately.  There is example
code which I've included in this project that does this, however,
I may end up rewriting it from scratch to get a better understanding
of how it works.

Last night I did sketch out ideas in figuring out how to autodetect
between Atari Joystick, Amiga Mouse and Atari Mouse.  I think that 
just by doing some simple analysis of the signals coming in, it 
should be possible to determine the device type.  When it powers on,
the device will go into a "detection" state.  In this state, it waits for 
N changes for the U/D/L/R or V/H/VQ/HQ (Amiga) or HQ/H/VQ/V (Atari) 
data bits.  By profiling these, it should be trivial to detect between 
mouse and joystick.  By profiling a little more, it should be possible 
to detect between Atari and Amiga mice. 

If we look at the signal sets, we can determine easily if it's not 
a joystick.  On a joystick it is impossible for (Up and Down) to be
low at the same time... same with (Left and Right).  If we ever see 
a case where either of those two situations occur, we know it's not a
joystick.

From there, we can profile the data in 4 pairs: (1,3), (2,4), (1,2),
(3,4).  By looking at the data, we can determine which of those
four have had only valid transition sequences: (forward or backwards
in the following two bit gray code sequence)

	00 - 01 - 11 - 10 - 00

If the valid transitions only happen in the (1,3) and (2,4)
pairs, then it is an Amiga Mouse.  If the valid transitions only
happen in the (1,2) and (3,4) pairs, then we know it's an Atari
mouse.
