/* Controller software for the arduino for the painter at the escape game ibk
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h>

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));

void get_mcusr(void)
{
  mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}

bool initialized = false;

int routine();
int main(){

	/* Disable interrupts during initialisation phase */	
	cli();

	/* initialize the uart connection to the controller */
        serial_init();
	
	sei();
	/* Enable the hardware watchdog. In case the microcontroller fails to 
	 * finish it's task within the specified time, the watchdog will reset
	 * the atmel cookie.
	 */
	wdt_enable(WDTO_4S);
	
        
	for(;;){
		wdt_reset();
                if(routine() < 0){
                }
        }

        return 0;
}

int routine(){

        if(command_received){
                /* if a command has been read from the master, read it back
                 * and clear it afterwards*/

                /* pretend to be atomic. we don't want to receive anything
		 * whilest processing the input.
                 */
                cli(); /* < Disable interrupts */
		parse_ecp_msg(recv_buf_master);
		memset(recv_buf_master, 0, BUFLEN_UART);
	        sei(); /* < Enable interrupts */
		command_received = false;
		master_serial_timeout = 0;

        }else{
		master_serial_timeout++;
		if(master_serial_timeout > MASTER_TIMEOUT_THRESHOLD){
			master_serial_timeout = 0;
			clear_master_buffer();
		}
		
	}

        return 0;
}
