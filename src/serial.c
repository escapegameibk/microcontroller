/* serial interface for communication with a host

 * Copyright © 2018 tyrolyean
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "serial.h"
#include "ecproto.h"

#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
        
uint8_t recv_buf_master[BUFLEN_UART];
	
bool command_received = false;
uint8_t recv_crsr_master = 0;

void serial_init(){

	uart_init_master();
#ifdef UART_SECONDARY

	uart_init_2();
#endif /* UART_SECONDARY */
        memset(recv_buf_master, 0, BUFLEN_UART);

        return;
}
/*
 * the interrupt service routine for the uart controller
 * it ready 1 byte and saves it at the end of the buffer. On revceive
 * completion, the command received flag is set to true. 
 */
ISR(USART0_RX_vect){
	
	recv_buf_master[recv_crsr_master++] = UDR0;
	
	if(recv_buf_master[recv_crsr_master - 1] == CMD_DELIMITER && 
		recv_buf_master[ECP_LEN_IDX] <= recv_crsr_master + 1){
		recv_crsr_master = 0;
		command_received = true;
	}

}

int write_frame_to_master(const uint8_t* frame){
	
	for(uint8_t i = 0; i < frame[0]; i++){
		
		while( !(UCSR0A & (1<<UDRE0)) ); /* Wait for UDR to get ready */
		UDR0 = frame[i];                       /* send */
	}

	return 0;

}

void uart_init_master(){

#if 0
	/* This is the original version of the algorithm used to calculate
	 * the uart baud rate register value by goliath. It was not able to
	 * handle higher baud rates. */
	uint16_t ubrr = (uint16_t) ( (uint32_t)F_CPU/(16*UART_BAUD_RATE) - 1 );
#else
	/* This was copied from the source code of the arduino avr core. Please
	 * take a look ath their source for more information:
	 * https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/HardwareSerial.cpp
	 * Line: 120.
	 * Date: 2018/09/08
	 */
	uint16_t ubrr = (uint16_t) (((uint32_t) F_CPU / 8 / UART_BAUD_RATE - 1)
		/ 2 );
#endif

        /* Set the baud rate */
        UBRR0H = (unsigned char)(ubrr>>8);
        UBRR0L = (unsigned char) ubrr;
 
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1 << RXCIE0);                /* Enable receive and transmit */
        UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* 8 data bits, 1 stop bit */

        return;
}

#ifdef UART_SECONDARY
int write_string_to_secondary(const char* str){
	
	for(uint8_t i = 0; str[i] != '\0'; i++){
		
		while( !(UCSR2A & (1<<UDRE2)) ); /* Wait for UDR to get ready */
		UDR2 = str[i];			/* send */
	}

	return 0;

}

void uart_init_2(){

#if 0
	/* This is the original version of the algorithm used to calculate
	 * the uart baud rate register value by goliath. It was not able to
	 * handle higher baud rates. */
	uint16_t ubrr = (uint16_t) ( (uint32_t)F_CPU/(16*
		UART_SECONDARY_BAUD_RATE) - 1 );
#else
	/* This was copied from the source code of the arduino avr core. Please
	 * take a look ath their source for more information:
	 * https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/HardwareSerial.cpp
	 * Line: 120.
	 * Date: 2018/09/08
	 */

	uint16_t ubrr = (uint16_t) (((uint32_t) F_CPU / 8 / 
		UART_SECONDARY_BAUD_RATE - 1) / 2 );

#endif

        /* Set the baud rate */
        UBRR2H = (unsigned char)(ubrr>>8);
        UBRR2L = (unsigned char) ubrr;
 
	UCSR2B = (1<<RXEN2) | (1<<TXEN2) | (1 << RXCIE2);                /* Enable receive and transmit */
        UCSR2C = (1<<UCSZ21) | (1<<UCSZ20); /* 8 data bits, 1 stop bit */

        return;
}

#endif /* UART_SECONDARY */
