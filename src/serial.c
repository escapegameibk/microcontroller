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
#include "uart_tools.h"

#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>

int serial_init(){
        
        cli();

        recv_buf = malloc(BUFLEN_UART);
#if DEBUG
        fprintf(&uart_stdout,"0%cserial init%c",PARAM_DELIMITER,CMD_DELIMITER);
#endif
        recv_len = 0;        
        memset(recv_buf, 0, BUFLEN_UART);

        sei();
        UCSRB |= (1<<RXCIE0);
        
        return 0;
}
/*
 * the interrupt service routine for the uart controller
 * it ready 1 byte and saves it at the end of the buffer. if the buffer
 * contains a valid 
 */
ISR(USART0_RX_vect){
        
        /* start from the beginning if a valid command has been written, or
         * if the buffer is full
         */         
        if(recv_buf[recv_len-1] == CMD_DELIMITER ){
                recv_len = 0;
        }

        recv_buf[recv_len] = fgetc(&uart_stdin);
        recv_buf[++recv_len] = 0; 
}

int command_ready(){
        return (recv_buf[recv_len-1] == CMD_DELIMITER);
}

