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

#ifndef SERIAL_H
#define SERIAL_H

#include "general.h"
#include "ecproto.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <stdbool.h>

#define CMD_DELIMITER 0xFF /* < END OF FRAME */

#define BUFLEN_UART 256

extern uint8_t recv_buf_master[];
uint8_t recv_crsr_master;

void serial_init();
bool command_received;
extern unsigned long long int master_serial_timeout;
#define MASTER_TIMEOUT_THRESHOLD 106250 /* max transmission length */

void clear_master_buffer();

int write_frame_to_master(const uint8_t* frame);

void uart_init_master();

#ifdef UART_SECONDARY

#define UART_SECONDARY_BAUD_RATE 9600
/* Only enabled if the secondary UART connection is enabled. Otherwise the UART2
 * pins are used as regular GPIO pins, and all ECP frames with a UART id are 
 * beeing ommitted
 */
int write_string_to_secondary(const char* str);

void uart_init_2();

#endif /* UART_SECONDARY */

static inline void consume_uart_master_byte(uint8_t byte){

	if(recv_crsr_master == 0xFF && byte == 0xFF){
		clear_master_buffer();
		return;
	}else if(recv_crsr_master == 0xFF){
		return;
	}

	recv_buf_master[recv_crsr_master++] = byte;
	if(recv_buf_master[recv_crsr_master - 1] == CMD_DELIMITER && 
		recv_buf_master[ECP_LEN_IDX] <= recv_crsr_master + 1){
		
		recv_crsr_master = 0;
		command_received = true;
	}

	master_serial_timeout = 0;

	return;
}

#endif
