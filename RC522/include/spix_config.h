/*
 * SPI MASTER AVR CONFIG
 */

#ifndef SPIX_CONFIG_H
#define SPIX_CONFIG_H

#include <avr/io.h>

/*
 * If set to != 0, spi api will work in master mode
 * else in slave mode
 */

#define SPI_CONFIG_AS_MASTER 	1


/*
 * SPI pin definition
 */

#define SPI_DDR		DDRB
#define SPI_PORT	PORTB
#define SPI_PIN		PINB
#define SPI_MOSI	PORTB3
#define SPI_MISO	PORTB4
#define SPI_SCK		PORTB5

#endif
