
/**
 *  @file          :  memoryapi.c
 *  @author        :  Vishnu
 *  @date          :
 *  @filerevision  :  2.0
 *  @brief         :  
 *  
 *  Each packet is appended with 1 readByte + 1 writeByte (writeByte is a signature, readByte is used to determine whether a packet has been sent or not)
 *  
 *  
 *  | readByte | writeByte  | .....................................data....................2 byte checksum |
 *  
 *  | __PACKET_HEADER_LEN__ | _________________________________ packetDataLength _________________________ |
 *
 *  | ____________________________________________ totalPacketLength _____________________________________ |
 *
 *	Checksum is a part of data
 *  
 *  During init program scans through all the packet headers to find out locationToBeWritten and numPacketsInMem 
 */
 
 
   
   
   
 
#include "mem_packet_api.h"
#include "mem_config_api.h"
#include "memorydriver.h"
#include "memoryport.h"
					   
#include "qmath.h"
#include "packet_api.h"
#include "stm32g0xx_hal.h"
#include <string.h>
#include "qtimespent.h"

#include "main.h"       

 
/********************************************************* PACKETS SECTION ********************************************************/

uint32_t packetDataLength = 0;
uint32_t totalPacketLength = 0;
uint8_t headerBuff[PACKET_HEADER_LEN];

uint32_t numPacketsInMem=0;		//number of packets remaining to be sent to the server 
uint32_t locationToBeWritten = 0xFFFFFF;		//initialise with an invalid packet number 
uint32_t currentReadLocationNo = 0;
uint8_t newPacketAfterLastReadFlag = 1;
uint32_t packetDataSize;
uint32_t maxPacketsInMem=0;

static packetStatus_t ReadPacketHeader(uint32_t pktLocationNo);
static void EraseBlockContainingHeader(uint32_t headerNo);
static void UpdateNumPacketsInMem();

uint32_t currentLocationNo = 0;
uint32_t blankCount = 0;
uint8_t firstLocBlankFlag = 0;
uint8_t validHeaderFlag = 0;
packetStatus_t packetStatus;

void ResetMemCntr(void)
{
   currentLocationNo = 0;
   blankCount = 0;
   firstLocBlankFlag = 0;
   validHeaderFlag = 0;
   numPacketsInMem=0;
   locationToBeWritten = 0xFFFFFF;
   currentReadLocationNo = 0;
   newPacketAfterLastReadFlag = 1;
}
void GetWriteIndex()
{
    for(currentLocationNo=0; currentLocationNo < maxPacketsInMem; currentLocationNo++)
    {
        packetStatus = ReadPacketHeader(currentLocationNo);		//reads the packet headers corresponding to the specified packet number
        
        switch(packetStatus)
        {
            case BLANK:	 
                          blankCount++;	//this will be used to check if all packet locations are blank

                          if(currentLocationNo == 0)
                          {
                              firstLocBlankFlag = 1;
                          }
                          else if((locationToBeWritten == 0xFFFFFF) && (firstLocBlankFlag != 1))	//to ensure lastStoredLocationNo is updated only once while looping
                          {
                              locationToBeWritten = currentLocationNo;
                          }
                          else if ((firstLocBlankFlag==1) && (validHeaderFlag==1))
                          {
                                //enters here if first location is blank and a blank location is found again at the end of memory 
                                locationToBeWritten = currentLocationNo;
                          }
                          break;
            
            
            case NOT_SENT:  
                            numPacketsInMem++;	
                            validHeaderFlag = 1;	//indicates that packet is not blank
                            break;
            
            case SENT:	
                        validHeaderFlag = 1;		//indicates that packet is not blank
                        break;
            
            case INVALID_LOCATION:    //should never get here
									   __nop();
                                      break;
                
        }
    }
    if(blankCount == maxPacketsInMem)
    {
        //all the packet locations are blank
        locationToBeWritten = 0;
        numPacketsInMem = 0;
    }
    else if((firstLocBlankFlag == 1) && (locationToBeWritten == 0xFFFFFF))
    {
        //enters here when first loc is blank and blank spaces are not found after valid locations (packets present till last location) 
        locationToBeWritten = 0;   
    }
}


/*
*	pktLocationNo is an index given to each location reserved for packets in the memory 
*   (this depends on the memory size and partitions)
*		     memory is divided into maxPacketsInMem  (pktLocationNo starts from 0 to maxPacketsInMem)
*/
packetStatus_t ReadPacketHeader(uint32_t pktLocationNo)
{
    uint32_t readAddr;
    
    if(pktLocationNo < maxPacketsInMem)
    {
        readAddr =  ((pktLocationNo * totalPacketLength) + PACKET_STORAGE_START_ADDR);
        
        ReadBlock(headerBuff, readAddr, PACKET_HEADER_LEN);	
        
        if(headerBuff[1] == 0xFF)
        {
            return BLANK;
        }
        else if(headerBuff[0] == 0xFF)
        {
            return NOT_SENT;
        }
        else
        {
            return SENT;
        }        
    }
    else
    {
        return INVALID_LOCATION;	//should never get here
    }
}

uint32_t GetNumPacketsInMem()
{
    return numPacketsInMem;
}



/**
 *  reads the packet header for next block.
 *  If the packet header is blank, do nothing
 *  else erase the next block
 */
void EraseNextBlockIfReqd(uint32_t location)
{
    packetStatus_t status;
    
    if(location == (maxPacketsInMem-1)) 
    {
//        LOG_DBG(CH_GSM,"loaction to be written , %d= max packet - 1 ,%d",location,maxPacketsInMem-1);
        status = ReadPacketHeader(0);	//read the next packetHeader
        if(status != BLANK)
        {
            EraseBlockContainingHeader(0);	
        }
    }
    else
    {
        status = ReadPacketHeader(location+1);		//read the next packetHeader
        if(status != BLANK)
        {
//            LOG_DBG(CH_GSM,"status != BLANK,location to be erased = %d",location);
            EraseBlockContainingHeader(location+1);	
        }
    }
}

void EraseBlockContainingHeader(uint32_t headerNo)
{
    uint16_t blockNo;
    uint32_t addr;
    
    addr = (((headerNo*totalPacketLength)+PACKET_STORAGE_START_ADDR)+1);
    //+1 is failsafe incase last byte of header is in the next block
	
    blockNo = addr / MEM_4KB_SIZE;
    EraseBlock4KB(blockNo);
    
    UpdateNumPacketsInMem();
}

void UpdateNumPacketsInMem()
{
    uint32_t tempNumPackets=0;
    packetStatus_t status;   	
    uint32_t i;
    
    //read all the packet headers (readBytes) to update numPacketsInMem
    //this is done since Erasing Blocks has a possibility of erasing packets that are NOT_SENT
    for(i=0; i<maxPacketsInMem; i++)
    {
        status = ReadPacketHeader(i);
        if(status == NOT_SENT)
        {
            tempNumPackets++;
        }
    }
    numPacketsInMem = tempNumPackets;	
}
uint32_t writeAddress = 0;
uint16_t W_checksum =0;
uint16_t R_checksum =0;//, tempLen=0;

volatile uint8_t readBuff[512];
volatile uint8_t flagSuccess = 0;
volatile uint8_t checkFlag=0;

void WritePacketToMem(uint8_t *data)		//length is not required since it is kept fixed		
{
    flagSuccess = 0;
  
 
    if(locationToBeWritten < maxPacketsInMem)
    {
        EraseNextBlockIfReqd(locationToBeWritten);
        
//        LOG_DBG(CH_GSM,"locationToBeWritten = %d",locationToBeWritten);
        
      //headerBuff[0] = 0xFF;			// leave the location untouched (will only be edited when the packet is read)
        headerBuff[1] = WRITE_SIGNATURE;
 
        W_checksum = GetCrc16(data, packetDataLength-2);
		
        writeAddress = (locationToBeWritten * totalPacketLength) + PACKET_STORAGE_START_ADDR;
		
        FlashWriteByte(headerBuff[1], (writeAddress+1));	//write only 2nd byte of the header (skip readByte)
        
        WriteBlock(data, writeAddress+PACKET_HEADER_LEN, packetDataLength-2);	//start writing the packet after 2 bytes of header(leave last 2 bytes for checksum)
        WriteBlock((uint8_t *)&W_checksum, writeAddress+totalPacketLength-2, 2);	
 
        //read data and verify?
        
//        ReadBlock((uint8_t *)readBuff,(writeAddress+PACKET_HEADER_LEN),packetDataLength); 
//        R_checksum = 	GetCrc16((uint8_t *)readBuff, packetDataLength-2);
//        if(memcmp(data,(uint8_t *)readBuff,packetDataLength - 2) == 0)
//        {
//            flagSuccess = 1;
//        }
        
        if(locationToBeWritten ==(maxPacketsInMem-1))
        {
            locationToBeWritten = 0;
        }
        else
        {
            locationToBeWritten++;
        }
        numPacketsInMem++;
        newPacketAfterLastReadFlag = 1;
        //ReadPacketFromMem((uint8_t *)readBuff, &tempLen);
    }	
    else
    {
        //invalid address (should never get here)
    }
}

/**
 *  fills the data buffer with whole packet data and header excluding the readByte 
 *  starts scanning from lastStoredLocationNo and complete one circular cycle
 *  searches for 0xFF in readByte. The location found first will be sent 
 *  
 *  
 *  DeleteSentPacket() has to be called once the packet has been sent successfully in order to clear the readByte
 */

void ReadPacketFromMem(uint8_t *data)	//length is fixed
{
    if(newPacketAfterLastReadFlag == 1)
    {
        /*
          the starting position for scanning is updated if there has been a new write
          otherwise scan will start from whatever was the previous read location
         */
        if(locationToBeWritten ==0)
        {
            currentReadLocationNo = (maxPacketsInMem-1);
        }
        else
        {
            currentReadLocationNo = (locationToBeWritten - 1);
        }
        newPacketAfterLastReadFlag = 0;
    }
//    LOG_DBG(CH_GSM,"currentReadLocationNo = %d",currentReadLocationNo);
    
    uint32_t addr;
    packetStatus_t status;
    uint32_t i;
    
    for(i=0; i<maxPacketsInMem; i++)	//scan one circular cycle
    {   
        addr = (currentReadLocationNo*totalPacketLength)+PACKET_STORAGE_START_ADDR;
        
        if(FlashReadByte(addr) == 0xFF)		//readbyte is stored at the first location
        {
            status = ReadPacketHeader(currentReadLocationNo);
            if(status == NOT_SENT)
            {
                ReadBlock(data,(addr+PACKET_HEADER_LEN) ,packetDataLength); //read the complete packet excluding header
                //currentReadLocationNo is maintained as it is and will be used in DeleteSentPacket()
								R_checksum = 	GetCrc16(data, packetDataLength-2);
                if(GetCrc16(data, packetDataLength) == 0)       //o indicates valid checksum
                {
                    
                    break;
                }
                else
                {
                    //checksum error actions to be handled here
                    
                    //clear the readByte for that packet and decrement currentReadLocationNo
									__nop();
                  DeleteSentPacket();
                    // continue reading next packets until a packet with valid checksum is found
                }
            }
            else if((status == BLANK) || (status == NOT_SENT))
            {
                if(currentReadLocationNo == 0)
                {
                    currentReadLocationNo = maxPacketsInMem-1;	//loop around and go to last packet
                }
                else
                {
                    currentReadLocationNo--;
                  //currentReadLocationNo = currentReadLocationNo + 1;
                }	
            }
            else
            {
              //Never come here
            }
        }
        else
        {
            if(currentReadLocationNo == 0)
            {
                currentReadLocationNo = maxPacketsInMem-1;	//loop around and go to last packet
            }
            else
            {
                currentReadLocationNo--;
              //currentReadLocationNo = currentReadLocationNo + 1;
            }	
        }
    }
    
}

/*
 *	Clears readByte for packet corresponding to currentReadLocationNo  
 * 	Also, updates the currentReadLocationNo for next read
 */
void DeleteSentPacket()
{
    uint32_t addr = 0;
    addr = (currentReadLocationNo*totalPacketLength)+PACKET_STORAGE_START_ADDR;
    FlashWriteByte(0x00,addr);	//declare that the packet has been sent
    if(numPacketsInMem >0)
    {
        numPacketsInMem--;
    }
    
    if(currentReadLocationNo == 0)
    {
        currentReadLocationNo = maxPacketsInMem-1;	//loop around and go to last packet
    }
    else
    {
        currentReadLocationNo--;
      //currentReadLocationNo = currentReadLocationNo + 1;
			
    }	
}

void InitMemory(void)
{
    InitiliaiseSPIMemoryDriver();
	
	  //WholeChipErase();
            
    packetDataLength = GetSizeOfPacket();                       //GetSizeOfPacket() to be defined by user
    totalPacketLength = packetDataLength + PACKET_HEADER_LEN;
    maxPacketsInMem = ((PACKET_STORAGE_END_ADDR - PACKET_STORAGE_START_ADDR)/totalPacketLength);
    
    GetWriteIndex();

    ReadAllConfigParamsFromFlash();	
}

void resetMemory(void)
{
    uint16_t i =0;
    for(i=PACKET_STORAGE_START_BLOCK; i < MEM_4KB_MAX_BLOCK_COUNT  ; i++)
    {
       EraseBlock4KB(i);
       FeedWatchdog();
    }
    
    ResetMemCntr();
    packetDataLength = GetSizeOfPacket();                       //GetSizeOfPacket() to be defined by user
    totalPacketLength = packetDataLength + PACKET_HEADER_LEN;
    maxPacketsInMem = ((PACKET_STORAGE_END_ADDR - PACKET_STORAGE_START_ADDR)/totalPacketLength);
    GetWriteIndex();
}

//float GetMemUsedInPercent(void)
//{
//  return((numPacketsInMem / maxPacketsInMem) * 100);
//
//}
	
// uint32_t configStartAddress = MEM_4KB_SIZE * MEM_CONFIG_BLOCK_NUMBER;		//test code

// uint8_t  memByte, mfidData[4] = {0};
// uint8_t  memData[200] = {0};
// uint8_t  dummy[200] = {0};
// uint16_t j;
// uint8_t dataToWr=0;
// uint8_t addrToWr=0;

// extern UART_HandleTypeDef huart_debug;		

// uint8_t blockEraseCmd=0;
// uint8_t chipEraseCmd =0;
// uint32_t wrStrt=0;
// uint32_t wrEnd=0;
// uint32_t currentAddr = 0;
// uint8_t writeReqdFlag = 0;

// void InitMemory()		
// {
    // InitiliaiseSPIMemoryDriver();
        
    // CLR_CS();
    // SpiTransmitByte(MEM_JEDEC_ID);              //enable memory write (required for program/erase)
    // memData[0]=SpiReceiveByte();
    // memData[1]=SpiReceiveByte();
    // memData[2]=SpiReceiveByte();
    // memData[3]=SpiReceiveByte();
    // SET_CS();
    // WaitIfFlashBusy();
    
    

    // if(blockEraseCmd == 1)
    // {
      // EraseBlock64KB(0);
      // blockEraseCmd =0;
    // }
    // else if(chipEraseCmd == 1)
    // {
      // WholeChipErase();
      // chipEraseCmd =0;
    // }

    // if(writeReqdFlag == 1)
    // {
        // wrStrt = GetStartTime();
        // for(j=0;j<60;j++)
        // {
            // WriteBlock(dummy,(j*5),5);
        // }
        // writeReqdFlag=0;
        // wrEnd = GetStartTime();
    // }

    // //FlashWriteByte(dataToWr,addrToWr);
    // //WriteBlock(dummy,0,200);

    // currentAddr=0;
    // while(1)
    // {
      
      // if((currentAddr+200) >= 0x7FFFFF)
      // {
        // ReadBlock(memData, currentAddr, (0x7FFFFF - currentAddr));
        // HAL_UART_Transmit(&huart_debug, memData, (0x7FFFFF - currentAddr), 90000);
        // break;
      // }
      // else
      // {
        // ReadBlock(memData, currentAddr, 200);
        // HAL_UART_Transmit(&huart_debug, memData, 200, 90000);
        // currentAddr += 200;
      // }
      
    // }

   // // ReadmemConfigParametersFromFlash();
    // //GetWriteIndex();

// }




/********************************************************* OTA SECTION ********************************************************/

// uint32_t otaFileWriteIndex = 0; //this indicates the current index of OTAfile being written

// /**
 // *  This function has to be called whenever a new write is initiated. 
 // */
// void EraseOTAFile()
// {
    // EraseBlock64KB(126);
    // EraseBlock64KB(127);
    
    // otaFileWriteIndex = 0;
// }

// void WriteOTAFile(uint8_t *data, uint16_t length)
// {
    // if((OTA_FILE_START_ADDR + otaFileWriteIndex + length) <= OTA_FILE_END_ADDR)
    // {
        // WriteBlock(data,(OTA_FILE_START_ADDR + otaFileWriteIndex) ,length);
        // otaFileWriteIndex += length;		
    // }
    // else
    // {
        // //should never get here
    // }
// }

// void ReadOTAFile(uint8_t * data, uint32_t readOffset, uint16_t length)		//offset has to be maintained in the application 
// {
    // if((readOffset + OTA_FILE_START_ADDR + length) >= OTA_FILE_END_ADDR)
    // {
        // ReadBlock(data, (readOffset+OTA_FILE_START_ADDR), length);		
    // }
// }



