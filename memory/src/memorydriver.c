/**
 *  @file          :  mem_driver.c
 *  @author        :  Vishnu
 *  @date          :
 *  @brief         :  Brief description
 *  @filerevision  :  2.0
 *  
 */
 
#include "memorydriver.h"
#include "memoryport.h"
#include "stm32g0xx_hal.h"
#include "qtimespent.h"

//uint8_t testWrite[] = {1,2,3,4,5};
//uint8_t testRead[5] = {0};

void InitiliaiseSPIMemoryDriver()
{
    InitiliaiseSPIMemoryPorts();

    CLR_CS();
    SpiTransmitByte(MEM_WREN);              //enable memory write (required for program/erase)
    SET_CS();
    WaitIfFlashBusy();
    
    CLR_CS();
    SpiTransmitByte(MEM_JEDEC_ID);  
//    FlashReadArray(testRead, 4);
    //enable memory write (required for program/erase)
    SET_CS();
    
    CLR_CS();
    SpiTransmitByte(MEM_WRSR);              //clear all block protect bits in the status register             
    SpiTransmitByte(0x00);
    SpiTransmitByte(0x00);
    SET_CS();
    WaitIfFlashBusy();
   
   //writing to the status register always disables the write enable latch
   //Write enable command has to be given after every status register write
   CLR_CS();
   SpiTransmitByte(MEM_WREN);              //enable memory write (required for program/erase)
   SET_CS();
   WaitIfFlashBusy();
   
   CLR_CS();
   SpiTransmitByte(MEM_ULBPR);              //Global Block Protection Unlock
   SET_CS();
   WaitIfFlashBusy();
    
//    EraseBlock4KB(0);
//    WriteBlock(testWrite,0,5);
//    ReadBlock(testRead,0,5);
    
//    CLR_CS();
//    SpiTransmitByte(MEM_JEDEC_ID);  
//    FlashReadArray(testRead, 4);
//    //enable memory write (required for program/erase)
//    SET_CS();
}

static uint32_t GetCurrentTime(void)
{
    return HAL_GetTick();
}

uint32_t waitStartTime = 0;
uint8_t memoryTimeoutErrorFlag = 0;
uint8_t status = 0;
uint32_t waitCounter = 0;

void WaitIfFlashBusy()
{
    waitStartTime = GetStartTime();
    //uint8_t status = 0;
    do{
        waitCounter++;
          CLR_CS();
          SpiTransmitByte(MEM_RDSR);
          status=SpiReceiveByte();
          SET_CS();
          //Delay(100);
          if(memoryTimeoutErrorFlag)      //not executed in normal state
          {
              HAL_Delay(1);        // once timout has occured, wait for 1 msec in every wait request.        
              break;
          }
          if((GetCurrentTime() - waitStartTime) > 1000)      //exit if stuck in while loop for too long (> 100 msec) 
          {
              memoryTimeoutErrorFlag = 1;
              //errorList[MEMORY_WAIT_TIMEOUT] = 1;            //declare error to be sent to server (continues normal operation)           
             // break;
          }
    }while((status & MEM_BUSY_BIT) == MEM_BUSY_BIT);
               __nop();
           __nop();
		
}


void WriteBlock(uint8_t *dataArray, uint32_t startAddress, uint16_t totalCount)
{
    /**
    *  MEM_PAGE_PROGRAM command allows only max 256 bytes to be written.
    *  if writing starts  from an intermediate position of the page, any data that 
    *  crosses the last address is loops around to the starting address of that page
    *  
    */
    uint8_t address[3];
    uint16_t remainingBytesInPage;
    uint16_t bytesLeft = totalCount;
     
    while(bytesLeft > 0)
    {
        address[2]=startAddress;					//lsb
        address[1]=startAddress >> 8;
        address[0]=startAddress >> 16;				//msb
        
        WaitIfFlashBusy();
        CLR_CS();
        SpiTransmitByte(MEM_WREN);									//enable write mode
        SET_CS();
        WaitIfFlashBusy();
        
        CLR_CS();
        SpiTransmitByte(MEM_PAGE_PROGRAM);							//send write command
        FlashWriteArray(address, 3);								//send address		
        
        remainingBytesInPage = PAGE_SIZE - (startAddress % PAGE_SIZE);	
        
        if(bytesLeft > remainingBytesInPage)
        {
            FlashWriteArray(dataArray, remainingBytesInPage);		//write data
			dataArray += remainingBytesInPage;
            startAddress += remainingBytesInPage;
            bytesLeft -= remainingBytesInPage;
        }
        else
        {
            FlashWriteArray(dataArray, bytesLeft);					//write data
            bytesLeft = 0;
        }
        SET_CS();
        WaitIfFlashBusy();
        
//        CLR_CS();
//        SpiTransmitByte(MEM_WRDI);									//disable write mode
//        SET_CS();
    }
}

void ReadBlock(uint8_t *readBuffer, uint32_t startAddress, uint16_t count)
{ 
    uint8_t address[3];
    address[2]=startAddress;					//lsb
    address[1]=startAddress >> 8;
    address[0]=startAddress >> 16;				//msb
     WaitIfFlashBusy();
    CLR_CS();		
    SpiTransmitByte(MEM_READ);	
    FlashWriteArray(address, 3);							//Start address for memory read
    FlashReadArray(readBuffer, count);						//read desired No. of bytes in readBuffer
    SET_CS();
}

void FlashWriteByte(uint8_t data, uint32_t address)
{
    uint8_t addr[3];
    addr[2]=address;					//lsb
    addr[1]=address >> 8;
    addr[0]=address >> 16;				//msb

          
    WaitIfFlashBusy();
    CLR_CS();
    SpiTransmitByte(MEM_WREN);              //enable memory write (required for program/erase)
    SET_CS();
    WaitIfFlashBusy();
    
    CLR_CS();
    SpiTransmitByte(MEM_PAGE_PROGRAM);
    FlashWriteArray(addr, 3);
    SpiTransmitByte(data);	
    SET_CS();
    WaitIfFlashBusy();

//    CLR_CS();
//    SpiTransmitByte(MEM_WRDI);									//disable write mode
//    SET_CS();
}

uint8_t FlashReadByte(uint32_t address)
{
    uint8_t data = 0;
    
    uint8_t addr[3];
    addr[2]=address;					//lsb
    addr[1]=address >> 8;
    addr[0]=address >> 16;				//msb
        
    WaitIfFlashBusy();
    CLR_CS();
    SpiTransmitByte(MEM_READ);
    FlashWriteArray(addr, 3);
    data = SpiReceiveByte();
    SET_CS();
    
    return data;
}


void EraseBlock4KB(uint16_t blockNumber)
{
    if(blockNumber <= MEM_4KB_MAX_BLOCK_COUNT)
    {
        uint32_t address =(MEM_4KB_SIZE * blockNumber);

        uint8_t addr[3];
        addr[2]=address;					//lsb
        addr[1]=address >> 8;
        addr[0]=address >> 16;				//msb

        CLR_CS();
        SpiTransmitByte(MEM_WREN);              //enable memory write (required for program/erase)
        SET_CS();
        WaitIfFlashBusy();
        
        CLR_CS();
        SpiTransmitByte(MEM_4KB_ERASE);
        FlashWriteArray(addr, 3);
        SET_CS();
        WaitIfFlashBusy();

        if(memoryTimeoutErrorFlag)      //not executed in normal state
        {
            HAL_Delay(100);        // once timout has occured, wait for 100 msec for erase commands.        
        }
    }
}


void EraseBlock32KB(uint16_t blockNumber)
{
    if(blockNumber <= MEM_32KB_MAX_BLOCK_COUNT)
    {
        uint32_t address =(MEM_32KB_SIZE * blockNumber);
        
        WaitIfFlashBusy();
        uint8_t addr[3];
        addr[2]=address;					//lsb
        addr[1]=address >> 8;
        addr[0]=address >> 16;				//msb

        CLR_CS();
        SpiTransmitByte(MEM_WREN);              //enable memory write (required for program/erase)
        SET_CS();
        WaitIfFlashBusy();

        CLR_CS();
        SpiTransmitByte(MEM_64_32_8_KB_ERASE);
        FlashWriteArray(addr, 3);
        SET_CS();
        WaitIfFlashBusy();

        if(memoryTimeoutErrorFlag)      //not executed in normal state
        {
            HAL_Delay(100);        // once timout has occured, wait for 100 msec for erase commands.        
        }
    }
}
uint32_t blockEraseStartTime = 0;
uint32_t blockEraseEndTime = 0;


void EraseBlock64KB(uint8_t blockNumber)
{
    if(blockNumber <= MEM_64KB_MAX_BLOCK_COUNT)
    {
        uint32_t address =(MEM_64KB_SIZE * blockNumber);

        uint8_t addr[3];
        addr[2]=address;					//lsb
        addr[1]=address >> 8;
        addr[0]=address >> 16;				//msb
        
        WaitIfFlashBusy();
        CLR_CS();
        SpiTransmitByte(MEM_WREN);              //enable memory write (required for program/erase)
        SET_CS();
        WaitIfFlashBusy();
        
         blockEraseStartTime = GetCurrentTime();
        CLR_CS();
        SpiTransmitByte(MEM_64_32_8_KB_ERASE);
        FlashWriteArray(addr, 3);
        SET_CS();
        WaitIfFlashBusy();
        blockEraseEndTime = GetCurrentTime();

        if(memoryTimeoutErrorFlag)      //not executed in normal state
        {
            HAL_Delay(100);        // once timout has occured, wait for 100 msec for erase commands.        
        }
    }
}

uint32_t eraseStart=0,eraseEnd=0;
void WholeChipErase()
{
    WaitIfFlashBusy();
    CLR_CS();
    SpiTransmitByte(MEM_WREN);              //enable memory write (required for program/erase)
    SET_CS();
    WaitIfFlashBusy();
     
    CLR_CS();
    SpiTransmitByte(MEM_CHIP_ERASE);
    SET_CS();
    eraseStart = GetStartTime();
    WaitIfFlashBusy();
    eraseEnd = GetStartTime();

    if(memoryTimeoutErrorFlag)      //not executed in normal state
    {
        HAL_Delay(100);        // once timout has occured, wait for 100 msec for erase commands.        
    }
}
