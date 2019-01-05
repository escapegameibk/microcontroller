---
Author: tyrolyean
title: README of the Escape Game Systems microcontroller repository
Mainfont: Arial
---

# About

This is the controller software for the arduino which is built into the games
at the escape game innsbruck and it is responsible for handling all internal
traffic and acting as an I/O to serial "adapter".

# Documentation

This file includes documentation for several things:

## ARDUINO - Raspberry Pi

The transmission from the Arduino to the Raspberry Pi is done via a UART
connection from the arduino to the raspberry pi. It's connected  at the USART0 
connection of the arduino and the gpio shield of the raspberry pi.

## The protocol

Offloaded to the **ECPROTO.md** file

# Externally controllable components

This section describes external controllable devices via this source code.

## MFRC522

The program is capable of reading from multiple MFRC522 devices connected via 
SPI. The usual Client select lines have been replaced by a different mechanism,
thus they may be set on any position on the gpio headers. The MFRC522
devices are dynamically configured by the master and may NOT exceed a fixed
number of devices.

### License

The original library code was taken from github, where it was licensed under the
GNU General Public License Version 2 or higher.

# Build information

This section contains information on how to properly build the source code.

## Preprocessor defintions

There are multiple preprocessor definitions possible, for example:

- SPI_EN:
  Enables SPI instantiation, blacklisted pins.
