%EC-Proto
# About

After doing some research on the stuff, the old protocol seemed unusable and a
bit idiotic. It was deemed nescessary to rebuild it from scratch. The protocol
was designed to be as flexile as possible, whilest keeping it simple stupid. The
result was the EC-proto, which stands for escape the protocol, which is a 
master-slave based protocol.

# Addressing

THe protocol is a peer-to-peer protocol, and therefore is intentionally not
able to address other than by the port pluged in, The protocol is however
capable of using multiple hosts as "jump hosts". The documentation on this is a 
TODO. The basic principle is to use the distance in jump-hosts as the basis to
address a host. 

# Protocol

The protocol is structured, that the very first byte sent is the total length of
the frame - 1, and the last byte sent is required to be 0xFF. A frame, where the
length byte, is not the index + 1 of a 0xFF byte, shoulde be considered invalid
and should be discarded. The length is followed by an action id. Everything
after the action id is the action's payload. The payload may contain any data,
(except for the 0xFF byte, which should only be sent once in an entire frame)
but the total length of the frame is required to be less than 255 bytes in
order to prevent the length byte to overflow, in which case the receiving party
may trat any incoming data as an invalid frame. The ONLY case where a 0xFF byte
is valid is inside of the checksum

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
0x00 bytes, the responding side was too laszy to implement checksums, in which
case a checksum comparison should be ommitted. The checksum may contain a 0xFF
byte.

## Action-IDs

The following is a declaration of the existant action ids:

0. The INIT statement.  ID 0x00

This action id may ONLY be sent to initialize a connection. It may never be used
to reactivate connection whatsoever. Any party may send this message at any time
in case a device-crash happens, or something else, and the device has to be
restarted. The device at the other end should now notify the device of it's
configuration and purpose. No payload may be sent.

1. The request to send from the master. ID 0x01

This is sent periodically by the device
closer to the host. It indicates, that the device on the other side may
speak now, and has the chance to send all of it's messages. The device on the
other side is now REQUIRED to respond. A timeout may be set by the master
device in order to avoid errors. It's payload is empty.

2. Send notify. ID 0x02

This is the intended response on a request to send or 0x01 action. It is sent
by the slave device ONLY, because only a slave has to notify before sending.
It's payload contains 1 byte indicating the amount of frames following this up.
As there is no acknowledgement for this from the master, this has to be followed
up by the specified amount of messages from the slave device. Any slave device
connected in between the slave beeing a relay for this message, should also
relay the specified amount of messages as defined in the payload following this
message.

3. Enumerate. ID 0x03

The enumerate command is issued at startup. It is used to enumerate the
connected devices. Every device should respond towards the host with 0x03 and
the current id as 8 bit integer as payload. In case another device is connected,
a slave should also respond to the devices with it's current id incremented by
one, or the id of the following host. By this way, the amount of jump-hosts to
the master is known. In case the device doesn't have another slave host 
connected, it is required to respond with the enumerate action, and it's id and
another 0x00 byte as payload. Via that means, the connection is considered
enumerated, all devices have responded , and all connections are waiting for
another command from the master. The payload from the master should be set to 
0x00, which will also be the id of the first device, event though, ids are not
used in that sense in a peer to peer connection, and as it isn't nescessary to
enumerate the bus.

4. remote command. ID 0x04

In case multiple hosts are connected, a 0x04 command may be sent, with the
remote id as it's first payload, and afterwards an action and a payload. The
slaves relaying the command may take a look at it's conetent, and if a slave
has announced to send multiple other commands, slaves should be prepared to
forward multiple mesages. If the id is the very next slave, the slave should
create a new frame from the payload of the received message, and transmit it
towards it's destination. The reply(s) by the destination shoud be encapsulated
by the same slave who did the deencapsulattion, which means he should take the
id, and the payload, create another 0x04 frame, and send it towards the master.

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

