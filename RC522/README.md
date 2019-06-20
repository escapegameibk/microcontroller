---
Author: tyrolyean
title: README of the Escape Game Systems microcontroller repository
Mainfont: Arial
---

# About

This is the controller software for an ecproto capable device which is used to
connect multiple RC522 devices to the given controller.

# Build instructions

The instructions for building the thing are in **../README.md**

## Microcontroller specific build options

- SEND_PIN:
  SEND_PIN enables the use of pin D2 as RX/TX Switch with sending beeing
  high and receiving beeing low. Pin D2 is therefore no longer able to be used
  as a salve select pin.

# LICENSE

This program is licensed under the GNU General Public License Version 3, or 
(at your option) any later Version of the license. See ../LICENSE.md for further
details

# The protocol

Offloaded to the **ECPROTO.md** file
