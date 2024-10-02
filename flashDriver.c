/*
 * flashDriver.c
 *
 *  Created on: Sep 23, 2024
 *      Author: PANAGIOTIS
 */

/* This driver was written to support stm32f405rgt6 chip and
 * Flash manipulation over SPI (w25n01gvxxig/it).
 *
 * Revision 0.1 : 28/9/2024
 *
 * Panagiotis Xydias
 *
 */

#include "flashDriver.h"


uint8_t sendCMD(uint8_t byte){
    uint8_t ret;
    HAL_SPI_TransmitReceive(FLASH_SPI, &byte, &ret, 1, HAL_MAX_DELAY);
	return ret;
}


uint32_t getFlashID(void){
	uint32_t ret[3];
	uint32_t R;
	Chip_Select();
	sendCMD(JEDEC_ID_CMD);
	sendCMD(DUMMY_BYTE);
	ret[0] = (uint32_t) sendCMD(DUMMY_BYTE);
	ret[1] = (uint32_t) sendCMD(DUMMY_BYTE);
	ret[2] = (uint32_t) sendCMD(DUMMY_BYTE);
	R = (ret[0] << 16) | (ret[1] << 8) | ret[2];
	HAL_Delay(100);
	Chip_Deselect();
	return R;
}


uint8_t checkStatusReg(uint8_t status_addr){
	uint32_t ret;
	Chip_Select();
	sendCMD(READ_STATUS_REG_CMD);
	sendCMD(status_addr);
	ret = sendCMD(DUMMY_BYTE);
	HAL_Delay(100);
	Chip_Deselect();
	return ret;
}


void BUFdisable(void){
	uint8_t tempConfReg;
	waitForNotBusy();
	tempConfReg = checkStatusReg(CONF_REG_ADDRESS);
	tempConfReg &= ~(1 << 3);
	Chip_Select();
	sendCMD(WRITE_STATUS_REG_CMD);
	sendCMD(CONF_REG_ADDRESS);
	sendCMD(tempConfReg);
	HAL_Delay(100);
	Chip_Deselect();
}


void waitForNotBusy(void){
	uint8_t BusyBitCheck;
	do{
		statReg = checkStatusReg(STATUS_REG_ADDRESS);
		BusyBitCheck = (statReg & 0x01);
	}while(BusyBitCheck == 0x01);
}


void waitForNotWEL(void){
	uint8_t WELBitCheck;
	do{
		statReg = checkStatusReg(STATUS_REG_ADDRESS);
		WELBitCheck = (statReg & 0x02);
	}while(WELBitCheck == 0x02);
}


void resetFlash(void){
	waitForNotBusy();
	Chip_Select();
	sendCMD(DEVICE_RESET_CMD);
	HAL_Delay(100);
	Chip_Deselect();
}


void enableWriteFlash(void){
	waitForNotBusy();
	Chip_Select();
	sendCMD(WRITE_ENABLE_CMD);
	Chip_Deselect();
	do{
		statReg = checkStatusReg(STATUS_REG_ADDRESS);
	}while((statReg & 0x02) != 0x02);
}


void initFlash(void){
	flashID = getFlashID();
	protReg = checkStatusReg(PROT_REG_ADDRESS);
	confReg = checkStatusReg(CONF_REG_ADDRESS);
	statReg = checkStatusReg(STATUS_REG_ADDRESS);
}


void blockErase(uint32_t addr){
	uint16_t page_addr = ADDR_TO_PAGE(addr);
	waitForNotBusy();
	enableWriteFlash();
	Chip_Select();
	sendCMD(BLOCK_ERASE_CMD);
	sendCMD(DUMMY_BYTE);
	sendCMD((page_addr & 0xFF00) >> 8);
	sendCMD(page_addr & 0xFF);
	Chip_Deselect();
	waitForNotWEL();
}


uint8_t writeFlashByte(uint8_t byte, uint16_t addr){
	uint16_t column_addr = ADDR_TO_COLUMN(addr);
	uint16_t page_addr = ADDR_TO_PAGE(addr);
	waitForNotBusy();
	enableWriteFlash();
	waitForNotBusy();
	Chip_Select();
	sendCMD(PROGRAM_DATA_LOAD_CMD);
	sendCMD((column_addr & 0xFF00) >> 8);
	sendCMD(column_addr & 0xFF);
	sendCMD(byte);
	Chip_Deselect();

	waitForNotBusy();
	Chip_Select();
	sendCMD(PROGRAM_EXECUTE_CMD);
	sendCMD(DUMMY_BYTE);
	sendCMD((page_addr & 0xFF00) >> 8);
	sendCMD(page_addr & 0xFF);
	Chip_Deselect();
	waitForNotWEL();

	return HAL_OK;
}


uint8_t readFlashByte(uint16_t addr){
	uint8_t retByte;
	uint16_t column_addr = ADDR_TO_COLUMN(addr);
	uint16_t page_addr = ADDR_TO_PAGE(addr);
	waitForNotBusy();
	Chip_Select();
	sendCMD(PAGE_DATA_READ_CMD);
	sendCMD((page_addr & 0xFF00) >> 8);
	sendCMD(page_addr & 0xFF);
	Chip_Deselect();

	waitForNotBusy();
	Chip_Select();
	sendCMD(FAST_READ_CMD);
	sendCMD((column_addr & 0xFF00) >> 8);
	sendCMD(column_addr & 0xFF);
	sendCMD(DUMMY_BYTE);
	retByte = sendCMD(DUMMY_BYTE);
	Chip_Deselect();

	return retByte;
}


uint8_t writeFlashPage(uint8_t *buf, uint32_t addr, uint16_t buf_len){
    int i;
	uint16_t column_addr = ADDR_TO_COLUMN(addr);
	uint16_t page_addr = ADDR_TO_PAGE(addr);
	waitForNotBusy();
	enableWriteFlash();
	waitForNotBusy();
	Chip_Select();
	sendCMD(PROGRAM_DATA_LOAD_CMD);
	sendCMD((column_addr & 0xFF00) >> 8);
	sendCMD(column_addr & 0xFF);
	for(i=0;i<buf_len;i++){
		sendCMD(buf[i]);
	}
	Chip_Deselect();

	waitForNotBusy();
	Chip_Select();
	sendCMD(PROGRAM_EXECUTE_CMD);
	sendCMD(DUMMY_BYTE);
	sendCMD((page_addr & 0xFF00) >> 8);
	sendCMD(page_addr & 0xFF);
	Chip_Deselect();
	waitForNotWEL();

    return HAL_OK;
}


uint8_t readFlashPage(uint8_t *buf, uint32_t addr, uint16_t buf_len){
    int i;
	uint16_t column_addr = ADDR_TO_COLUMN(addr);
	uint16_t page_addr = ADDR_TO_PAGE(addr);
	waitForNotBusy();
	Chip_Select();
	sendCMD(PAGE_DATA_READ_CMD);
	sendCMD((page_addr & 0xFF00) >> 8);
	sendCMD(page_addr & 0xFF);
	Chip_Deselect();

	waitForNotBusy();
	Chip_Select();
	sendCMD(FAST_READ_CMD);
	sendCMD((column_addr & 0xFF00) >> 8);
	sendCMD(column_addr & 0xFF);
	sendCMD(DUMMY_BYTE);
	for(i=0;i<buf_len;i++){
		buf[i] = sendCMD(DUMMY_BYTE);
	}
	Chip_Deselect();

    return HAL_OK;
}





