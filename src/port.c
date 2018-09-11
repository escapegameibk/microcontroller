/* Port controlling parts.
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

#include "port.h"
#include "ecproto.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

/* Registers used are A,B,C,D,F,G,H,J,K,L */
const struct gpio_register_t gpio_registers[] = {
{&PORTA, &DDRA, &PINA, 'A' },
{&PORTB, &DDRB, &PINB, 'B' },
{&PORTC, &DDRC, &PINC, 'C' },
{&PORTD, &DDRD, &PIND, 'D' },
#if 0
/* Done in order to protect the uart pins */
{&PORTE, &DDRE, &PINE, 'E' },
#endif
{&PORTF, &DDRF, &PINF, 'F' },
{&PORTG, &DDRG, &PING, 'G' },
{&PORTH, &DDRH, &PINH, 'H' },
{&PORTJ, &DDRJ, &PINJ, 'J' },
{&PORTK, &DDRK, &PINK, 'K' },
{&PORTL, &DDRL, &PINL, 'L' }
};

uint8_t gpio_register_cnt = sizeof(gpio_registers) / 
	sizeof(struct gpio_register_t);

unsigned char last_pin_states[sizeof(gpio_registers) / sizeof(struct 
	gpio_register_t)];

unsigned char actual_pin_states[sizeof(gpio_registers) / sizeof(struct 
	gpio_register_t)];

int init_ports(){
	
	/* Initialize all pins with default values */
	for(size_t i = 0; i < gpio_register_cnt; i++){
		struct gpio_register_t reg = gpio_registers[i];
		*reg.ddir = 0xFF;
		*reg.port = 0x00;
	}
	
	update_pins();

	return save_ports();
}

int update_pins(){

	for(size_t i = 0; i < gpio_register_cnt; i++){
		struct gpio_register_t reg = gpio_registers[i];
		actual_pin_states[i] = *reg.pin;
	}

	return 0;
}

size_t get_port_update_count(){
	size_t upcnt = 0; 

	for(size_t regcnt = 0; regcnt < gpio_register_cnt; regcnt++){

		uint8_t pinold = last_pin_states[regcnt];
		uint8_t pinnew = actual_pin_states[regcnt];

		for(size_t bit = 0; bit < 8; bit++){

			bool bitold = (pinold >> bit) & 0x01;
			bool bitnew = (pinnew >> bit) & 0x01;
			if(bitold != bitnew){
				upcnt++;
			}
		}
	}

	return upcnt;
}

int send_port_updates(){

	int status = 0;

	for(size_t regcnt = 0; regcnt < gpio_register_cnt; regcnt++){

		uint8_t pinold = last_pin_states[regcnt];
		uint8_t pinnew = actual_pin_states[regcnt];

		for(size_t bit = 0; bit < 8; bit++){

			bool bitold = (pinold >> bit) & 0x01;
			bool bitnew = (pinnew >> bit) & 0x01;
			if(bitold != bitnew){
				struct gpio_register_t reg =
					gpio_registers[regcnt];
				status |= print_ecp_pin_update(reg.car, bit, bitnew);
			}
		}
	}

	status |= save_ports();

	return 0;	
}

/* Save the new port states to the old registers */
int save_ports(){
	
	for(size_t i = 0; i < gpio_register_cnt; i++){
		last_pin_states[i] = actual_pin_states[i];
	}
	return 0;	
}

gpio_register_t* get_port_reg_by_id(const char id){
	for(size_t i = 0; i < gpio_register_cnt; i++ ){
		if(gpio_registers[i].car == id){
			return gpio_registers[i];
		}
	}
	/* No found */
	return NULL;
}
