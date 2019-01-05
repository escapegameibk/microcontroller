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
#include "general.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int parse_ecp_msg(const uint8_t* msg){
	
	if(msg[msg[ECP_LEN_IDX] - 1] != 0xFF){
		/* It doesn't matter, whether the checksums are right or not,
		 * this shouldn't have happened.
		 */
		 print_ecp_error("invld frme len");
		 return -1;
	}

	if(msg[ECP_ADDR_IDX] != ECP_DEVICE_ID && msg[ECP_ID_IDX] != 
		ENUMERATEACTION){
		/* None of my interest. */
		return 0;
	}

	/* Perced to checksum calculation */
	uint16_t crc_is = ((msg[msg[ECP_LEN_IDX] - 3] & 0xFF) << 8 ) | 
		(msg[msg[ECP_LEN_IDX] - 2] & 0xFF);
	uint16_t crc_should = ibm_crc(msg, msg[ECP_LEN_IDX] - 3);
	if(crc_is != crc_should){
		print_ecp_error("CRC msmtch");
		return -2;
	}
	/* By this point we should have gotten a valid frame. All checksums are
	 * valid and the frame should have a correct length. Now, let's start
	 * validating it's logic. */
	switch(msg[ECP_ID_IDX]){
		
		case INIT_ACTION:
			/* I don't know why i got this message, but it's not my
			 * purpose to respond to it. The master will probably
			 * notify me of my purpose anyway.
			 */
			break;
		case REQ_SEND:

			/* I have to send now all update messages that i have.
			 * I should first reply with a send notify to my master,
			 * so that he knows what's going to happen. I don't want
			 * my master to loose trust in me.
			 */
			 return process_updates();
			 break; /* Just in case :D */
		case SEND_NOTIFY:
			/* Ähhm i shouldn't receive that. This is only what i
			 * have to send my master in order to not disappoint
			 * him. Has my master disappointed me? What has
			 * happened, i don't understand. */
			 break;
		case DEFINE_PORT_ACTION:
			/* Defines a port direction / writes to the ddr */
			if(msg[ECP_LEN_IDX] <  3 + ECPROTO_OVERHEAD){
				print_ecp_error("2 few parms");
				return -4;
			}
			return print_success_reply(DEFINE_PORT_ACTION, 
				write_port_ddr(msg[ECP_PAYLOAD_IDX],
				msg[ECP_PAYLOAD_IDX + 1], 
				msg[ECP_PAYLOAD_IDX + 2]) >= 0);
			break;
		case GET_PORT_ACTION:
			/* Gets a port */
			if(msg[ECP_LEN_IDX] <  2 + ECPROTO_OVERHEAD){
				print_ecp_error("2 few parms");
				return -5;
			}
			
			return print_ecp_pin_update(msg[ECP_PAYLOAD_IDX], 
				msg[ECP_PAYLOAD_IDX + 1], 
				get_port_pin(msg[ECP_PAYLOAD_IDX], 
				msg[ECP_PAYLOAD_IDX + 1]));

			break;
		case WRITE_PORT_ACTION:
			/* Defines a port state / writes to the port */
			if(msg[ECP_LEN_IDX] <  3 + ECPROTO_OVERHEAD){
				print_ecp_error("2 few parms");
				return -6;
			}

			return print_success_reply(7,
				write_port(msg[ECP_PAYLOAD_IDX],
				msg[ECP_PAYLOAD_IDX + 1], 
				msg[ECP_PAYLOAD_IDX + 2]) >= 0);
			break;
		case REGISTER_COUNT:
			/* Request the gpio register count. */
			return print_ecp_msg(REGISTER_COUNT, &gpio_register_cnt, 
				sizeof(uint8_t));
			break;
		case REGISTER_LIST:
			/* Requested a list of register ids */
			return print_port_ids();
			break;
		case PIN_ENABLED:
			/* Request wether the given pin is disabled or not */
		{
			if(msg[ECP_LEN_IDX] < 2 + ECPROTO_OVERHEAD){
				print_ecp_error("2 few parms");
				return -7;
			}
			uint8_t pay[] = {msg[ECP_PAYLOAD_IDX], 
				msg[ECP_PAYLOAD_IDX + 1], 
				!is_pin_blacklisted(msg[ECP_PAYLOAD_IDX], 
					msg[ECP_PAYLOAD_IDX + 1])};
				
				/* The pin blacklist function is inverted, 
				 * because it returns true if the pin is 
				 * blacklisted and false if not. The action
				 * querys wther the pin is enabled. Therefore
				 * the state should be inverted
				 */
			return print_ecp_msg(11, pay ,sizeof(pay) );
		}


		break;
#ifdef UART_SECONDARY
		
		case SECONDARY_PRINT:
			if(msg[msg[ECP_LEN_IDX] - 4] != 0){
				print_ecp_error("nonull");
				return -8;		
			}
			{
				uint8_t success = 0>=write_string_to_secondary(
					(char*)&msg[ECP_PAYLOAD_IDX]);
				return print_ecp_msg(SECONDARY_PRINT, &success,
					sizeof(uint8_t));
			}

			break;

#endif /* UART_SECONDARY */
#ifdef ANALOG_EN
		case ADC_GET:
			{
				/* DEPRECATED, TO BE REMOVED: TODO*/
				uint8_t adc_res = get_adc();
				return print_ecp_msg(ADC_GET, &adc_res, 
					sizeof(adc_res));
			}
			break;
#endif /* ANALOG_EN */
		default:
			print_ecp_error("not implmntd");
			return -1;
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

/* If the target is larger than 1, tell the master that the inputs were invalid
 */
int print_ecp_pin_update(char reg_id, uint8_t bit_id, uint8_t target){
	
	initialized = true;

	if(target > 1){
		print_ecp_error("fld 2 get prt");
	}

	uint8_t pay[] = {(uint8_t)reg_id, bit_id, target};
	return print_ecp_msg(GET_PORT_ACTION, pay, sizeof(pay));
}

int print_success_reply(uint8_t action_id, bool success){
	uint8_t suc = success;
	return print_ecp_msg(action_id, &suc, sizeof(suc));
}

#ifdef ANALOG
int print_adc(){
	uint8_t adc = get_adc_result();
	return print_ecp_msg(ADC_GET, &adc, sizeof(adc));
}

#endif /* ANALOG */

int print_ecp_msg(uint8_t action_id, uint8_t* payload, size_t payload_length){

	static uint8_t frame[255];
	memset(frame,0, 255);
	
	frame[ECP_LEN_IDX] = ECPROTO_OVERHEAD + payload_length;
	frame[ECP_ADDR_IDX] = ECP_DEVICE_ID;
	frame[ECP_ID_IDX] = action_id;
	memcpy(&frame[ECP_PAYLOAD_IDX], payload, payload_length);
	uint16_t crc = 0;

	uint8_t crc_low, crc_high;

	for(size_t ov = 0; ov < ( 255 - payload_length); ov++){
		
		crc = ibm_crc(frame,frame[0] - 3);
		crc_high = (crc >> 8) & 0xFF;
		crc_low = crc & 0xFF;
		if(crc_high == 0xFF || crc_low == 0xFF){
			
			frame[ECP_LEN_IDX]++;
			continue;
		}else{
			/* No 0xff bytes have ben found inside the checksum.
			 * Stop 0-padding the frame */
			break;
		}
	}

		
	frame[frame[ECP_LEN_IDX] - 3] = (crc >> 8) & 0xFF;
	frame[frame[ECP_LEN_IDX] - 2] = crc & 0xFF;
	frame[frame[ECP_LEN_IDX] - 1] = 0xFF;

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
	if(!initialized){
		/* In case the µc has not yet been initialized. Most likely to
		 * happen in case the microcontroller has been reset. The
		 * microcontroller is considered initialized, when it has first
		 * answered a port state request.
		 */

		print_ecp_msg(INIT_ACTION, NULL, 0);
		return 1; // It's still successfull.
	}
	if(update_pins() < 0){
		print_ecp_error("updt fail");
	}
	uint8_t updatecnt = 0xFF & get_port_update_count();
	print_ecp_msg(SEND_NOTIFY, &updatecnt, sizeof(uint8_t));
	
	return send_port_updates();
}

