/**
 *  @file          :  mem_port.c
 *  @author        :  Vishnu
 *  @date          :
 *  @brief         :  Brief description
 *  @filerevision  :  2.0
 *  
 */

#ifndef __MEMORYAPI_H
#define __MEMORYAPI_H

#include <stdint.h>
#include "memorydriver.h"


#define PACKET_HEADER_LEN			    2		// 1 readByte + 1 writeByte


#define WRITE_SIGNATURE				    0xAA 	// used to differentiate between a blank packet and valid packet



#define  PACKET_STORAGE_START_ADDR		0x080000	                            //should be start address of a 4kb block	
#define  PACKET_STORAGE_END_ADDR 		0x7DFFFF	

#define  PACKET_STORAGE_START_BLOCK      (PACKET_STORAGE_START_ADDR / MEM_4KB_SIZE)
typedef enum
{
	BLANK,
	SENT,
	NOT_SENT,
	INVALID_LOCATION
}packetStatus_t;


uint32_t GetNumPacketsInMem();
void WritePacketToMem(uint8_t *data);
void ReadPacketFromMem(uint8_t *data);
void DeleteSentPacket();
void InitMemory(void);
void resetMemory(void);
//float GetMemUsedInPercent(void);



#endif