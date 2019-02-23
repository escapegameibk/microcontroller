/* PWM controller part
 * Copyright © 2019 tyrolyean
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

#include "port.h"

#ifndef PWM_H
#define PWM_H

extern struct gpio_pin_t active_pwm_pins[];
extern size_t current_pwm_count;

void add_pwm_pin(char car, uint8_t bit);
void remove_pwm_pin(char car, uint8_t bit);

bool is_pwm_pin_blacklisted(char car, uint8_t bit);

void init_pwm();

int set_pwm(uint8_t counter, uint8_t output, uint8_t value);

#endif /* PWM_H */
