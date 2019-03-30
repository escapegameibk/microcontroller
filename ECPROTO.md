---
Author: tyrolyean
Mainfont: Arial
Title: Escape The Protocol
geometry: margin=2cm
---
# About

After doing some research on the stuff, the old protocol seemed unusable and a
bit idiotic. It was deemed nescessary to rebuild it from scratch. The protocol
was designed to be as flexile as possible, whilest keeping it simple stupid. The
result was the EC-proto, which stands for escape the protocol, which is a 
master-slave based protocol. It may be driven over any layer 2 connection.

# Addressing

The protocol is very well able to address a host, and therefore no buffering
or any such thing is required. The address is always the SECOND byte 
transferred.

## Byte ordering

The byteordering is BIG ENDIAN, which means the first byte has the highest 
rank. For more infomration on endiannes see this link:

https://en.wikipedia.org/wiki/Endianness

# Protocol

The protocol is structured, that the very first byte sent is the total length of
the frame, and the last byte sent is required to be 0xFF. A frame, where the
length byte, is not the index - 1 of a 0xFF byte, shoulde be considered invalid
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

## Zero padding

The parameters may be zero padded in order to avoid the 0xff byte in the
checksum. Zero padding is required to be supported on any action, independent of
any hardware changes.

## Notes on command receiption

As any command is required to be answered at any time, it is possible to answer
a received frame with ANY frame id. It is also possible to, for example, misuse
the 0x02 frame id to send the master multiple frames ASAP, even though he has
requested something else. The only thing which needs to be done on reception of
a frame by the master is, wether he can successfully pasrse the frame, and
wether he was successful at parsing it.

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

### 0. The INIT statement.  ID 0x00

This action id may ONLY be sent to initialize a connection. It may never be used
to reactivate connection whatsoever. Any party may send this message at any time
in case a device-crash happens, or something else, and the device has to be
restarted. The device at the other end should now notify the device of it's
configuration and purpose. No payload may be sent. The master is NOT required to
send this message at init, neither is the slave. No answer on this message is
required from any side.

### 1. The request to send from the master. ID 0x01

This is sent periodically by the device
closer to the host. It indicates, that the device on the other side may
speak now, and has the chance to send all of it's messages. The device on the
other side is now REQUIRED to respond. A timeout may be set by the master
device in order to avoid errors. It's payload is empty, and it's reply consists
of a message with the 0x02 or SEND NOTIFY message id.

### 2. Send notify. ID 0x02

This is the intended response on a request to send or 0x01 action. It is sent
by the slave device ONLY, because only a slave has to notify before sending.
It's payload contains 1 byte indicating the amount of frames following this up.
As there is no acknowledgement for this from the master, this has to be followed
up by the specified amount of messages from the slave device.

### 3. RESERVED. ID 0x03

This message is reserved for future use.

### 4. RESERVED. ID 0x04

This message is reserved for future use. 

### 5. define port. ID 0x05

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
everything is an ouput, and it's state is low.

### 6. get port. ID 0x06

This get's the current state of a port. It takes two parameters, the register 
and the bit, each represented by one byte. It's reply consists of a frame with
the same action id and the current state of the requested port as a boolean.

### 7. write port. ID 0x07

This writes the given boolean to the output, OR set's the port to be pulled low
in case it is NOT an output, but an input. It's response should be a frame with
the same action_id and a boolean indicating success. It needs 3 params, the
firsdt is the id of the register, the second one is the bit and the third one
is a boolean indicating the state of the port.

### 8. ERROR. ID 0x08

This is replied on error. It may contain ANY DATA as payload, but it is
suggested to be a string. The current operation is considered FAILED and the
master should act accordingly. No messages should follow up this message, and
it may only be sent by a slave.

### 9. Request gpio register count ID 0x09

Request the amount of gpio registers configured. It's reply consists of a
message with the same id and 1 parameter, the amount of registers. 

### 10. Request all gpio registers ID 0x0A

Request a list of gpio register IDs. The master may send this request. It's
payload is empty. The slave should respond with a message with the same id and
with the id-list as payload. A null-termination is not required.

### 11. Request gpio pin state ID 0x0B

Request wther the requested gpio pin is enabled or not. This action takes
2 parameters: the register id, and the bit. It returns a frame with the same
id and 3 paramters: the register id, the bit, and the pin state. The pin state
should be 0 if the pin is disabled, and 1 if the pin is enabled.

### 12. Print string to secondary connection ID 0x0C

Print some string to a secondary serial connection. Where and what the secondary 
connection is is implementaion defined. It may be anything. It takes as a
parameter a null terminated string, and transfers it to the secondary 
connection. The NULL-Terminator is NOT transmitted. Its response consists of a
frame with the same id and a boolean indicating success as a payload.

### 13. Get analog input ID 0x0D **DEPRECATED INTERMEDIATE**

Request an analog value from a slave. What that analog value may be is not the
concern of the master, though the masterm ay have influenced this value by some
other means. This request requires no parameters. Its response consists of a 
frame with the same id and a 8-bit value of the analog-digital converter, where
255 is the highest, and 0 the lowest value.

**As of 2018-12 this command is deprecated. Please use commands 0x0F for any new
features**

### 14. RESERVED ID 0x0E

RESERVED FOR FUTURE USE

### 15. Get analog input ID 0x0F

Reads the analog input value from the specified channel. The 1 byte payload in
a master's request specifies the targeted analog input channel. A slave's
reply has the channel as 1st byte of the payload, and the adc value as 2 byte
value as 2nd and 3rd byte respectively.

### 16. Get device capabilities ID 0x10

The 0x10 command may be used on any device to retrieve it's capablilities.
A master request may use 0x10 as action id and an empty payload to
request a list of special devices from a slave. A response must start with the
amount of special device kinds as the very first byte of the payload, followed
by a list of the specified amount of capability IDs. A NULL-Termination is
not required. For a list of device capablility IDs please see the device-
capablilities section at the end of this document.

### 17. Special device interaction ID 0x11

This function is used for any kind of special device interaction. Please see the
section on special devices at the end of this document.

### 18. PWM SET ID 0x12

This action is used to set the specified PWM output to the desired value. A
request from the master must have 3 bytes as payload: the first one beeing a
timer id, the second one beeing the output id, and the third one beeing the
pwm value between 0 and 255. What combination is what output is microcontroller
specific and has to be specified by the manufacturer of the device and the
author of the code, but are required to start at 0. A reply has 1 byte payload
containing a boolean value which indicates success. The value 0 is special and
is used to disable PWM on that output. Enabled by the PWM extension

### 19. Get GPIO Registers 0x13

This action is used to get the specified GPIO Registers. A request with this
action requires only 1 parameter, the GPIO register ID. A reply with this ID
has 4 parameters. The first one is the GPIO register ID, the second one is the
GPIO Data Direction Register, the third one is the GPIO PORT Register, and the
fourth one is the GPIO PIN register. Enabled by the faster GPIO extension.

### 20. Get GPIO Registers 0x14

This action is used to set the specified GPIO registers to the desired values.
A request contains 3 parameters. The first one beeing the register ID, the
second one beeing the GPIO DDIR registerand the third one the GPIO PORT 
register. Enabled by the faster GPIO extension.

### 21. Get disabled pins 0x15

This action is used to get all disabled pins. A request with this id doesn't
contain any parameters. A reply with this ID is INVALID. A response should
announce the amount of pin disables via the send notify action. Enabled by the
faster GPIO extension.


# Special Devices / Device Capabilities

Whilest developing more and more stuff it became apparent to me, that the 
microcontroller code was becomeing too complex. To keep it backwards
compatiable the devices basically needed to be able to do basically anything,
and I was planning to enable any device to perform any task, which was pretty
dumb. So it was decided, that it was best to separate code for special tasks
into different code bases, so that it stays maintainable and doesn't look too
dumb.5

## device capablilities

Some devices have a more special use-case than just plainely GPIO, for example
controlling multiple SPI-slaves, uart device controllers, etc. Therefore
special devices hae been introduced to the protocol in order to be flexible
enough to handle the very strange situations in an escape room and the
corresponding strange hardware. It has therefore been decided to implement a
way to request what an ECP device is capable of, and a way to handle special
situations.

#### Special device IDs

The following device capablility IDs have been specified thus far:

0. GPIO:
	An ID of the numeric value 0 specifies, that a device is GPIO capable.
	It is not nescessary for a device to have GPIO capability.

1. OLD ANALOG:
	An ID of the numeric value 1 specifies that the device is compatiable 
	with the deprecated analog device system. Please don't use this in any 
	new installation. This may be assumed to exist by a master. Please 
	REALLY don't use this anymore!

2. NEW ANALOG:
	An ID of the numeric value 2 specifies that the device is capable of the
	new analog input system. Please use this instead of the deprecated
	system!

3. MFRC522:
	An ID with the numeric value 3 represents an MFRC522 action. MFRC522
	capable devices usually have no, or limited amounts of GPIO pins.
	MFRC522 may lock any GPIO pins needed for SPI communication.
	For further details on how to handle MFRC522 capable devices see
	the following paragraph on the 0x11 command.

4. PWM:
	Specifies, that the device supports PWM output.
5. FASTER GPIO:
	Specifies, that the device supports the faster GPIO access
	routines.

#### Action 0x11

Commands with the action ID 0x11 are reserved for communication regarding
special device actions. The entire payload may be imagined as if it was "passed
on to a submodule an never looked at during transport", by which I want to tell
you that the payload may contain anything and is entirely special device type
dependant. A command regarding action 0x11 may ALWAYS contain a payload starting
with a capablility ID which the device previously has been announceing
through the 0x10 command that it is capable of using it.

This command is meant to **ONLY** be used for communication regarding some
special device which would not be used normally in an escape room. **Any**
communication regarding more frequently used functions is to be given a
real action-ID.

Communication
has been specified for the following special devices:

##### GPIO:
**NO ACTION MAY BE PERFORMED FOR THE GPIO SUBMODULE! PLEASE USE REGULAR 
ACTIONS!**

##### OLD ANALOG:
**NO ACTION MAY BE PERFORMED FOR THIS SUBMODULE! PLEASE USE REGULAR ACTIONS!**

##### NEW ANALOG:
**NO ACTION MAY BE PERFORMED FOR THIS SUBMODULE! PLEASE USE REGULAR ACTIONS!**

##### MFRC522:

MFRC522 contains after the capablility ID, a so called sub-action id, which 
is used to indicate to the slave what the master wants and is exactly 1 octet 
in lenght. As with a regular command, every request by a master has to be 
answered with at least one frame. The following sub-commands have to be 
implemented:

0. How many:

	A sub-action ID with the numeric value 0 indicates a request of the
	master how many MFRC522 devices are connected to the slave. A response
	must be a frame with the same sub-action as the sender, but containing
	an extra byte in the payload containing the number of connected
	devices. A non-responsive device may still be viewed as "connected".

1. What's there:

	A sub-action ID with the numeric value 1 indicates a request if a RFID-
	Tag is present at the given position. The position is the first octet to
	follow the sub-action ID, and in a request the only parameter. In a
	response it is followed by 1 octet containing a boolean value
	indicateing wether a RFID-tag is present or not and ALWAYS 5 octets
	containing the RFID-tag ID. If no rfid tag is present the ID may contain
	garbage. In case the device at the given position is not responding, the
	boolean indicating, wether a RFID-TAG is present should be set to false,
	and the 4 following octets, representing the tag-id, should be set to 
	**E R O R** in ASCII. In a numeric representation this would be 
	equivalent to 0x45524F52 or 1163022162 at base 16 and 10 respectively. 
	Implementation of this is not nescessary, but may help during 
	debugging.

##### PWM:
**NO ACTION MAY BE PERFORMED FOR THIS SUBMODULE! PLEASE USE REGULAR ACTIONS!**

##### FASTER GPIO:
**NO ACTION MAY BE PERFORMED FOR THIS SUBMODULE! PLEASE USE REGULAR ACTIONS!**

# ERRATA

An errat, or things known to be wrong or have gone wrong in the past.

## The baud rate problem

Well as you maynow see in the documentation, the default baud rate for the ecp-
bus is 38400 baud. It was previously set to 115200 baud... Then i had a glimpse
at the atmel datasheet forthe atmega 2560.  There is a table in the usart 
section of the datasheet of the atmel, and found out, that eith a 16MHz 
oszillator the error on transmission is ~ -3%, which makes the uart connection
drop some frames. The baud rate was then shrunk to 38400 baud in order to avoid
this issue. The connection has since had no problems.

## What connects where?

The initial microcontroller was the arduino mega, even though it was only used
as a breakout board for the underlying atmega2560. The ardino mega has UART as
well as an USB port. The USB port may be used in some cases, though there was a
problem: The device was reset on each open on the serial port. So if the host
crashes, the microcontroller resets. If that happens, we may have a scenario
where an entire game is ruined, and that is NOT acceptable. There is however a
solution to this problem. There is a small trace between two larger metal plates
labeled with RESET-EN. If that trace is cut, the atmega no longer receives a
reset signal from the USB-controller, thereby solving this problem.

## Analog value inversion? What?

Well that's a funny story, I was workign on a game called "The orphanage" and 
i was at implementing the ADC command, when I noticed something weired. The
ADC was throwing values in the range of 250-254 at me. The reason for that was,
that the Pin was configured as input (that was planned) and pulled high (that 
wasn't planned). When i disabled the pullup resistors, the ADC spit out nothing.
It never gave back anything. The board of the orphanage NEEDS the pullup
resistors to be activated.


