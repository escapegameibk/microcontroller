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
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stddef.h>

#define CMD_DELIMITER 0x0A /* < END OF TRANSMISSION */
#define PARAM_DELIMITER 0x3B /* < UNIT SEPARATOR */

#define BUFLEN_UART 256

size_t recv_len;
char *recv_buf;

int serial_init();
int command_ready();

#endif
