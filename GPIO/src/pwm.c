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
#include <string.h>
#include <stdbool.h>

/* The array lengths are set to be the max amount of pwm outputs */
#ifdef __AVR_ATmega2560__
	struct gpio_pin_t active_pwm_pins[20];
#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
	struct gpio_pin_t active_pwm_pins[6];
#endif

size_t current_pwm_count = 0;

void add_pwm_pin(char car, uint8_t bit){
	
	for(size_t i = 0; i < current_pwm_count; i++){
		if(active_pwm_pins[i].car == car && 
			active_pwm_pins[i].pin == bit){
			/* Already active */
			return;
		}
	}
	/* Not yet present. Add it! */
	write_port_ddr(car, bit, 1);
	
	active_pwm_pins[current_pwm_count].car = car;
	active_pwm_pins[current_pwm_count++].pin = bit;
				
	uint8_t pay = 2;
	print_ecp_msg(SEND_NOTIFY, &pay, sizeof(pay));
	
	uint8_t dis_pay[] = {car, bit, false};
	print_ecp_msg(PIN_ENABLED, dis_pay, sizeof(dis_pay));

	return;
}

void remove_pwm_pin(char car, uint8_t bit){
	
	for(size_t i = 0; i < current_pwm_count; i++){
		if(active_pwm_pins[i].car == car && 
			active_pwm_pins[i].pin == bit){
			/* Found ya! */
				
				uint8_t pay = 2;
				print_ecp_msg(SEND_NOTIFY, &pay, sizeof(pay));
				uint8_t dis_pay[] = {car, bit, true};
				print_ecp_msg(PIN_ENABLED, dis_pay, 
					sizeof(dis_pay));
				

				if( i != current_pwm_count-1){
					memmove(&active_pwm_pins[i], 
						&active_pwm_pins[i+1],
						(current_pwm_count - i) *
						sizeof(struct gpio_pin_t));
					
				}
				current_pwm_count--;

			break;
			
		}
	}

	/* Yeah whatever */
	
	return;
}
bool is_pwm_pin_blacklisted(char car, uint8_t bit){
	
	
	for(size_t i = 0; i < current_pwm_count; i++){
		if(active_pwm_pins[i].car == car && 
			active_pwm_pins[i].pin == bit){
			return true;
		}
	}

	return false;

}

/* Initialize counter registers and set thresholds. */
void init_pwm(){

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || \
	defined(__AVR_ATmega2560__)

	/* Set clock speeds and PWM modes */
	TCCR0B  = (1 << CS02); /* Is missing the phase and frq correct mode */
	TCCR1B  = ((1 << WGM13) | (1 << CS12));
	/* Top counter limit */
	ICR1  = 0xFF;
	

#ifdef __AVR_ATmega2560__
	TCCR2B  = (1 << CS22); /* Is missing the phase and frq correct mode */
	TCCR3B  = ((1 << WGM33) | (1 << CS32));
	TCCR4B  = ((1 << WGM43) | (1 << CS42));
	TCCR5B  = ((1 << WGM53) | (1 << CS52));
	/* Top counter limits */
	ICR3  = 0xFF;
	ICR4  = 0xFF;
	ICR5  = 0xFF;

#endif /* __AVR_ATmega2560__ */
#endif	/* multiple */
	return;

}

int set_pwm(uint8_t counter, uint8_t output, uint8_t value){

	if(counter == 0){
		
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
		if(output == 0){
			if(value != 0){
				TCCR0A |= ((1 << COM0A1) | (0 << COM0A0));
				OCR0A = value;
				add_pwm_pin('D', 5);

			}else{
				TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
				remove_pwm_pin('D', 5);

			}
		}else 
#endif
		if(output == 1){
			if(value != 0){
				TCCR0A |= ((1 << COM0B1) | (0 << COM0B0));
				OCR0B = value;
#ifdef __AVR_ATmega2560__
				add_pwm_pin('G', 5);

#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
				add_pwm_pin('D', 6);
#endif

			}else{
				TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));
#ifdef __AVR_ATmega2560__
				remove_pwm_pin('G', 5);

#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
				remove_pwm_pin('D', 6);
#endif

			}

		}else{
			return -1;
		}

	}else if(counter == 1){
		
		if(output == 0){
			if(value != 0){
				TCCR1A |= ((1 << COM1A1) | (0 << COM1A0));
				OCR1A = value;
#ifdef __AVR_ATmega2560__
				add_pwm_pin('B', 5);

#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
				add_pwm_pin('B', 1);
#endif

			}else{
				TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));
#ifdef __AVR_ATmega2560__
				remove_pwm_pin('B', 5);

#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
				remove_pwm_pin('B', 1);
#endif

			}
		}else if(output == 1){
			if(value != 0){
				TCCR1A |= ((1 << COM1B1) | (0 << COM1B0));
				OCR1B = value;
#ifdef __AVR_ATmega2560__
				add_pwm_pin('B', 6);

#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
				add_pwm_pin('B', 2);
#endif

			}else{
				TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0));
#ifdef __AVR_ATmega2560__
				remove_pwm_pin('B', 6);

#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
				remove_pwm_pin('B', 2);
#endif

			}
#ifdef __AVR_ATmega2560__
		}else if(output == 2){
			if(value != 0){
				TCCR1A |= ((1 << COM1C1) | (0 << COM1C0));
				OCR1C = value;
				add_pwm_pin('B', 7);

			}else{
				TCCR1A &= ~((1 << COM1C1) | (1 << COM1C0));
				remove_pwm_pin('B', 7);

			}

#endif /* __AVR_ATmega2560__ */
		}else{
			return -1;
		}

#ifdef __AVR_ATmega2560__
	}else if(counter == 2){
		
		if(output == 0){
			if(value != 0){
				TCCR2A |= ((1 << COM2A1) | (0 << COM2A0));
				OCR2A = value;
				add_pwm_pin('B', 4);

			}else{
				TCCR2A &= ~((1 << COM2A1) | (1 << COM2A0));
				remove_pwm_pin('B', 4);

			}
		}else if(output == 1){
			if(value != 0){
				TCCR2A |= ((1 << COM2B1) | (0 << COM2B0));
				OCR2B = value;
				add_pwm_pin('H', 6);

			}else{
				TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));
				remove_pwm_pin('H', 6);

			}
		}else{
			return -1;
		}
	}else if(counter == 3){
		
		if(output == 0){
			if(value != 0){
				TCCR3A |= ((1 << COM3A1) | (0 << COM3A0));
				OCR3A = value;
				add_pwm_pin('E', 3);

			}else{
				TCCR3A &= ~((1 << COM3A1) | (1 << COM3A0));
				remove_pwm_pin('E', 3);

			}
		}else if(output == 1){
			if(value != 0){
				TCCR3A |= ((1 << COM3B1) | (0 << COM3B0));
				OCR3B = value;
				add_pwm_pin('E', 4);

			}else{
				TCCR3A &= ~((1 << COM3B1) | (1 << COM3B0));
				remove_pwm_pin('E', 4);

			}
		}else if(output == 2){
			if(value != 0){
				TCCR3A |= ((1 << COM3C1) | (0 << COM3C0));
				OCR3C = value;
				add_pwm_pin('E', 5);

			}else{
				TCCR3A &= ~((1 << COM3C1) | (1 << COM3C0));
				remove_pwm_pin('E', 5);

			}

		}else{
			return -1;
		}
	}else if(counter == 4){
		
		if(output == 0){
			if(value != 0){
				TCCR4A |= ((1 << COM4A1) | (0 << COM4A0));
				OCR4A = value;
				add_pwm_pin('H', 3);

			}else{
				TCCR4A &= ~((1 << COM4A1) | (1 << COM4A0));
				remove_pwm_pin('H', 3);

			}
		}else if(output == 1){
			if(value != 0){
				TCCR4A |= ((1 << COM4B1) | (0 << COM4B0));
				OCR4B = value;
				add_pwm_pin('H', 4);

			}else{
				TCCR4A &= ~((1 << COM4B1) | (1 << COM4B0));
				remove_pwm_pin('H', 4);

			}
		}else if(output == 2){
			if(value != 0){
				TCCR4A |= ((1 << COM4C1) | (0 << COM4C0));
				OCR4C = value;
				add_pwm_pin('H', 5);

			}else{
				TCCR4A &= ~((1 << COM4C1) | (1 << COM4C0));
				remove_pwm_pin('H', 5);

			}

		}else{
			return -1;
		}
	}else if(counter == 5){
		
		if(output == 0){
			if(value != 0){
				TCCR5A |= ((1 << COM5A1) | (0 << COM5A0));
				OCR5A = value;
				add_pwm_pin('L', 3);

			}else{
				TCCR5A &= ~((1 << COM5A1) | (1 << COM5A0));
				remove_pwm_pin('L', 3);

			}
		}else if(output == 1){
			if(value != 0){
				TCCR5A |= ((1 << COM5B1) | (0 << COM5B0));
				OCR5B = value;
				add_pwm_pin('L', 4);

			}else{
				TCCR5A &= ~((1 << COM5B1) | (1 << COM5B0));
				remove_pwm_pin('L', 4);

			}
		}else if(output == 2){
			if(value != 0){
				TCCR5A |= ((1 << COM5C1) | (0 << COM5C0));
				OCR5C = value;
				add_pwm_pin('L', 5);

			}else{
				TCCR5A &= ~((1 << COM5C1) | (1 << COM5C0));
				remove_pwm_pin('L', 5);

			}

		}else{
			return -1;
		}

#endif /* __AVR_ATmega2560__ */
	}else{
		/* I'm guessing wrong device */
		return -1;
	}
	
	return 0;

}

