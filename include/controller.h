/* controller for the hardware attached to the atmega and other stuff
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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stddef.h>
#include <stdint.h>

int controller_init();
int execute_command(char* cmd);
int io_patrol();
int update_io(char REG, int bit, int state);
int process_update(char REG, volatile uint8_t* regp_new, uint8_t* regp_old);
int register_dump();

/* A save state of all registers, which get updated in sync with the host. */
uint8_t PINA_o, PINB_o, PINC_o, PIND_o, PINE_o, PINF_o, PING_o, PINH_o, PINJ_o,
       PINK_o, PINL_o;


#endif
