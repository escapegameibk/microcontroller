%EC-Proto

# About

After doing some research on the stuff, the old protocol seemed unusable and a
bit idiotic. It was deemed nescessary to rebuild it from scratch. The protocol
was designed to be as flexile as possible, whilest keeping it simple stupid. The
result was the EC-proto, which stands for escape the protocol, which is a 
master-slave based protocol. It may be driven over any layer 2 connection.

# Addressing

The protocol is very well able to address a host, and therefore no buffering
or any such thing is required. The address is always the SECOND bit transferred.

## Byte ordering

The byteordering is LITTLE ENDIAN, which menas the first bit has the highest 
rank. For more infomration on endiannes see this link:

https://en.wikipedia.org/wiki/Endianness

# Protocol

The protocol is structured, that the very first byte sent is the total length of
the frame, and the last byte sent is required to be 0xFF. A frame, where the
length byte, is not the index + 1 of a 0xFF byte, shoulde be considered invalid
and should be discarded. The length is followed by an address.
Afterwards an action id is sent. Everything after the action id is the action's
payload. The payload may contain any data, (except for the 0xFF byte, which 
should only be sent once in an entire frame)but the total length of the frame is
required to be less than 255 bytes in order to prevent the length byte to 
overflow, in which case the receiving party may trat any incoming data as an 
invalid frame. The ONLY case where a 0xFF byte is valid is inside of the 
checksum.

# Speed

The protocol may run on any layer of the osi layer model. The protocol itself
doesn't really care about speed, but the protocol is mostly run on layer 2, so
the hardware has to set the speed. This is for UART:

The speed has been chosen to be compatiable with most Integrated circuits,
operating systems and to ensure no data is lost in transmission: 38400 Baud or
38400 bits/second. Please don't increase this speed. Only 1 stop bit is
required and 8 data bits are transferred at once. As the protocol brings it's
own checksum with it, there is no parity bit used. Please see the src/serial.c
file of the microcontroller repository, or the src/ecproto.c file of the host
repostiroy for further details.

# Example frames

Here are a few example frames:

1. The master initializing a enumeration action:
	0x06 0x00 0x03 0x3B 0x40 0xFF

2. The slave responding to a enumeration action whilest beeing the last slave
	0x06 0x00 0x03 0x00 0x00 0x2E 0xFF

3. The master requesting device 0 register A pin 7
	0x06 0x00 0x6 0x00 0x41 0x07 0xD3 0xB1 0xFF

## Zero padding

The parameters may be zero padded in order to avoid the 0xff byte in the
checksum. Zero padding is required to be supported on any action, independent of
any hardware changes.

## Sending and receiving

The EC-Proto has an action called "request to send", which may only be SENT from
a device closer to the master device to the device further away from the master.
The receiving device is required to respond with the appropriate answer. Please
consult the section on ids in order to understand the messages sent better.

## Checksum
The last 2 bytes before the 0xFF byte contain a crc-16 checksum. The crc version
used should match the one of the CRC-16-IBM version and is also used in the
modbus protocol and the usb protocol. The whole frame before the checksum is
used as basisi for cecksum calculation. In case the checksum only consists of
0x00 bytes, the responding side was too lazy to implement checksums, in which
case a checksum comparison should be ommitted. The checksum may contain a 0xFF
byte, but it is discouraged to NOT implement zero padding.

## Action-IDs

The following is a declaration of the existant action ids:

0. The INIT statement.  ID 0x00

This action id may ONLY be sent to initialize a connection. It may never be used
to reactivate connection whatsoever. Any party may send this message at any time
in case a device-crash happens, or something else, and the device has to be
restarted. The device at the other end should now notify the device of it's
configuration and purpose. No payload may be sent. The master is NOT required to
send this message at init, neither is the slave. No answer on this message is
required from any side.

1. The request to send from the master. ID 0x01

This is sent periodically by the device
closer to the host. It indicates, that the device on the other side may
speak now, and has the chance to send all of it's messages. The device on the
other side is now REQUIRED to respond. A timeout may be set by the master
device in order to avoid errors. It's payload is empty, and it's reply consists
of a message with the 0x02 or SEND NOTIFY message id.

2. Send notify. ID 0x02

This is the intended response on a request to send or 0x01 action. It is sent
by the slave device ONLY, because only a slave has to notify before sending.
It's payload contains 1 byte indicating the amount of frames following this up.
As there is no acknowledgement for this from the master, this has to be followed
up by the specified amount of messages from the slave device.

3. Enumerate. ID 0x03

This command is issued at startup or at connection initialisation. The first
parameter is sent by the master without a payload. The slaves are required to
respond with their ids in incremental order. The first device to respond has
the id 0, the second 1, and so on. The device should all take a look at the
previous frame, look at the previous frame sent, and if its own device number is
the next one up, it should respond. The last device has to know, wether it is
the last one, and should send it's own address as payload. Via that means, the
bus is considred enumerated and normal operation is resumed.

4. RESERVED. ID 0x04

This message is reserved for future use. 

5. define port. ID 0x05

This defines a port to be an output, or an input. A port is a physical port
on the slave. The payload consists of three bytes. The first one is the register
of the slave, a character describing it. For example on an atmel mega chip,
there is a register PORTA, and the first byte to address it would be 0x41 or
'A', if the board used is not from atmel, anything else may specifiy which
register to use, as long as it doesn't conflict anythig in the protocol. The
second parameter is the 'bit' of the register. The third parameter specifies
wheter the port should be an "INPUT" or "OUTPUT" by using
0x00 and 0x01 respectively as byte. It's response should be a frame with
the same action id, and a boolean indicating success as payload. By default,
everything is an ouput, and it's state is low. In case a invalid payload is
received, it is converted to a boolean, which makes everything apart from 0x00
a 0x01 byte. 

6. get port. ID 0x06

This get's the current state of a port. It takes two parameters, the register 
and the bit, each represented by one byte. It's reply consists of a frame with
the same action id and the current state of the requested port as a boolean.

7. write port. ID 0x07

This writes the given boolean to the output, OR set's the port to be pulled low
in case it is NOT an output, but an input. It's response should be a frame with
the same action_id and a boolean indicating success. It needs 3 params, the
firsdt is the id of the register, the second one is the bit and the third one
is a boolean indicating the state of the port.

8. ERROR. ID 0x08

This is replied on error. It may contain ANY DATA as payload, but it is
suggested to be a string. The current operation is considered FAILED and the
master should act accordingly. No messages should follow up this message, and
it may only be sent by a slave.

9. Request gpio register count ID 0x09

Request the amount of gpio registers configured. It's reply consists of a
message with the same id and 1 parameter, the amount of registers. 

10. Request all gpio registers ID 0x0A

Request a list of gpio register IDs. The master may send this request. It's
payload is empty. The slave should respond with a message with the same id and
with the id-list as payload. A null-termination is not required.

11. Request gpio pin state 0x0B

Request wther the requested gpio pin is enabled or not. This action takes
2 parameters: the register id, and the bit. It returns a frame with the same
id and 3 paramters: the register id, the bit, and the pin state. The pin state
should be 0 if the pin is disabled, and 1 if the pin is enabled.

12. Print string to secondary connection 0x0C

Print some string to a secondary serial connection. Where and what the secondary 
connection is is implementaion defined. It may be anything. It takes as a
parameter a null terminated string, and transfers it to the secondary 
connection. The NULL-Terminator is NOT transmitted. Its response consists of a
frame with the same id and a boolean indicating success as a payload.

# ERRATA

An errat, or things known to be wrong or have gone wrong in the past.

## The baud rate problem

Well as you maynow see in the documentation, the default baud rate for the ecp-
bus is 38400 baud. It was previously set to 115200 baud... Then i had a glimpse
at the atmel datasheet forthe atmega 2560.  There is a table in the usart 
section of the datasheet of the atmel, and found out, that eith a 16MHz 
oszillator the error on transmission is ~ -3%, which makes the uart connection
drop some frames. The baud rate was then shrunk to 
