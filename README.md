# Amiga Input To USB

This is a project to connect a Commodore Amiga mouse, joystick
(Atari compatible) and maybe an Amiga Keyboard as USB HID
devices.

# Overview

The purpose of this project is to make and document a small,
inexpensive device that you can use to hook up some Commodore Amiga
(and Atari) input peripherals to modern computers with USB inputs.
It presents itself to the modern host computer as a USB keyboard
and USB mouse.

It has the following supported ports:

- D9 Male mouse/joystick port
- 4P4C (RJ-22) Amiga 1000 Keyboard port
- or, 5-Pin DIN connector can be used for A2000/3000 keyboard
- or, 6-Pin mini-DIN for A4000 keyboard

It supports the following schemes:

- Amiga mouse as HID mouse
- Atari mouse as HID mouse
- Atari Joystick as HID mouse
- Atari Joystick as various HID Keyboard control sets
- Amiga Keyboard as HID Keyboard (stretch goal)

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
hardware would be an 4P4C (RJ-22) jack (telephone handset), 5-pin DIN jack,
or 6-pin mini-DIN Jack for various Amiga keyboard models.  The native
interface connection on the prototyped board uses the Amiga 500
keyboard pinout, so that is also an option. (They all speak the
same protocol.

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
- NOTE: swapping two lines XB and YA in the code will let this work for Atari mice too.
  - in implementation this means data for D9 pins 1 and 4 need to be swapped

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

## Amiga Keyboard
- Support for using an Amiga keyboard as a HID USB keyboard
 - All Amiga keyboards all use the same protocol, but different connectors 
 - My hardware uses an 8 position header for the interface, wired as an A500 keyboard port
 - Interface cables can be made to connect to 
  - Amiga 1000 keyboards (4P4C (RJ-22) jack)
  - Amiga 2000,3000 keyboards (5-pin DIN jack)
  - Amiga 4000 keyboards (6-pin mini-DIN jack)
   
