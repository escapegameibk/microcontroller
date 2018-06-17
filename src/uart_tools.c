/* uart tool for the atmel chips. OC: goliath
 * Copyright © 2018 goliath, tyrolyean
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
/*
 * Original Author:
 * this file was originally written by David Oberhollenzer and was handed over
 * to me at 2018/06/04. All behail him. 
 */

#include "uart_tools.h"

#if DEBUG
const char* vt100palette[] =
{
    "\033[0m",  /* RESET */
    "\033[22;30m", "\033[22;31m", "\033[22;32m", "\033[22;33m",
    "\033[22;34m", "\033[22;35m", "\033[22;36m", "\033[22;37m",
    "\033[01;30m", "\033[01;31m", "\033[01;32m", "\033[01;33m",
    "\033[01;34m", "\033[01;35m", "\033[01;36m", "\033[01;37m"
};

#endif


int uart_putc( char c, FILE* stream )
{
    if( c=='\n' )
        uart_putc( '\r', NULL );

    while( !(UCSRA & (1<<UDRE)) ); /* Wait for UDR to get ready */

    UDR = c;                       /* send */

    return 0;
}

int uart_getc( FILE* stream )
{
    while( !(UCSRA & (1<<RXC)) ); /* Wait until we receive a character */

    char c = (char)UDR;

    if( c>31 && c<127 )
        uart_putc( c, NULL );

    return c;                     /* Return received character */
}

/* File pointer for uart communication via fprintf */
FILE uart_stdout = FDEV_SETUP_STREAM( uart_putc, NULL,      _FDEV_SETUP_WRITE );
FILE uart_stdin  = FDEV_SETUP_STREAM( NULL,      uart_getc, _FDEV_SETUP_READ  );


/*
 * This function returns a null-terminated string which is received from the
 * stdin. it teminates a string at a carriage return or line feed and deletes
 * chars after a backsapce. it emulates a terminal 
 *
 */

int uart_gets( char* ptr )
{
        char c;
        int i=0;

        for( ; ; ){
                c = getchar( );

                if( c=='\033'          ) continue;
                if( c=='\n' || c=='\r' ) break;

                if( c=='\b' ){
                        if( i>0 ){
                                printf( "\033[D \033[D" );
                                --i;
                        }

                        continue;
                }

                ptr[ i++ ] = c;
        }

        ptr[ i++ ] = '\0';

        putchar( '\n' );

        return i;
}

/*
 * \brief: initializes the uart controller
 *
 */
void uart_init( int pipetostd )
{
        uint16_t ubrr = (uint16_t) ( (uint32_t)F_CPU/(16*UART_BAUD_RATE) - 1 );

        UBRRH = (unsigned char)(ubrr>>8);
        UBRRL = (unsigned char) ubrr;

        UCSRB = (1<<RXEN) | (1<<TXEN);                /* Enable receive and transmit */
        UCSRC = (1<<UCSZ1) | (1<<UCSZ0); /* 8 data bits, 1 stop bit */

        /* Flush Receive-Buffers */
        do{
                UDR;
        }while( UCSRA & (1<<RXC) );

        /* map standard input and output streams to uart */
        if( pipetostd ){

                stdin  = &uart_stdin;
                stdout = &uart_stdout;
                stderr = &uart_stdout;
        }

        return;
}

