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
#include "port.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int routine();
int main(){
        
	cli();

        /* initialize the uart connection to the controller */
	init_ports();
        serial_init();
	sei();
        while(1){
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
                sei(); /* < Enable interrupts */
		command_received = false;
        }
        return 0;
}
