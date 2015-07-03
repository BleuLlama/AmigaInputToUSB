# Amiga Input To USB
This is a project to connect a Commodore Amiga mouse, joystick (Atari compatible) and eventually anAmiga Keyboard as USB HID devices.

# Overview

The purpose of this project is to make and document a small, inexpensive 
device that you can use to hook up some Commodore Amiga (and Atari) input
peripherals to modern computers with USB inputs.  It presents itself to
the modern host computer as a USB keyboard and USB mouse. 

It has the following supported ports:

- D9 Male mouse/joystick port
- RJ-22 Amiga 1000 Keyboard port
- alternately, DIN connector can be used for A2000 keyboard

It supports the following schemes:

- Amiga mouse as HID mouse
- Atari mouse as HID mouse
- Atari Joystick as HID mouse
- Atari Joystick as HID Keyboard controls
- Amiga Keyboard as HID Keyboard (stretch goal)

This was originally made for the http://retrochallenge.org Retro Challenge
2015/7.

# Theory of operation

The entire design is based around the ATmega 32u4 microcontroller with the
Arduino bootloader installed.  This part with support hardware can be 
purchased under the name "Arduino Leonardo", or "Arduino Pro Micro".  
The Arduino libraries take care of implementing the full USB HID stacks
necessary for the device to present itself as a common HID keyboard or
common HID mouse.  ("Authentic" boards  can be purchased for $10-$25 each, 
or through some Chinese production places at about $2 for a Pro Micro.)

The additional hardware necessary to hook these boards up to a device 
is basically a male D9 connector, for the mouse or joystick to plug into,
and a few 10K ohm resistors.

When plugged in, the device will appear to be a USB Keyboard, USB Mouse and 
USB Serial Port.  (The serial port is used for configuration.)

Amiga and Atari mice output quadrature for X and Y using two lines each in gray code.
As you move positive X or Y, it outputs on two data lines the gray code 
sequence over time: 00 01 11 10 00 and so on.  for negative movement, it 
outputs the sequence 00 10 11 01 00 etc.  By sitting in a tight loop, we can
determine these X and Y movements, and output them using the Arduino's
Mouse library.

# Development Phases

## Prototype / Proof of concept (complete)
- Simple implementation of Amiga mouse movement and button clicking

## Amiga Mouse full implementation (complete)
- Left, Right Mouse button supported
- Movement with reasonable acceleration
- NOTE: swapping two lines XB and YA in the code will let this work for Atari mice too.

## Atari Joystick as mouse (complete)
- Joystick movement translated to Mouse movements (like joymouse)

## Serial configuration
- simple serial shell to change configuration parameters
- stored in EEPROM

## Atari Joystick as keyboard
- Joytick movements mapped as keyboard
 - Arrow keys for movement, right ctrl, right alt for buttons (FS-UAE)
 - Arrow keys for movement, left ctrl, left alt, space for buttons (MAME P1)
 - RFGD for movement, a, s, q, for buttons (MAME P2)
 - WERSDFXCV for movement, space for button (VICE)
 - WASD for movement, space bar, ctrl, shift (Common WASD)
 - HJKL for movement, (vi)

## Amiga Keyboard
- Support for using an Amiga keyboard as a HID USB keyboard
 - A1000, A2000, A500 keyboards all use the same protocol, but different connectors and pinouts.
 - My hardware will be made to use an A1000 keyboard
