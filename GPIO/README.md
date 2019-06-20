---
Author: tyrolyean
title: README of the Escape Game Systems microcontroller repository
Mainfont: Arial
---

# About

This is the controller software for the GPIO controller. Devices running this
program may only send status reports about their pins. Analog pins are seen as a
kinda digital extension, as well a PWM pins are. Devices with this code are
ECPROTO slaves.

# The protocol

Offloaded to the **../ECPROTO.md** file. For further information please consult
that file.

# Build instructions

The instructions for building the thing arein **../README.md**

## Microcontroller specific build options

- UART_SECONDARY:
  UART_SECONDARY may be defined to enable transmission via the SECONDARY print
  action on the second UART interface on the ATMega2560.

- ANALOG_EN:
  Enables the deprecated old analog module. This has been replaced by the new
  analog system and is due for removal. The new analog system is on by default.

# License

This program is licensed under the GNU General Public License Version 3 or (at
your option) any later Version of the license. Please see the ../LICENSE.md file
for further information on the license.

