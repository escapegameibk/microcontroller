/*
 * spi.c
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
 * Copyright 2018 tyrolyean <tyrolyean@tyrolyean.net>
 *
 */

#include "spix.h"
#include "serial.h"
#include "port.h"

#include <string.h>

uint8_t spi_devcnt = 0;
bool spi_initialized = false;

struct gpio_pin_desc_t spi_current_cs = {
	NULL,
	0
};

#if SPI_CONFIG_AS_MASTER
void spi_init(){

	SPI_DDR = (1<<SPI_MOSI)|(1<<SPI_SCK);
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);//prescaler 16

	spi_initialized = true;

	return;
}

/*
 * Transmit one char to the slave. The Chip select pin is to be pulled low
 * by the caller.
 */
uint8_t spi_transmit(uint8_t data){
	
	if(!spi_initialized){
		spi_init();
	}

	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	
	return SPDR;
}

#else /* SPI_CONFIG_AS_MASTER */
void spi_init(){

	SPI_DDR = (1<<SPI_MISO);
	SPCR = (1<<SPE) | (1<<SPR00);

	spi_initialized = true;
}

/*
 * Transmit one char to the slave. The Chip select pin is to be pulled low
 * by the caller.
 */
uint8_t spi_transmit(uint8_t data){
	
	if(!spi_initialized){
		spi_init();
	}

	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

#endif /* SPI_CONFIG_AS_MASTER */

void spi_set_cs(char car, uint8_t bit){

	if(bit >= 8){
		return;
	}

	if(get_port_reg_by_id(car) == NULL){
		return;
	}

	spi_current_cs.reg = get_port_reg_by_id(car);
	spi_current_cs.bit = bit;

	/* Set the SPI pin to output mode and drive it high */
	
	*spi_current_cs.reg->ddir |= (0x1 << spi_current_cs.bit);
	*spi_current_cs.reg->port |= (0x1 << spi_current_cs.bit);
	
	return;

}

int spi_select_slaveno(uint8_t id){

	if( id >= spi_devcnt){
		return -1;
	}
	
	/* Copy the original pin as the current one */
	spi_set_cs(spi_used_pins[id].car, spi_used_pins[id].pin);

	return 0;
	
}

int spi_register_slave(char reg_car, uint8_t bit){

	if(SPI_SS_CNT <= spi_devcnt){
		/* No space left to store devices. */
		return -1;
	}

	if(get_port_reg_by_id(reg_car) == NULL){
		/* No such register */
		return -2;
	}

	if(bit >= 8){
		/* Apparently you have 8+n | n > 0 bit bytes. */ 
		return -3;
	}

	if(!spi_initialized){
		/* I guess if you begin to add slaves, you may just as well use 
		 * them. 
		 */
		spi_init();
	}

	/* Write data to newest position of array */
	spi_devcnt++;
	uint8_t arrin = spi_devcnt + SPI_BASE_PORTCNT - 1;

	struct gpio_pin_t ss = {reg_car, bit};

	memcpy(&spi_used_pins[arrin], &ss, sizeof(ss));
	/* Successfull addition of the spi device */

	return 0;
}


