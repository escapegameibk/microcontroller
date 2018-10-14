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
#include <stdbool.h>

/* Registers used are A,B,C,D,E,F,G,H,J,K,L */
const struct gpio_register_t gpio_registers[] = {
{&PORTA, &DDRA, &PINA, 'A' },
{&PORTB, &DDRB, &PINB, 'B' },
{&PORTC, &DDRC, &PINC, 'C' },
{&PORTD, &DDRD, &PIND, 'D' },
{&PORTE, &DDRE, &PINE, 'E' },
{&PORTF, &DDRF, &PINF, 'F' },
{&PORTG, &DDRG, &PING, 'G' },
{&PORTH, &DDRH, &PINH, 'H' },
{&PORTJ, &DDRJ, &PINJ, 'J' },
{&PORTK, &DDRK, &PINK, 'K' },
{&PORTL, &DDRL, &PINL, 'L' }
};

const struct gpio_pin_t gpio_disabled_pins[] ={
#ifdef UART_SECONDARY
{'H', 0},	/* USART2 RXD */
{'H', 1},	/* USART2 TXD */
#endif /* UART_SECONDARY */
{'E', 0},	/* USART0 RXD */
{'E', 1}	/* USART0 TXD */
};

#define gpio_register_cnt (sizeof(gpio_registers) / sizeof(struct gpio_register_t))

unsigned char last_pin_states[gpio_register_cnt];

unsigned char actual_pin_states[gpio_register_cnt];

void init_ports(){
	
	/* Query pins for their default values */
	update_pins();

	save_ports();
	return;
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

const struct gpio_register_t* get_port_reg_by_id(const char id){
	for(size_t i = 0; i < gpio_register_cnt; i++ ){
		if(gpio_registers[i].car == id){
			return &gpio_registers[i];
		}
	}
	/* No found */
	return NULL;
}

/* Returns >= 0 on success and <= 0 on error */
int write_port(char id, uint8_t bit, bool value){
	const struct gpio_register_t* bank = get_port_reg_by_id(id);
	if(bank == NULL){
		/* Couldn't find bank */
		return -1;
	}

	if(bit >= 8){
		/* Too high bit */
		return -2;
	}
	
	if(is_pin_blacklisted(id, bit)){
		/* Not allowed to write to pin */
		return -3;
	}

	/* Should be fine */
	if(!!value){
		*bank->port |= (!!value) << bit;
	}else{
		*bank->port &= ~(1 << bit);
	}

	return 0;
}

/* Returns >= 0 on success and <= 0 on error */
int write_port_ddr(char id, uint8_t bit, bool value){
	const struct gpio_register_t* bank = get_port_reg_by_id(id);
	if(bank == NULL){
		/* Couldn't find bank */
		return -1;
	}

	if(bit >= 8){
		/* Too high bit */
		return -2;
	}

	if(is_pin_blacklisted(id, bit)){
		/* Not allowed to write to pin */
		return -3;
	}

	if(!!value){
		*bank->ddir |= (1) << bit;
	}else{
		*bank->ddir &= ~(1 << bit);
	}
	return 0;
}

/* Returns 0 for low, 1 for high, and > 1 on error */
uint8_t get_port_pin(char id, uint8_t bit){
	const struct gpio_register_t* bank = get_port_reg_by_id(id);
	if(bank == NULL){
		/* Couldn't find bank */
		return 2;
	}

	if(bit >= 8){
		/* Too high bit */
		return 3;
	}

	return ((*bank->pin >> bit) & 0x01);
}

int print_port_ids(){
	uint8_t regids[gpio_register_cnt];
	for(size_t i = 0; i < gpio_register_cnt; i++){
		regids[i] = gpio_registers[i].car;
	}

	return print_ecp_msg(10, regids, gpio_register_cnt);
}
/*
 * Returns true on blacklist and false if not
*/
bool is_pin_blacklisted(char car, uint8_t id){

	for(size_t i = 0; i < (sizeof(gpio_disabled_pins) / 
		sizeof(struct gpio_pin_t)); i++){
		if( gpio_disabled_pins[i].car == car &&
			gpio_disabled_pins[i].pin == id){
			return true;
		}
	}
	return false;
}
