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


struct gpio_pin_desc_t spi_current_cs = {
	NULL,
	0
};

void spi_init(){
	/* Today was a good day. After 3 days of debugging, I finally found out,
	 * why the fuck the avr would seemingly at random hang up. It wouldn't 
	 * let me transmit anything over the SPI. My initial suspicions of a
	 * stack overflow turned out to be incorrect. A small sentence in the
	 * AVR's datasheet made me a bit angry:
	 *  Bit 4 – MSTR: Master/Slave Select
	 *  This bit selects Master SPI mode when written to one, and Slave SPI
	 *  mode when written logic zero. If SS is config-ured as an input and 
	 *  is driven low while MSTR is set, MSTR will be cleared, and SPIF in 
	 *  SPSR will become set.The user will then have to set MSTR to 
	 *  re-enable SPI Master mode.
	 *
	 * Well FUCK. Please leave the SPI_SS in here.
	 */

	SPI_DDR = (1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_SS);
	SPI_PORT = (1<<SPI_SS); /* DISABLE SS */
	/* The SPI prescaler is set to 16, which ends up at ~1MHz. This would be
	 * small enough to get through a whole room probably. Well not
	 * seriously, it would come close to 3-4m though.*/
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR0);//prescaler 16

	return;
}

/*
 * Transmit one char to the slave. The Chip select pin is to be pulled low
 * by the caller.
 */
uint8_t spi_transmit(uint8_t data){
	
	SPDR = data;
	cli();
	while(!(SPSR & (1<<SPIF))){
		if(UCSR0A & (1<<RXC0)){
			consume_uart_master_byte(UDR0);
		}
	}

	return SPDR;
}


void spi_set_cs(char car, uint8_t bit){

	if(bit >= 8){
		/* WTF are you doing? */
		return;
	}

	if(get_port_reg_by_id(car) == NULL){
		return;
	}

	spi_current_cs.reg = get_port_reg_by_id(car);
	spi_current_cs.bit = bit;

	/* Set the SPI pin to output mode and drive it high */
	
	*spi_current_cs.reg->ddir |= (0x1 << spi_current_cs.bit);
	
	return;

}
