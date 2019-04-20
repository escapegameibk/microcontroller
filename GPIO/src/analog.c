/* ADC conversion and storage
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

#include "analog.h"
#include "ecproto.h"

#include <avr/io.h>
#include <stdbool.h>


#ifdef __AVR_ATmega2560__

struct adc_channel_t adc_channels[64];

#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)

struct adc_channel_t adc_channels[16];

#endif

uint8_t get_new_adc_updates(bool send){
	
	uint8_t updatecnt = 0;

	for(uint8_t i = 0; i < 
		(sizeof(adc_channels) / sizeof(struct adc_channel_t)); i++){
		
		struct adc_channel_t* channel = &adc_channels[i];
		
		if(!channel->active){
			continue;
		}
		
		if(!send){
			channel->new_value = read_adc(i);
			if(channel->new_value != channel->old_value){
				updatecnt++;
			}

		}else if(channel->new_value != channel->old_value){
			
			int16_t res = channel->new_value;

			uint8_t payload[] = {
				i,
				((res >> 8) & 0xFF),
				(res & 0xFF)
			};

			if(print_ecp_msg(ADC_GET2, payload, sizeof(payload)) 
				< 0){

				return -1;
			}else{
				channel->old_value = channel->new_value;
				continue;
			}

		}else{
			continue;
		}


	}
	return updatecnt;
}

/* Registers a new adc channel to be used. */
int register_new_adc_channels(const uint8_t* frame){
	
	if((frame[ECP_LEN_IDX] - frame[ECP_PAYLOAD_IDX] )< ECPROTO_OVERHEAD){
		return -1;
	}

	for(size_t i = 0; i < frame[ECP_PAYLOAD_IDX]; i++){
		uint8_t channel = frame[ECP_PAYLOAD_IDX + i + 1];
		
		if(channel >=
			(sizeof(adc_channels) / sizeof(struct adc_channel_t))){
			
			return -1;

		}

		adc_channels[channel].active = true;
		adc_channels[channel].old_value = 0;
		adc_channels[channel].new_value = 0;

	}

	return 0;

}

int16_t read_adc(const uint8_t channel){

	if(channel >= (sizeof(adc_channels) / sizeof(struct adc_channel_t))){
		
		return -1;

	}

#ifdef __AVR_ATmega2560__
		
	if(channel & (1 << 5)){
		ADCSRB |= (1 << MUX5);
	}else{
		
		ADCSRB &= ~(1 << MUX5);
	}

#endif

	ADMUX = ( 0b11111 & (channel)) | (ADMUX & ~(0b11111));
	
	ADCSRA |= (1<<ADSC);
	while((ADCSRA >> ADSC) & 0x1){ /* Conversion in progress*/}
	int16_t val = (ADC & 0b1111111111);

	if((val & 0xFF)  == 0xFF){
		val--;
	}
	
	adc_channels[channel].new_value = val;


	return val;
}

