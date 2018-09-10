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

#include "ecproto.h"
#include "serial.h"
#include "port.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int parse_ecp_msg(const uint8_t* msg){
	
	if(msg[msg[0] - 1] != 0xFF){
		/* It doesn't matter, whether the checksums are right or not,
		 * this shouldn't have happened.
		 */
		 print_ecp_error("invld frme len");
		 char* str = malloc(30);
		 sprintf(str, "%i", msg[0]);
		 print_ecp_error(str);
		 free(str);
		 return -1;
	}

	/* Perced to checksum calculation */
	uint16_t crc_is = ((msg[msg[0] - 3] & 0xFF) << 8 ) | (msg[msg[0] - 2] 
		& 0xFF);
	uint16_t crc_should = ibm_crc(msg, msg[0] - 3);
	if(crc_is != crc_should){
		print_ecp_error("CRC msmtch");
		return -2;
	}
	/* By this point we should have gotten a valid frame. All checksums are
	 * valid and the frame should have a correct length. Now, let's start
	 * validating it's logic. */
	switch(msg[1]){
		
		case 0:
			/* I don't know why i got this message, but it's not my
			 * purpose to respond to it. The master will probably
			 * notify me of my purpose anyway.
			 */
			break;
		case 1:

			/* I have to send now all update messages that i have.
			 * I should first reply with a send notify to my master,
			 * so that he knows what's going to happen. I don't want
			 * my master to loose trust in me.
			 */
			 return process_updates();
			 break; /* Just in case :D */
		case 9:
			/* Request the gpio register count. */
			return print_ecp_msg(9, &gpio_register_cnt, 
				sizeof(uint8_t));
			break;
	}

	return 0;
	
}

/* Please use this function as RARELY as possible. The AVR copies all of it's
 * strings into ram during startup. Due to this, it is NOT recommended to
 * use strings in any way. */
int print_ecp_error(char* string){
	return print_ecp_msg(ERROR_ACTION, (uint8_t*)string, strlen(string) + 
		1);
}

int print_ecp_pin_update(char reg_id, uint8_t bit_id, bool target){
	uint8_t pay[] = {(uint8_t)reg_id, bit_id, target};
	return print_ecp_msg(0x06, pay, sizeof(pay));
}

int print_ecp_msg(uint8_t action_id, uint8_t* payload, size_t payload_length){

	static uint8_t frame[255];
	
	frame[0] = ECPROTO_OVERHEAD + payload_length;
	frame[1] = action_id;
	memcpy(&frame[2], payload, payload_length);
	uint16_t crc = ibm_crc(frame,frame[0] - 3);
	frame[frame[0] - 3] = (crc >> 8) & 0xFF;
	frame[frame[0] - 2] = crc & 0xFF;
	frame[frame[0] - 1] = 0xFF;

	return write_frame_to_master(frame);
}

uint16_t ibm_crc(const uint8_t* data, size_t len){

	uint16_t crc = 0xFFFF;
  
	for(size_t pos = 0; pos < len; pos++){
		crc ^= (uint16_t)data[pos];
  
		for(size_t i = 8; i != 0; i--){
			if((crc & 0x0001) != 0){
				crc >>= 1;
				crc ^= 0xA001;
			}else{
				crc >>= 1;
			}
		}
	}

	return crc;  
}

int process_updates(){
	if(update_pins() < 0){
		print_ecp_error("updt pns fail");
	}
	uint8_t updatecnt = 0xFF & get_port_update_count();
	print_ecp_msg(2, &updatecnt, sizeof(uint8_t));
	
	return send_port_updates();
}

