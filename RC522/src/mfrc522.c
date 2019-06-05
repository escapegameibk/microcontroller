/*
 * mfrc522.c
 * 
 * Copyright 2013 Shimon <shimon@monistit.com>
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
 * 
 */
#include "mfrc522.h"
#include "spix.h"
#include "ecproto.h"

#include <string.h>
#include <stdio.h>
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)

struct  mfrc522_dev_t mfrc_devs[] = {
#ifndef SEND_PIN
	{{0,0,0,0},{0,0,0,0},false,false,{'D',2}, false, 0},
#endif
	{{0,0,0,0},{0,0,0,0},false,false,{'D',3}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'D',4}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'D',5}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'D',6}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'D',7}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'B',0}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'B',1}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'B',2}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'C',0}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'C',1}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'C',2}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'C',3}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'C',4}, false, 0},
	{{0,0,0,0},{0,0,0,0},false,false,{'C',5}, false, 0}
	};
#endif

volatile bool mfrc522_to_be_updated = false;

uint8_t mfrc_devcnt = sizeof(mfrc_devs) / sizeof(struct mfrc522_dev_t);

uint8_t keyA_default[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t keyB_default[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

/*
 * Write one byte of data into a register.
 * This library doesn't use the ability to write multiple bytes at once.
 */
static void mfrc522_write(uint8_t reg, uint8_t data){

	/* Pull the chip select line to 0 */
	ENABLE_CHIP();
	/* Function defined in the spix c file */
	spi_transmit((reg<<1)&0x7E);
	spi_transmit(data);
	/* Pull the chip select line to 1 */
	DISABLE_CHIP();

	return;
}

/*
 * Read one byte of data from a register.
 * This library doesn't use the ability to read multiple bytes at once.
 */
uint8_t mfrc522_read(uint8_t reg){

	uint8_t data;
	/* Pull the chip select line to 0 */
	ENABLE_CHIP();
	
	/* Registers are read by first transmitting the address in the standard
         * way, and then transmitting 0x0 and waiting for the reply. A second
	 * register could be transmitted instead of the 0x0, but this is not
	 * used currently.
	 */
	spi_transmit(((reg<<1)&0x7E)|0x80);
	data = spi_transmit(0x00);

	/* Pull the chip select line to 1 */
	DISABLE_CHIP();
	return data;
}
/*
 * Initial device configuration. Set all registers to their desired values,
 * enable device functionallities ans so on... Also performs a device reset.
 */
void mfrc522_init(){

	uint8_t byte;
	mfrc522_reset();
	
	mfrc522_write(TModeReg, 0x8D);
	mfrc522_write(TPrescalerReg, 0x3E);
	mfrc522_write(TReloadReg_1, 30);
	mfrc522_write(TReloadReg_2, 0);
	mfrc522_write(TxASKReg, 0x40);
	mfrc522_write(ModeReg, 0x3D);
	                       
	/* Originally: Turn on antenna
	 * If both carrier signals are turned off, turn them on (bits 0 and 1)
	 */
	byte = mfrc522_read(TxControlReg);
	if(!(byte&0x03))
	{
		mfrc522_write(TxControlReg,byte|0x03);
	}

	return;
}


/*
 * This performs a software reset on the MFRC522.
 * All internal registers are set to their default values, and all currently
 * stored data is flushed.
 */
void mfrc522_reset(){
	mfrc522_write(CommandReg,SoftReset_CMD);
}

/*
make command request to rc522
*/
uint8_t	mfrc522_request(uint8_t req_mode, uint8_t * tag_type){
	uint8_t  status; /* Wether the operation was successfull or not. */
	uint32_t backBits; //The 4  received data bytes
	/* TxLastBists = BitFramingReg[2..0] 
	 *  originally there was a */
	mfrc522_write(BitFramingReg, 0x07);
	
	tag_type[0] = req_mode;
	status = mfrc522_to_card(Transceive_CMD, tag_type, 1, tag_type, &backBits);

	if ((status != CARD_FOUND) || (backBits != 0x10))
	{    
		status = ERROR;
	}
   
	return status;
}

/*
 * MFRC522 command execution. Sends and executes a command.
 */
uint8_t mfrc522_to_card(uint8_t cmd, uint8_t *send_data, uint8_t send_data_len,
	uint8_t *back_data, uint32_t *back_data_len){

	uint8_t status = ERROR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint8_t	tmp;
	uint32_t i;
	
	switch (cmd){
		
		/* Certification cards close */
		case MFAuthent_CMD:
	    		
			irqEn = 0x12;
	    		waitIRq = 0x10;
	    		break;

		/* Transmit FIFO data */
	    	case Transceive_CMD:
	    		irqEn = 0x77;
	    		waitIRq = 0x30;
	    		break;
	    	
		default:
	    		break;
	}
		
	n=mfrc522_read(ComIrqReg);
	mfrc522_write(ComIrqReg,n&(~0x80));//clear all interrupt bits
	n=mfrc522_read(FIFOLevelReg);
	mfrc522_write(FIFOLevelReg,n|0x80);//flush FIFO data
    
	mfrc522_write(CommandReg, Idle_CMD);	//NO action; Cancel the current cmd???

	//Writing data to the FIFO
	for (i=0; i<send_data_len; i++){
		mfrc522_write(FIFODataReg, send_data[i]);    
	}

	//Execute the specified command
	mfrc522_write(CommandReg, cmd);
	if (cmd == Transceive_CMD){
		    
		n=mfrc522_read(BitFramingReg);
		mfrc522_write(BitFramingReg,n|0x80);  
	}   
    
	/* Waiting to receive data to complete
	 * i according to the clock frequency adjustment, the operator M1 card 
	 * maximum waiting time 25ms???
	 * Notes:
	 * tyrolyean:
	 *	What is that comment? I believe he calculated the timeout via the
	 *	oszillator frequency on the usual mifare cards...
	 */

	i = 2000;	
	do{
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = mfrc522_read(ComIrqReg);
		i--;

		/* Don't exit the loop unless data has successfully been 
		 * received, or the timeout has run out */
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	tmp=mfrc522_read(BitFramingReg);
	mfrc522_write(BitFramingReg,tmp&(~0x80));
	
	if (i != 0){
		    	
		//BufferOvfl Collerr CRCErr ProtecolErr
		if(!(mfrc522_read(ErrorReg) & 0x1B)){

			status = CARD_FOUND;
			if (n & irqEn & 0x01){
				//??
				status = CARD_NOT_FOUND;
			}

			if (cmd == Transceive_CMD){

				n = mfrc522_read(FIFOLevelReg);
				lastBits = mfrc522_read(ControlReg) & 0x07;
				if (lastBits){

					*back_data_len = (uint32_t)(n-1)*8 + 
						(uint32_t)lastBits;   
				}else{

					*back_data_len = (uint32_t)n*8;   
				}

				if (n == 0){
					n = 1;    
				}

				if (n > MAX_LEN){
					n = MAX_LEN;   
				}
				
				//Reading the received data in FIFO
				for (i=0; i<n; i++){
					back_data[i] = 
						mfrc522_read(FIFODataReg);    
				}
			}
		}else{   
			status = ERROR;  
		}
        
	}
	
    //SetBitMask(ControlReg,0x80);           //timer stops
    //mfrc522_write(cmdReg, PCD_IDLE); 

	return status;
}

/*
 * Request the current card's serial key. 
 */
uint8_t mfrc522_get_card_serial(uint8_t * serial_out){

	uint8_t status;
	uint8_t i;
	uint8_t serNumCheck=0;
	uint32_t unLen;
	//TxLastBists = BitFramingReg[2..0]

	mfrc522_write(BitFramingReg, 0x00);
	serial_out[0] = PICC_ANTICOLL;
	serial_out[1] = 0x20;
	status = mfrc522_to_card(Transceive_CMD, serial_out, 2, serial_out, &unLen);

	if(status == CARD_FOUND){

		// Check card serial number
		for (i=0; i<4; i++){
			   
		 	serNumCheck ^= serial_out[i];
		}

		if (serNumCheck != serial_out[i]){ 
			  
			status = ERROR;    
		}
	}
	return status;
}

/* 
 * set bit mask
 */
void mfrc522_setBitMask(uint8_t reg, uint8_t mask){

	uint8_t tmp;
	tmp = mfrc522_read(reg);
	mfrc522_write(reg, tmp | mask);  // set bit mask
}
/*
 * clear bit mask
 */
void mfrc522_clearBitMask(uint8_t reg, uint8_t mask){

	uint8_t tmp;
	tmp = mfrc522_read(reg);
	mfrc522_write(reg, tmp & (~mask));  // clear bit mask
}

/*
 * calculate crc using rc522 chip
 */
void mfrc522_calculateCRC(uint8_t *pIndata, uint8_t len, uint8_t 
	*pOutData){
	uint8_t i, n;

	mfrc522_clearBitMask(DivIrqReg, 0x04);			//CRCIrq = 0
	mfrc522_setBitMask(FIFOLevelReg, 0x80);			//Claro puntero FIFO
	//Write_MFRC522(CommandReg, PCD_IDLE);

	// Write len bytes from the parameters to the fifo
	for (i=0; i<len; i++){

		mfrc522_write(FIFODataReg, *(pIndata+i));
	}
	mfrc522_write(CommandReg, PCD_CALCCRC);

	// Standby until crc calculation is done
	i = 0xFF;
	do{
		n = mfrc522_read(DivIrqReg);
		i--;

		// Exit condition: CRCIrq = 1 or timeout exceeded
	}while ((i!=0) && !(n&0x04));

	//Read back the CRC
	pOutData[0] = mfrc522_read(CRCResultReg_2);
	pOutData[1] = mfrc522_read(CRCResultReg_1);

	return;
}

/*
 * halt the card (release it to be able to read again)
 */
uint8_t mfrc522_halt(){

	uint8_t status;
	uint32_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	mfrc522_calculateCRC(buff, 2, &buff[2]);

	mfrc522_clearBitMask(Status2Reg, 0x08); // turn off encryption

	status = mfrc522_to_card(Transceive_CMD, buff, 4, buff,&unLen);
	
	return status;
}

/*
 * get reader version, should be either 0x91 or 0x92
 */
uint8_t mfrc522_get_version(){

	return mfrc522_read(VersionReg);
}

/*
 * check if a card is in range
 */
uint8_t mfrc522_is_card(uint16_t *card_type){
	uint8_t buff_data[MAX_LEN],
	status = mfrc522_request(PICC_REQIDL,buff_data);     
	if(status == CARD_FOUND){
		*card_type = (buff_data[0]<<8)+buff_data[1];
		return 1;
	}else{
		return 0;    
	}

	return 0;
}

/* ############################################################################
 * # Section for mifiare card functionality.
 * # 
 * # This section contains functions for use with the mfrc522 in order for it
 * # to work with the standard mifare cards.
 * ############################################################################
 */

/*
 * Function Name : MFRC522_Auth
 * Description : Verify card password
 * Input parameters : authMode - Password Authentication Mode
                 0x60 = A key authentication
                 0x61 = B key authentication
             BlockAddr - block address
             Sectorkey - Sector password
             serNum - card serial number, 4-byte
 * Return value: the successful return CARD_FOUND
 */

uint8_t mfrc522_auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, 
	uint8_t *serNum){

	uint8_t status;
	uint32_t recvBits;
	uint8_t i;
	uint8_t buff[12]; 

	/* Validate instruction block address + sector + password + 
	 * card serial number
	 */

	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i=0; i<6; i++){
		buff[i+2] = *(Sectorkey+i);   
	}
	for (i=0; i<4; i++){
		buff[i+8] = *(serNum+i);   
	}
	status = mfrc522_to_card(PCD_AUTHENT, buff, 12, buff, &recvBits); 
	i = mfrc522_read(Status2Reg);
    
	if ((status != CARD_FOUND) || (!(i & 0x08))){   
		status = ERROR;   
	}
    
	return status;
}

/*
 * Function Name : MFRC522_Write
 * Description : Write block data
 * Input parameters : blockAddr - block address ; writeData - to 16-byte data 
 * block write
 * Return value: the successful return CARD_FOUND
 */
uint8_t mfrc522_write_block(uint8_t blockAddr, uint8_t *writeData){

	uint8_t status;
	uint32_t recvBits;
	uint8_t i;
	uint8_t buff[18]; 
    
	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	mfrc522_calculateCRC(buff, 2, &buff[2]);
	status = mfrc522_to_card(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);  
    
	if ((status != CARD_FOUND) || (recvBits != 4) || 
		((buff[0] & 0x0F) != 0x0A)){   
		
		status = ERROR;   
	}
        
	if (status == CARD_FOUND){
		//?FIFO?16Byte??
		for (i=0; i<16; i++){    
			buff[i] = *(writeData+i);   
		}

		mfrc522_calculateCRC(buff, 16, &buff[16]);
		status = mfrc522_to_card(PCD_TRANSCEIVE, buff, 18, buff, 
			&recvBits);   
        
		if ((status != CARD_FOUND) || (recvBits != 4) || 
			((buff[0] & 0x0F) != 0x0A)){   
			
			status = ERROR;   
		}
	}
    
	return status;
}

/*
 * Function Name : MFRC522_Read
 * Description : Read block data
 * Input parameters : blockAddr - block address ; recvData - read block data
 * Return value: the successful return MI_OK
 */
uint8_t mfrc522_read_block(uint8_t blockAddr, uint8_t *recvData){

	uint8_t status;
	uint32_t unLen;
	
	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	mfrc522_calculateCRC(recvData,2, &recvData[2]);
	status = mfrc522_to_card(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
		
	if ((status != CARD_FOUND) || (unLen != 0x90))
	{
	    status = ERROR;
	}
	
	return status;
}

/*
 * Function Name : MFRC522_SelectTag
 * Description: election card , read the card memory capacity
 * Input parameters : serNum - Incoming card serial number
 * Return value: the successful return of card capacity
 */
uint8_t mfrc522_select_tag(uint8_t *serNum){
	uint8_t i;
	uint8_t status;
	uint8_t size;
	uint32_t recvBits;
	uint8_t buffer[9]; 
		
	buffer[0] = PICC_SELECTTAG;
	buffer[1] = 0x70;

	for (i=0; i<5; i++){

		buffer[i+2] = *(serNum+i);
	}

	mfrc522_calculateCRC(buffer, 7, &buffer[7]);
	status = mfrc522_to_card(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	
	if ((status == CARD_FOUND) && (recvBits == 0x18)){

	    	size = buffer[0]; 
	}else{
		size = 0;    
	}
	
	return size;
}


/* ############################################################################
 * # Section for escape game system functions and subroutines.
 * # 
 * # This section contains functions added in order for this library to perform
 * # escape game system tasks.
 * ############################################################################
 */

/* Check wether a reader is present and store it to the given struct
 */

bool mfrc522_check_forreader( struct mfrc522_dev_t* dev){

	if(dev == NULL){
		return false;
	}else{
		spi_set_cs(dev->pindesc.car, dev->pindesc.pin);
	}
	
	uint8_t curr_read_status = mfrc522_get_version();
	
	if(curr_read_status<0x90 || curr_read_status>0x92){
		/* Reader not really present */
		
		dev->reader_present = false;
		uint8_t dat[sizeof(dev->current_tag)] = {'E', 'R', 'O', 'R'};
		memcpy(dev->current_tag, dat, sizeof(dev->current_tag));
		
		return false;
	}
	
	/* If the IC hasn't been present before, initialize it. */
	if(!dev->reader_present){
		mfrc522_init();
	}

	dev->reader_present = true;
	return true;
}

bool mfrc522_check_forreaders(){

	for(size_t i = 0; i < mfrc_devcnt; i++){
		
		mfrc522_check_forreader(&mfrc_devs[i]);

	}

	return true;

}

void mfrc52_init_readers(){
	
	for(size_t i = 0; i < mfrc_devcnt; i++){
		struct mfrc522_dev_t* dev = &mfrc_devs[i];
		if(!dev->reader_present){
			continue;
		}
		spi_set_cs(dev->pindesc.car, dev->pindesc.pin);
		mfrc522_init();

	}
	
}

void mfrc522_update_tag(struct mfrc522_dev_t* dev){

	if(dev == NULL){
		/* WTF? */
		return;
	}
	if(!mfrc522_check_forreader(dev)){
		/* There isn't really anything here eh? 
		 */
		return;
	}
	
	spi_set_cs(dev->pindesc.car, dev->pindesc.pin);
	uint8_t tag[MFRC_TAGLEN];
	
	bool present = rc522_read_card_id(tag);

	if(!present && (dev->retention_counter++ < RETENTION_COUNT)){
		return;
	}
	dev->current_tag_present = present;
	dev->retention_counter = 0;

	if(dev->current_tag_present){
		memcpy(dev->current_tag, tag, MFRC_TAGLEN-1);
	}else{
		memset(dev->current_tag, 0, MFRC_TAGLEN-1);
	}

	return;
}

void mfrc522_update_tags(){

	for(size_t i = 0; i < mfrc_devcnt; i++){
		
		mfrc522_update_tag(&mfrc_devs[i]);

	}

	return;

}

void mfrc522_save_tag(struct mfrc522_dev_t* dev){

	
	memcpy(dev->last_tag, dev->current_tag, sizeof(dev->last_tag));
	dev->last_tag_present = dev->current_tag_present;


	return;
}

void mfrc522_save_tags(){

	for(size_t i = 0; i < mfrc_devcnt; i++){
		
		mfrc522_save_tag(&mfrc_devs[i]);

	}

	return;

}

uint8_t rc522_read_card_id(uint8_t *card_id){

	uint8_t status, resx = 0;
	uint8_t buff_data[MAX_LEN]; 
	uint16_t is_present = 0;
	
	if(mfrc522_is_card(&is_present)){

		status = mfrc522_get_card_serial(buff_data);
		if (status==CARD_FOUND){
			
			//copy id and checksum in the last byte (5th)
			memcpy(card_id,buff_data,5);
			resx = 1;
		}else{

			resx = 0;
		}
	}else{

		resx = 0;
	}
	
	return resx;
}

/* The write flag inidicates, wether the host should be told what changed or
 * not. Returns the amount of updates.
 */
uint8_t process_mfrc522_update(bool write){

	uint8_t updatecnt = 0;
	
	for(uint8_t i = 0; i < mfrc_devcnt; i++){
		
		struct mfrc522_dev_t* dev = &mfrc_devs[i];
		
		if((memcmp(dev->last_tag, dev->current_tag, 
			sizeof(dev->last_tag)) != 0 ) || 
			(dev->last_tag_present != dev->current_tag_present)){
				
				updatecnt++;	
				if(!write){
					continue;
				}
				
				uint8_t *tag = 
					dev->current_tag;

				uint8_t pay[] = {SPECIALDEV_MFRC522,
					MFRC522_GET_TAG, i,
					dev->current_tag_present,
					tag[0],
					tag[1],
					tag[2],
					tag[3]};

				print_ecp_msg(SPECIAL_INTERACT, pay, 
					sizeof(pay));
			}

	}

	return updatecnt;

}
