/*
 * spi.h
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * Copyrights:
 *
 * Copyright 2013 Shimon <shimon@monistit.com>
 * Copyright 2018-2019 tyrolyean <tyrolyean@mtyrolyean.net>
 * 
 */
#ifndef SPIX_H
#define SPIX_H

#include "spix_config.h"

#include <stdint.h>
#include <stdbool.h>

void    spi_init();
uint8_t spi_transmit(uint8_t data);

void    spi_set_cs(char car, uint8_t bit);

extern struct gpio_pin_desc_t spi_current_cs;

#define ENABLE_CHIP() (*spi_current_cs.reg->port &= (~(1<<spi_current_cs.bit)))
#define DISABLE_CHIP() (*spi_current_cs.reg->port |= (1<<spi_current_cs.bit))

#endif
