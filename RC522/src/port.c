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

/* According to https://www.microchip.com/webdoc/AVRLibcReferenceManual/using_tools_1using_avr_gcc_mach_opt.html 
 * the used preprocessor definitions may be used to identify an AVR chip. The
 * ports are specified according to their datasheet from atmel. Microchip is
 * a bit awkward. 
 */

#ifdef __AVR_ATmega2560__ 
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
#endif

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
/* The atmega328 series only has three gpio registers */
const struct gpio_register_t gpio_registers[] = {
{&PORTB, &DDRB, &PINB, 'B' },
{&PORTC, &DDRC, &PINC, 'C' },
{&PORTD, &DDRD, &PIND, 'D' }
};

#endif


uint8_t gpio_register_cnt = (sizeof(gpio_registers) / 
	sizeof(struct gpio_register_t));

const struct gpio_register_t* get_port_reg_by_id(const char id){
	for(size_t i = 0; i < gpio_register_cnt; i++ ){
		if(gpio_registers[i].car == id){
			return &gpio_registers[i];
		}
	}
	/* No found */
	return NULL;
}
