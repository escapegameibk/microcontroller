/* controller for the hardware attached to the atmega
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

#include "controller.h"
#include "general.h"
#include "serial.h"
#include "utils.h"
#include "uart_tools.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * according to the documentation of atmel, the PORTx, DDRx and PINx registers
 * are responsible for each group of 8 pins.the atmega2560 which is used for
 * this program has A,B,C,D,E,F,G,H,J and L registers and the according 8 pins.
 * the DDRx registers define if the pin is input or output where 0 is input
 * and 1 is output. The PORTx registers activate pullup resistors if the pin is
 * input, or the pin is driven high if 1 or low if 0 if it is configured as
 * output pin, which means if the register is set to 0, the pin is either 0
 * (if PORTx is 1) or 1. the PINx registers represent the value of the pin, so 
 * HIGH 1 or LOW 0.
 */

/* PIN MAP:
 * PINA:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PINB:
 *      0: relay1 switch1 output
 *      1: relay2 switch1 output
 *      2: relay1 switch2 output
 *      3: relay2 switch2 output
 *      4:
 *      5:
 *      6:
 *      7:
 * PINC:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PIND:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7: relay2 switch8 output
 * PINE:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PINF:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PING:
 *      0: relay1 switch7 output
 *      1: relay2 switch7 output
 *      2: relay1 switch8 output
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PINH:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PINJ:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PINK:
 *      0:
 *      1:
 *      2:
 *      3:
 *      4:
 *      5:
 *      6:
 *      7:
 * PINL:
 *      0: relay1 switch3 output
 *      1: relay2 switch3 output 
 *      2: relay1 switch4 output
 *      3: relay2 switch4 output
 *      4: relay1 switch5 output
 *      5: relay2 switch5 output
 *      6: relay1 switch6 output
 *      7: relay2 switch6 output
 */    

/* If this pin map isn't up to date, you probably shouldn't trust the
 * programmer who was here in the first place.
 */


/*
 * init the hardware registers
 */

int controller_init(){

#if DEBUG
        printf("0%cINIT HARDWARE%c",PARAM_DELIMITER,CMD_DELIMITER);
#endif

        /* Define outputs and inputs */
        DDRA = 0b00000000;
        DDRB = 0b00001111;
        DDRC = 0b00000000;
        DDRD = 0b10000000;
        DDRE = 0b00000000;
        DDRF = 0b00000000;
        DDRG = 0b00000111;
        DDRH = 0b00000000;
        DDRJ = 0b00000000;
        DDRK = 0b00000000;
        DDRL = 0b11111111;

        PORTA = 0b11111111;
        PORTB = 0b11111111;
        PORTC = 0b11111111;
        PORTD = 0b11111111;
        PORTE = 0b11111111;
        PORTF = 0b11111111;
        PORTG = 0b11111111;
        PORTH = 0b11111111;
        PORTJ = 0b11111111;
        PORTK = 0b11111111;
        PORTL = 0b11111111;


        return 0;
}
/*
 * this function executes a command received from the interface.
 */
int execute_command(char* cmd){
        
        size_t paramcnt = charcnt(cmd, PARAM_DELIMITER);       
        size_t len = strlen(cmd);
        
        replace_char(cmd,CMD_DELIMITER,0);
        replace_char(cmd,PARAM_DELIMITER,0);

        char* params[paramcnt+1];
        
        /* make all strings reachable*/
        size_t i = 0;
        for(char* iterator = cmd; iterator < (cmd + len); iterator++ ){
                if(*iterator == 0 && i < paramcnt){
                        /* found end of string */
                        params[i++] = iterator + 1;
                }
        }

        int action = atoi(cmd);

#if DEBUG
        printf("0%creceived %i params for action %i %c", PARAM_DELIMITER, 
                        paramcnt, action, CMD_DELIMITER);
#endif
        
        switch (action) {
        case 0: /* debug or error*/
#if !DEBUG
                /* if not compiled for debug, throw error */
                fprintf(&uart_stdout,"1%cDEBUG%c",PARAM_DELIMITER,
                                CMD_DELIMITER);
#endif
                break;
        case 1: /* INIT */
                register_dump();
                break;
        case 3: /* update messages */
               
                update_io(params[0][0], atoi(params[1]), atoi(params[2]));

                break;
        default:
                fprintf(&uart_stdout, "1%cINVCMD%c",PARAM_DELIMITER,
                                CMD_DELIMITER);
                return -1;
                break;
                        
        }       
        

        return 0;
}

int io_patrol(){
        
        if(PINA_o != PINA){
                return process_update('A',&PINA,&PINA_o);
        }

        if(PINB_o != PINB){
                return process_update('B',&PINB,&PINB_o);
        }
        
        if(PINC_o != PINC){
                return process_update('C',&PINC,&PINC_o);
        }

        if(PIND_o != PIND){
                return process_update('D',&PIND,&PIND_o);
        }

       /* The E register contains the uart connection and thus changes value
        * when a transmission occures. i had to disable it for the time beeing.
        * if(PINE_o != PINE){
                return process_update('E',&PINE,&PINE_o);
        }*/

        if(PINF_o != PINF){
                return process_update('F',&PINF,&PINF_o);
        }
        
        if(PING_o != PING){
                return process_update('G',&PING,&PING_o);
        }
        if(PINH_o != PINH){
                return process_update('H',&PINH,&PINH_o);
        }
        if(PINJ_o != PINJ){
                return process_update('J',&PINJ,&PINJ_o);
        }
        if(PINK_o != PINK){
                return process_update('K',&PINK,&PINK_o);
        }
        if(PINL_o != PINL){
                return process_update('L',&PINL,&PINL_o);
        }

        return 0;
}

int update_io(char REG, int bit, int state){

#if DEBUG
        printf("0%cupdating register %c bit %i to %i%c",PARAM_DELIMITER,
                        REG, bit, state, CMD_DELIMITER);
#endif
        volatile uint8_t* regp_new;
        uint8_t* regp_old;

        switch(REG){
        
        case 'A':
                regp_new = &PORTG;
                regp_old = &PINA_o;
                break;
        case 'B':
                regp_new = &PORTG;
                regp_old = &PINB_o;
                break;
        case 'C':
                regp_new = &PORTG;
                regp_old = &PINC_o;
                break;
        case 'D':
                regp_new = &PORTG;
                regp_old = &PIND_o;
                break;
        case 'E':
                regp_new = &PORTG;
                regp_old = &PINE_o;
                break;
        case 'F':
                regp_new = &PORTG;
                regp_old = &PINF_o;
                break;
        case 'G':
                regp_new = &PORTG;
                regp_old = &PING_o;
                break;
        case 'H':
                regp_new = &PORTH;
                regp_old = &PINH_o;
                break;
        case 'J':
                regp_new = &PORTJ;
                regp_old = &PINJ_o;
                break;
        case 'K':
                regp_new = &PORTK;
                regp_old = &PINK_o;
                break;
        case 'L':
                regp_new = &PORTL;
                regp_old = &PINL_o;
                break;

        default:
                fprintf(&uart_stdout,"1%cINVREG%c",PARAM_DELIMITER, CMD_DELIMITER);
                return -1;
                break;
        }
        
        if(state != 0){

                *regp_old |= 0b00000001 << bit;
                *regp_new |= 0b00000001 << bit;

        }else{
                *regp_old &= ~(0b0000001 << bit);
                *regp_new &= ~(0b0000001 << bit);
        }
        
        return 0;
}

/* 
 * sends an update to the host and updates a regster's old value based on the
 * first registered input change. 
 */
int process_update(char REG, volatile uint8_t* regp_new, uint8_t* regp_old){
        
        uint8_t regread= *regp_new;
        for(int i = 0; i < 8; i++){
                if(((regread >> i ) | 0b00000001 ) ==  
                                ((*regp_old >> i) | 0b00000001)){
                        /* send statechange to host */
                        fprintf(&uart_stdout,"3%c%c%c%i%c%i%c",PARAM_DELIMITER,
                                        REG, PARAM_DELIMITER,i,PARAM_DELIMITER,
                                        ((regread >> i) 
                                                & 0b00000001),
                                        CMD_DELIMITER);
                        *regp_old ^= (0x1 << i) ;
                        return 0;
                }
        }

        return -1;
}

int register_dump(){
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINA ^ (0x1 << i);
                process_update('A',&PINA, &reg);
        }
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINB ^ (0x1 << i);
                process_update('B',&PINB, &reg);
        }
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINC ^ (0x1 << i);
                process_update('C',&PINC, &reg);
        }
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PIND ^ (0x1 << i);
                process_update('D',&PIND, &reg);
        }
        
        /* Register E is disabled, due to the uart register beeing in it
         * for(int i = 0; i < 8; i ++){
                uint8_t reg = PINE ^ (0x1 << i);
                process_update('E',&PINE, &reg);
        }*/
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINF ^ (0x1 << i);
                process_update('F',&PINF, &reg);
        }
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PING ^ (0x1 << i);
                process_update('G',&PING, &reg);
        }
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINH ^ (0x1 << i);
                process_update('H',&PINH, &reg);
        }
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINJ ^ (0x1 << i);
                process_update('J',&PINJ, &reg);
        }
        
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINK ^ (0x1 << i);
                process_update('K',&PINK, &reg);
        }
        for(int i = 0; i < 8; i ++){
                uint8_t reg = PINL ^ (0x1 << i);
                process_update('L',&PINL, &reg);
        }

        return 0;
}
