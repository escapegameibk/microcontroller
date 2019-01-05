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

#define ECPROTO_OVERHEAD 6

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int parse_ecp_msg(const uint8_t* msg);

int print_ecp_error(char* string);
int print_ecp_pin_update(char reg_id, uint8_t bit_id, uint8_t target);
int print_success_reply(uint8_t action_id, bool success);
#ifdef ANALOG
int print_adc();
#endif /* ANALOG */
int print_ecp_msg(uint8_t action_id, uint8_t* payload, size_t payload_length);

uint16_t ibm_crc(const uint8_t* data, size_t len);
int process_updates();

#define INIT_ACTION            0x00
#define REQ_SEND               0x01 
#define SEND_NOTIFY            0x02
#define ENUMERATEACTION        0x03
#define REMOTE_COMMAND         0x04
#define DEFINE_PORT_ACTION     0x05
#define GET_PORT_ACTION        0x06
#define WRITE_PORT_ACTION      0x07
#define ERROR_ACTION           0x08
#define REGISTER_COUNT         0x09
#define REGISTER_LIST          0x0A
#define	PIN_ENABLED            0x0B
#define SECONDARY_PRINT        0x0C
#define ADC_GET                0x0D
#define ADC_GET2               0x0F
#define EXT_DEV_REG            0x10
#define EXT_DEV_INT            0x11

#define ECP_LEN_IDX 0
#define ECP_ADDR_IDX 1
#define ECP_ID_IDX 2
#define ECP_PAYLOAD_IDX 3

#endif /* ECPROTO_H */

