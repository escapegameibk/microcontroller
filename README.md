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

Offloaded to the ECPROTO.md file
