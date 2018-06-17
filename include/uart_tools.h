#ifndef UART_TOOLS_H
#define UART_TOOLS_H

#include "general.h"

#include <avr/io.h>
#include <stdio.h>

/** The baud rate to use for the UART */
#define UART_BAUD_RATE 9600UL

extern FILE uart_stdout;    /**< uart output filestream for functions like fprintf */
extern FILE uart_stdin;     /**< uart input filestream for functions like fscanf */

/* \brief define the UART registers to use */
#define UCSRA UCSR0A
#define UDRE UDRE0
#define UDR UDR0
#define RXC RXC0
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UCSRB UCSR0B
#define RXEN RXEN0
#define TXEN TXEN0
#define UCSZ0 UCSZ00
#define UCSZ1 UCSZ01
#define UCSRC UCSR0C

#if DEBUG
/** \brief Internaly used lookuptable to translate color codes into vt100 escape codes */
extern const char* vt100palette[17];

#define RESET 0  /**< Reset the text color */
#define BLACK 1
#define RED 2
#define GREEN 3
#define YELLOW 4 /**< Brown on some terminals */
#define BLUE 5
#define MAGENTA 6
#define CYAN 7
#define GRAY 8
#define DARKGRAY 9
#define LIGHTRED 10
#define LIGHTGREEN 11
#define LIGHTYELLOW 12
#define LIGHTBLUE 13
#define LIGHTMAGENTA 14
#define LIGHTCYAN 15
#define WHITE 16

#endif

/** \brief Change the text color of the attached terminal */
#define uart_set_color( col ) fprintf( &uart_stdout, "%s", vt100palette[ (col) ] )

/** \brief Clear the content of the terminal attached over UART */
#define uart_cls( ) fprintf( &uart_stdout, "\033[2J\033[H" )

/** \brief Completely reset an attached terminal, clearing screen, resetting colors and everything */
#define uart_reset( ) fprintf( &uart_stdout, "\033c" )

/**
 * \brief Move the cursor to a position relative to the upper left corner of the terminal
 *
 * \param x X coordinate in rows
 * \param y Y coordinate in lines
 */
#define uart_set_cursor_position( x, y ) fprintf( &uart_stdout, "\033[%i;%if", (y), (x) )

/**
 * \brief Fancy gets function
 *
 * \param ptr Puffer to write the retrieved characters to
 *
 * \return The number of bytes written to the buffer
 */
int uart_gets( char* ptr );

/**
 * \brief Initialise UART
 *
 * Initialises UART with the compiled in baud rate(UART_BAUD_RATE) and
 * creates uart_stdin and uart_stdout streams. It optionally pipes stdout,
 * stdin and stderr to the uart streams.
 *
 * \param pipetostd If non zero, the standard streams will be piped to uart
 */
void uart_init( int pipetostd );

#endif
