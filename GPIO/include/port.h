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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef PORT_H
#define PORT_H

struct gpio_register_t{
	unsigned volatile char* port;
	unsigned volatile char* ddir;
	unsigned volatile char* pin;
	char car;
};

struct gpio_pin_t{
	char car;
	uint8_t pin; /* [0; 8[ */
};

/* This struct is used to completely describe one single GPIO pin */
struct gpio_pin_desc_t{
	const struct gpio_register_t* reg;
	uint8_t bit; /* [0; 8[ */
};

extern uint8_t gpio_register_cnt;

void init_ports();

int update_pins();
size_t port_updates(bool send);
int save_ports();

const struct gpio_register_t* get_port_reg_by_id(const char id);
int write_port(char id, uint8_t bit, bool value);
int write_port_ddr(char id, uint8_t bit, bool value);
/* Returns 0 for low, 1 for high, and > 1 on error */
uint8_t get_port_pin(char id, uint8_t bit);
int print_port_ids();
bool is_pin_blacklisted(char car, uint8_t id);
#ifdef ANALOG_EN
uint8_t get_adc();
#endif /* ANALOG_EN */

uint8_t get_disabled_gpios(bool send);

#endif /* PORT_H */
