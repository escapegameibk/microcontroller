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

#include "uart_tools.h"
#include "serial.h"
#include "controller.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int routine();
int main(int argc, const char *argv[]){
        
        /* initialize the uart connection to the controller */
        uart_init(DEBUG);
        controller_init();
        serial_init();
#if DEBUG
        printf("0%cstartup finished%c",PARAM_DELIMITER,CMD_DELIMITER);
#endif

        while(1){
                if(routine() < 0){
                        printf("1%cfailed to routine!%c",PARAM_DELIMITER, 
                                        CMD_DELIMITER);
                }
        }

        return 0;
}


int routine(){

        if(command_ready()){
                /* if a command has been read from the server, read it back
                 * and clear it afterwards*/
               

                /* pretend to be atomic. if we would have gotten an interrupt
                 * in the meantime, we will get an invalid command the next 
                 * time we read.
                 */
                cli(); /* < Disable interrupts */
                char* buftmp = malloc(strlen(recv_buf));
                if(buftmp == NULL){
#if DEBUG
                        printf("0%cfailed to allocate %i bytes for buftmp%c",
                                        PARAM_DELIMITER, strlen(recv_buf),
                                        CMD_DELIMITER);
#endif
                        sei();
                        return -1;
                }

                strcpy(buftmp,recv_buf);
                recv_len = 0;
                memset(recv_buf,0,BUFLEN_UART);
                sei(); /* < Enable interrupts */
                execute_command(buftmp);
                free(buftmp);
        }
        
        io_patrol();
        

        return 0;
}
