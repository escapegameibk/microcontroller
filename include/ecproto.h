/* EC-Proto controller for the escape game system's microcontroller part.
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

#ifndef ECPROTO_H
#define ECPROTO_H

/* The protocol overhead includes everything, but the payload length, and
 * therefore is length, action, checksum and termination byte.
 */

#define ECPROTO_OVERHEAD 5

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int parse_ecp_msg(const uint8_t* msg);

int print_ecp_error(char* string);
int print_ecp_pin_update(char reg_id, uint8_t bit_id, uint8_t target);
int print_success_reply(uint8_t action_id, bool success);
int print_ecp_msg(uint8_t action_id, uint8_t* payload, size_t payload_length);

uint16_t ibm_crc(const uint8_t* data, size_t len);
int process_updates();


#define INIT_ACTION 0
#define REQ_SEND 1
#define SEND_NTOIFY 2
#define ENUMERATEACTION 3
#define REMOTE_COMMAND 4
#define DEFINE_PORT_ACTION 5
#define GET_PORT_ACTION 6
#define WRITE_PORT_ACTION 7
#define ERROR_ACTION 8

#endif /* ECPROTO_H */

