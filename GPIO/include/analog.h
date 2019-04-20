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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef ANALOG_H
#define ANALOG_H

struct adc_channel_t{
	uint16_t old_value;
	uint16_t new_value;
	bool active;
};

extern struct adc_channel_t adc_channels[];

uint8_t get_new_adc_updates(bool send);
int register_new_adc_channels(const uint8_t* frame);
int16_t read_adc(const uint8_t channel);

#endif /* ANALOG_H */
