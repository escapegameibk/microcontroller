# About

This is the controller software for the arduino which is built into the games
at the escape game innsbruck and it is responsible for handling all internal
traffic and acting as an I/O to serial "adapter".

# Documentation

This file includes documentation for several things:

## ARDUINO - Raspberry Pi

The transmission from the Arduino to the Raspberry Pi is done via a UART
connection from the arduino to the raspberry pi. It's connected with 9600 BAUD
speed at the USART0 connection of the arduino and the gpio shield of the 
raspberry pi.

### PROTOCOL

For communication a special protocol is used, which is extremely simple.
Transmission occures in ASCII text, and NOT in binary. The protcol consists of
an action and several parameters. to speed up the protcol the process is
transmitted via ASCII representations of the number. I know this sounds idiotic
but it helps to speed up the isr.
#### Construction

A protocol frame is defined as an action separated from all other parameters
by the unit separator character (0x1F). The parameters are separated by the
same character and the command is finished with the end of transmission
character (0x04).

```---------------------------------------```<br/>
```| action | 0x1F | <parameters> | 0x04 |```<br/>
```---------------------------------------```<br/>

The action can be more than 1 byte and is NOT defined to be just 1 byte long.
Whilest in transmission NO NULL-terminated strings may be sent, however without
the null terminator, it is allowed to send strings, which must be terminated by
the 0x1f char.

#### States

States are transmitted as ASCII representations of unsigned integers.


As of now these states are defined:

- 0 : Debug output. Any participant can safely ignore anything.
- 1 : Error output. An error has occured and here's information.
- 2 : init. Information needed for communication initialisation
- 3 : update. update a state, register or something

The states are defined like this to make communication internally easier.
More states may be defined later, and any invalid action number may be
ignored.

##### 0/DEBUG

Debug statements may only be ignored, if the device is configured to be in
debug mode. Otherwise, it may send an error, as debug messages may not be
distinguishable from invalid receives. The device may not send any debug stuff,
if not configured to do so. Any content may follow a debug statement. Those are
a lot of mays...

##### 1/ERROR

Errors are sent if something invalid is received or may have been received.
The error statemnt has only 1 parameter which defines what error occured. It is
NOT permissible to send more than 1 parameter for whatever purpose!


The following error types exist:

- DEBUG: Sent if a debug message has been received, but the device is not
configured to be in debug mode.
- INVPARAM: Sent if an invalid parameter has been received.
- INVCM: Sent if an invalid command has been received.

##### 2/INIT

The init process of the protocol is defined as follows: the host sends the
controller a message without parameters, and the action set to be an INIT
message. the controller responds with a complete dump of all register states
it is allowed to display. This is the whole init process. A init command may be
sent spontaneously via the serial connection to the controller, due to a
reboot of the host. The controller is required to ALWAYS anser a init call.

##### 3/UPDATE

If a update is sent from the host to the controller, the latter should apply
the update immediately. An update-message consists of a register, a bit and a
value to update it to. NONE of the mentioned s REQUIRED to be a fixed size of
bytes. If, for example, the controller needs to write more than 1 bit into a
register, the command may contain some stupid value in the bit field, to mark
it as not-relevant. You may have to look into the actual code, which processes
update messages to fully understand it, but a basic frame containing a update
message looks like this:

```------------------------------------```<br/>
```| 3 | <REGISTER> | <BIT> | <VALUE> |```<br/>
```------------------------------------```<br/>

### ERRATA

 Strings containing (for whatever reason) the 0x04 character may not be sent by
 ANY part of the transmission and HAVE to be filtered out before the
 frame is assembled.
