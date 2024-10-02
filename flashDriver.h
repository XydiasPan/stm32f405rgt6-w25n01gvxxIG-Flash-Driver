/*
 * flashDriver.h
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

#ifndef INC_FLASHDRIVER_H_
#define INC_FLASHDRIVER_H_

#include "stm32f4xx.h"
#include "string.h"

extern uint32_t flashID; //w25n01gvxxig/it : 0xEFAA21
extern uint8_t protReg; //Protection register (register-1), contains bit : WP-E
extern uint8_t confReg; //Configuration register (register-2), contains bit : BUF
extern uint8_t statReg; //Status register (register-2), contains bits : WEL, BUSY

extern SPI_HandleTypeDef hspi2;
#define FLASH_SPI &hspi2 //Macro for the SPI handler

#define FLASH_NCS_Pin GPIO_PIN_12 //Chip select GPIO
#define FLASH_NCS_GPIO_Port GPIOB
#define FLASH_NHOLD_Pin GPIO_PIN_6 //HOLD GPIO
#define FLASH_NHOLD_GPIO_Port GPIOC
#define FLASH_NWP_Pin GPIO_PIN_7 //WP(Write Protect) GPIO
#define FLASH_NWP_GPIO_Port GPIOC

//Macros for chip select/de-select to start and end SPI commands
#define Chip_Select() HAL_GPIO_WritePin(FLASH_NCS_GPIO_Port, FLASH_NCS_Pin, GPIO_PIN_RESET);
#define Chip_Deselect() HAL_GPIO_WritePin(FLASH_NCS_GPIO_Port, FLASH_NCS_Pin, GPIO_PIN_SET);

#define FLASH_PAGE_SIZE 2048 //Page size in bytes
#define FLASH_PAGES_PER_BLOCK 64 //64 pages per block
#define FLASH_TOTAL_BLOCKS 1024 //1024 blocks are available

//Macros for converting hex address (uint16_t) to column, page and block addresses
#define ADDR_TO_COLUMN(addr) ((addr) % FLASH_PAGE_SIZE)
#define ADDR_TO_PAGE(addr) (((addr) / FLASH_PAGE_SIZE) & 0xFFFF)

#define DUMMY_BYTE 0xF0 //Dummy byte is declared for transmitting and receiving

//Commands for the w25n01gvxxig/it chip
#define JEDEC_ID_CMD 0x9F
#define DEVICE_RESET_CMD 0xFF
#define BLOCK_ERASE_CMD 0xD8
#define READ_STATUS_REG_CMD 0x0F
#define WRITE_STATUS_REG_CMD 0x1F
#define PROT_REG_ADDRESS 0xA0
#define CONF_REG_ADDRESS 0xB0
#define STATUS_REG_ADDRESS 0xC0
#define WRITE_ENABLE_CMD 0x06
#define WRITE_DISABLE_CMD 0x04
#define PROGRAM_DATA_LOAD_CMD 0x02
#define PROGRAM_EXECUTE_CMD 0x10
#define PAGE_DATA_READ_CMD 0x13
#define READ_CMD 0x03
#define FAST_READ_CMD 0x0B

//Defining functions for usage in main code
uint8_t sendCMD(uint8_t byte);
uint32_t getFlashID(void);
uint8_t checkStatusReg(uint8_t status_addr);
void BUFdisable(void);
void waitForNotBusy(void);
void waitForNotWEL(void);
void resetFlash(void);
void enableWriteFlash(void);
void initFlash(void);
void blockErase(uint32_t addr);
uint8_t writeFlashByte(uint8_t byte, uint16_t addr);
uint8_t readFlashByte(uint16_t page_addr);
uint8_t writeFlashPage(uint8_t *buf, uint32_t addr, uint16_t buf_len);
uint8_t readFlashPage(uint8_t *buf, uint32_t addr, uint16_t buf_len);

#endif /* INC_FLASHDRIVER_H_ */
