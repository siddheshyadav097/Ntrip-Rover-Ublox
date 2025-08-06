/**
 *  @file          :  mem_driver.h
 *  @author        :  Vishnu
 *  @date          :
 *  @brief         :  Brief description
 *  @filerevision  :  2.0
 *  
 */
 
#ifndef __MEMORYDRIVER_H
#define __MEMORYDRIVER_H

#include <stdint.h>

#define	SST26VF064B
    
         //Instruction set for SST26VF064B (64Mbit memory)
	#ifdef	SST26VF064B
		#define	MEM_READ				0x03	//Read data from memory
		#define	MEM_4KB_ERASE	        		0x20	
		#define	MEM_64_32_8_KB_ERASE			0xD8
		#define	MEM_CHIP_ERASE				0xC7	//0x60 or 0xC7 will erase full memory array
		#define	MEM_PAGE_PROGRAM			0x02
		#define MEM_RDSR 				0x05	//Read-Status-Register	
		#define MEM_EWSR				0x50	//Enable-Write-Status-Register
		#define MEM_WRSR				0x01	//Write-Status-Register
		#define MEM_WREN				0x06	//Write-Enable
		#define MEM_WRDI				0x04	//Write-Disable#define MEM_EBSY					0x70	//Enable SO to output RY/BY# status during AAI programming#define MEM_DBSY					0x80	//Disable SO as RY/BY# status during AAI programming#define MEM_RDID			        	0x90	//0x90 or 0xAB read chip ID
    #define MEM_JEDEC_ID				0x9F	//JEDEC ID read
    #define MEM_ULBPR                               0x98    //Global Block Protection Unlock
		#define MEM_BUSY_BIT				0x01//0x80	//0000 0001 lsb of status register indicates busy status for winbon memory  change it to 0x01 works for microchip mem also
		
		#define MEM_4KB_SIZE				4096	//block size occupied by 4kb used for erase memory
		#define MEM_32KB_SIZE				32768	//block size occupied by 32kb used for erase memory
		#define MEM_64KB_SIZE				65536	//block size occupied by 64kb used for erase memory
		
		#define MEM_4KB_MAX_BLOCK_COUNT			2048		//block size occupied by 64kb used for erase memory
		#define MEM_32KB_MAX_BLOCK_COUNT		256	//block size occupied by 64kb used for erase memory
		#define MEM_64KB_MAX_BLOCK_COUNT		128	//block size occupied by 64kb used for erase memory

                #define PAGE_SIZE			        256

	//Instruction set for AT25DF641A
//	#elif defined	AT25DF641A
//		#define MEM_READ				0x03
//		#define MEM_4KB_ERASE			        0x20
//		#define MEM_32KB_ERASE			        0x52
//		#define MEM_64KB_ERASE			        0xD8
//		#define MEM_CHIP_ERASE				0xC7
//		#define MEM_PAGE_PROGRAM			0x02
//		#define MEM_WREN				0x06
//		#define MEM_WRDI				0x04
//		#define MEM_RDSR				0x05
//		#define MEM_WRSR				0x01
//		#define MEM_RDID				0x9F
//		#define MEM_BUSY_BIT	        		0x01
//		
//		#define PAGE_SIZE			        256
//		
//		#define MEM_4KB_SIZE				4096	//block size occupied by 4kb used for erase memory
//		#define MEM_32KB_SIZE				32768	//block size occupied by 32kb used for erase memory
//		#define MEM_64KB_SIZE				65536	//block size occupied by 64kb used for erase memory
//		
//		#define MEM_4KB_MAX_BLOCK_COUNT			127		//block size occupied by 64kb used for erase memory
//		#define MEM_32KB_MAX_BLOCK_COUNT		15	//block size occupied by 64kb used for erase memory
//		#define MEM_64KB_MAX_BLOCK_COUNT		7	//block size occupied by 64kb used for erase memory
//                #define	MEM_64_32_8_KB_ERASE				0xD8
//
//	//Instruction set for SST25VF040B (4Mbit memory)
//	#elif	SST25VF040B
//		#define	MEM_READ					0x03	//Read data from memory
//		#define	MEM_READ_HS					0x0B	// Read data from memory at high speed
//		#define	MEM_4KB_ERASE				0x20	
//		#define	MEM_32KB_ERASE				0x52
//		#define	MEM_64KB_ERASE				0xD8
//		#define	MEM_CHIP_ERASE					0x60	//0x60 or 0xC7 will erase full memory array
//		#define	MEM_BYTE_WRITE					0x02
//		#define	MEM_AAI_WRITE					0xAD	//Auto Address Increment Programming
//		#define MEM_RDSR 					0x05	//Read-Status-Register	
//		#define MEM_EWSR					0x50	//Enable-Write-Status-Register
//		#define MEM_WRSR					0x01	//Write-Status-Register
//		#define MEM_WREN					0x06	//Write-Enable
//		#define MEM_WRDI					0x04	//Write-Disable
//		#define MEM_EBSY					0x70	//Enable SO to output RY/BY# status during AAI programming
//		#define MEM_DBSY					0x80	//Disable SO as RY/BY# status during AAI programming
//                #define MEM_RDID			        	0x90	//0x90 or 0xAB read chip ID
//                #define MEM_JEDEC_ID					0x9F	//JEDEC ID read
//
//		#define MEM_BUSY_BIT					0x01	//0000 0001 lsb of status register indicates busy status
//		
//		#define MEM_4KB_SIZE					4096	//block size occupied by 4kb used for erase memory
//		#define MEM_32KB_SIZE					32768	//block size occupied by 32kb used for erase memory
//		#define MEM_64KB_SIZE					65536	//block size occupied by 64kb used for erase memory
//		
//		#define MEM_4KB_MAX_BLOCK_COUNT			127		//block size occupied by 64kb used for erase memory
//		#define MEM_32KB_MAX_BLOCK_COUNT		15	//block size occupied by 64kb used for erase memory
//		#define MEM_64KB_MAX_BLOCK_COUNT		7	//block size occupied by 64kb used for erase memory
//
//	//Instruction set for AT25DF041A
//	#elif defined	AT25DF041A
//		#define	MEM_READ	0x03	//0x3 //0000 0011 Read data from memory array begining at selected address
//		#define	MEM_WRITE	0x02	//0000 0010 Write data tomemory array beginning at selected address
//		#define	MEM_WRENL	0x06	//0000 0110 Set the write enable latch
//		#define	MEM_WRDI	0x04	//0000 0100 Reset the write enable latch 
//		#define	MEM_RDSR	0x05	//0000 0101 Read status register
//		#define	MEM_WRSR	0x01	//0000 0001 Write status register
//		#define	MEM_SE		0xD8	//64k sectors 0101 0010 Sector Erage- one sector in memory array for atmel (erase atmel)
//		#define	MEM_CE		0x60	//
//		#define MEM_BE		0x20	// Block Erase
//		
//		
//	//Instruction set for AT25F4096	
//	#elif defined AT25F4096
//		#define	MEM_READ	0x03	//0000 0011 Read data from memory array begining at selected address
//		#define	MEM_WRITE	0x02	//0000 0010 Write data tomemory array beginning at selected address	#define	MEM_WRENL	0x06	//0000 0110 Set the write enable latch
//		#define	MEM_WRDI	0x04	//0000 0100 Reset the write enable latch 
//		#define	MEM_RDSR	0x05	//0000 0101 Read status register
//		#define	MEM_WRSR	0x01	//0000 0001 Write status register
//		#define	MEM_SE		0x52	//64k sectors 0101 0010 Sector Erage- one sector in memory array for atmel (erase atmel)
//		#define	MEM_CE		0x62	//
	#endif

void InitiliaiseSPIMemoryDriver();
void WaitIfFlashBusy();
void WriteBlock(uint8_t *dataArray, uint32_t startAddress, uint16_t TotalCount);
void ReadBlock(uint8_t *readBuffer, uint32_t StartAddress, uint16_t count);
void FlashWriteByte(uint8_t data, uint32_t address);
uint8_t FlashReadByte(uint32_t address);
void EraseBlock4KB(uint16_t blockNumber);
void EraseBlock32KB(uint16_t blockNumber);
void EraseBlock64KB(uint8_t blockNumber);
void WholeChipErase();

#endif

