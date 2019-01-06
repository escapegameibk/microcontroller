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

extern const struct gpio_register_t gpio_registers[];


const struct gpio_register_t* get_port_reg_by_id(const char id);

#endif /* PORT_H */
