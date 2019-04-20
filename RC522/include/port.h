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

/*!
 * \brief GPIO Register description.
 *
 * Fully describes a GPIO banks register(s). A Bank consisting of 8 GPIO
 * pins with it's PORT, DDR and PIN register are fully described by this, and
 * are selected by this.
 */
struct gpio_register_t{
	unsigned volatile char* port; /*!< register pointer for the PORT 
					register */

	unsigned volatile char* ddir; /*!< register pointer for the DDIR
					 register */
	unsigned volatile char* pin; /*!< register pointer for the PIN register
					*/
	char car; /*!< A character describing what GPIO bank is used, e.g. 
	PORTA */

};

struct gpio_pin_t{
	char car;
	uint8_t pin; /* [0; 8[ */
};

/*! \brief This struct is used to fastly describe one single GPIO pin
 * 
 * This struct is used to describe a GPIO pin. a reference to the bank 
 * description is saved directly, so that it can be accessed without a delay.
 */
struct gpio_pin_desc_t{
	const struct gpio_register_t* reg;
	uint8_t bit; /* [0; 8[ */
};

extern const struct gpio_register_t gpio_registers[];


const struct gpio_register_t* get_port_reg_by_id(const char id);

#endif /* PORT_H */
