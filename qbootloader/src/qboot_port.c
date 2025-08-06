#include "qboot_port.h"
#include "debug_log.h"


uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0,lastPage = 0;
uint32_t Address = 0;
__IO uint32_t MemoryProgramStatus = 0;
__IO uint32_t data32 = 0;

uint32_t errorCode = 0;

uint32_t backupWriteAddress = BACKUP_START_ADDR;
uint32_t PageError = 0;
/*Variable used for Erase procedure*/
 FLASH_EraseInitTypeDef EraseInitStruct = {0};
 
static uint32_t GetPage(uint32_t Address);
static uint32_t GetBank(uint32_t Address);

HAL_StatusTypeDef writeStatus = HAL_OK;

/**
 *  @brief  :Will erase the app memory
 *  @return :none
 */
void QbootFlashAppErase(void)
{
    HAL_FLASH_Unlock();
	
	/* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	
	/* Get the 1st page to erase */
  FirstPage = GetPage(APP_START_ADDR);
	
	lastPage =   GetPage(APP_END_ADDR);//BACKUP_START_ADDR);
	
	/* Get the number of pages to erase from 1st page */
  NbOfPages = lastPage - FirstPage + 1;
	
	/* Get the bank */
  BankNumber =GetBank(APP_START_ADDR);
	
	 /* Clear pending flags (if any) */  
      /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = BankNumber;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;
	
	
    /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /*
      Error occurred while page erase.
      User can add here some code to deal with this error.
      PageError will contain the faulty page and then to know the code error on this page,
      user can call function 'HAL_FLASH_GetError()'
    */
    /* Infinite loop */
  }
   
	/* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
}

/**
 *  @brief  :It will erase the Backup memory and set the write pointer to start address of 
 *           back up memory
 *  @return :none
 */
void QbootFlashBackupOpen(void)
{
	  FLASH_EraseInitTypeDef EraseInitStruct = {0};
//	  /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();
	
	/* Clear OPTVERR bit set on virgin samples */
   __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	
	
//	  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
//    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR);
//    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR);
//    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGAERR);
//    //__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR);
//    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGSERR);
	
	 /* Get the 1st page to erase */
  FirstPage = GetPage(BACKUP_START_ADDR);
	
	/* Get the number of pages to erase from 1st page */
  lastPage = GetPage(FLASH_END_ADDR);// - FirstPage + 1;//GetPage(FLASH_END_ADDR) - FirstPage;// + 1;
	
	NbOfPages =lastPage  - FirstPage + 1;
	 /* Get the bank */
  BankNumber = GetBank(BACKUP_START_ADDR);
	
    /* Clear pending flags (if any) */  
      /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = BankNumber;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;
	
   /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
	//if(HAL_FLASHEx_Erase_IT(&EraseInitStruct) != HAL_OK)
  {
    /*
      Error occurred while page erase.
      User can add here some code to deal with this error.
      PageError will contain the faulty page and then to know the code error on this page,
      user can call function 'HAL_FLASH_GetError()'
    */
		LOG_DBG(CH_GSM,"******BACKUP_FLASH_ERASED_ERROR******");
		
		errorCode = HAL_FLASH_GetError();
    /* Infinite loop */
  }
	/* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
	//__enable_irq();
	
	  LOG_DBG(CH_GSM,"******BACKUP_FLASH_ERASED_OK******");
	
    backupWriteAddress = BACKUP_START_ADDR;
}

/**
 *  @brief  :Write the data to flash memory
 *  @param  :[in] addr      start address
 *  @param  :[in] writeData data to be written 
 *  @param  :[in] writeLen  length of the data to be written
 *  @return :none
 */


void QbootFlashWrite(uint32_t addr, uint8_t *writeBuff,uint32_t writeLen)
{
    uint16_t remainderVal;
   uint32_t index = 0;
	uint64_t doubleWord = 0;
    //uint16_t halfWord;
    
    /* Enable the flash control register access */
    HAL_FLASH_Unlock();

    /* Clear pending flags (if any) */  
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
    
    remainderVal = writeLen%8;
    if(remainderVal != 0)
    {
        writeLen -= remainderVal;
    }
				LOG_DBG(CH_GSM,"index = %d,writeLen = %d,remainderVal = %d ",index,writeLen,remainderVal);

		
      
    while(index < writeLen)
    {
			  doubleWord = ((uint64_t)writeBuff[index+7] << 56);
				doubleWord =doubleWord | ((uint64_t)writeBuff[index+6] << 48);
				doubleWord =doubleWord | ((uint64_t)writeBuff[index+5] << 40);
				doubleWord =doubleWord | ((uint64_t)writeBuff[index+4] << 32);
				doubleWord =doubleWord | ((uint64_t)writeBuff[index+3] << 24);
				doubleWord =doubleWord | ((uint64_t)writeBuff[index+2] << 16);
				doubleWord =doubleWord | ((uint64_t)writeBuff[index+1] << 8);
				doubleWord =doubleWord | ((uint64_t)writeBuff[index+0] << 0);

//        halfWord = (uint16_t) writeBuff[index+1] << 8;
//        halfWord = halfWord | writeBuff[index];
			
		     writeStatus = 	  HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr,(uint64_t) doubleWord);
        //HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, halfWord);
			 if(writeStatus == HAL_OK)
			 {
         index = index + 8;
         addr = addr + 8;
			 }
    }
  
    if(remainderVal != 0)
    {
		  LOG_DBG(CH_GSM,"index = %d,writeLen = %d,remainderVal = %d ",index,writeLen,remainderVal);

			 if(remainderVal == 7)
			{
				  doubleWord |= 0xFF00000000000000; 
					doubleWord =doubleWord | ((uint64_t)writeBuff[6] << 48);
					doubleWord =doubleWord | ((uint64_t)writeBuff[5] << 40);
					doubleWord =doubleWord | ((uint64_t)writeBuff[4] << 32);
					doubleWord =doubleWord | ((uint64_t)writeBuff[3] << 24);
					doubleWord =doubleWord | ((uint64_t)writeBuff[2] << 16);
					doubleWord =doubleWord | ((uint64_t)writeBuff[1] << 8);
					doubleWord =doubleWord | ((uint64_t)writeBuff[0] << 0);
					 
			    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, doubleWord);
				}	
			
				else if(remainderVal == 6)
				{
						doubleWord |= 0xFFFF000000000000; 
						doubleWord = doubleWord | ((uint64_t)writeBuff[5] << 40);
						doubleWord = doubleWord | ((uint64_t)writeBuff[4] << 32);
						doubleWord = doubleWord | ((uint64_t)writeBuff[3] << 24);
						doubleWord = doubleWord | ((uint64_t)writeBuff[2] << 16);
						doubleWord = doubleWord | ((uint64_t)writeBuff[1] << 8);
						doubleWord = doubleWord | ((uint64_t)writeBuff[0] << 0);
				 
						HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, doubleWord);
				}
				
				else if(remainderVal == 5){
			      doubleWord |= 0xFFFFFF0000000000; 
						doubleWord = doubleWord | ((uint64_t)writeBuff[4] << 32);
						doubleWord = doubleWord | ((uint64_t)writeBuff[3] << 24);
						doubleWord = doubleWord | ((uint64_t)writeBuff[2] << 16);
						doubleWord = doubleWord | ((uint64_t)writeBuff[1] << 8);
						doubleWord = doubleWord | ((uint64_t)writeBuff[0] << 0);
				 
						HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, doubleWord);
				}
        
				else if(remainderVal == 4){
					  doubleWord |= 0xFFFFFFFF00000000; 
						doubleWord = doubleWord | ((uint64_t)writeBuff[3] << 24);
						doubleWord = doubleWord | ((uint64_t)writeBuff[2] << 16);
						doubleWord = doubleWord | ((uint64_t)writeBuff[1] << 8);
						doubleWord = doubleWord | ((uint64_t)writeBuff[0] << 0);
				 
						HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, doubleWord);
				}
				
				else if(remainderVal == 3){
					  doubleWord |= 0xFFFFFFFFFF000000; 
						doubleWord = doubleWord | ((uint64_t)writeBuff[2] << 16);
						doubleWord = doubleWord | ((uint64_t)writeBuff[1] << 8);
						doubleWord = doubleWord | ((uint64_t)writeBuff[0] << 0);
				 
						HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, doubleWord);
				}
				
				else if(remainderVal == 2){
				    doubleWord |= 0xFFFFFFFFFFFF0000; 
						doubleWord = doubleWord | ((uint64_t)writeBuff[1] << 8);
						doubleWord = doubleWord | ((uint64_t)writeBuff[0] << 0);
				 
			      HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, doubleWord);
				}
				
				else{
				    doubleWord |= 0xFFFFFFFFFFFFFF00; 
						doubleWord = doubleWord | ((uint64_t)writeBuff[0] << 0);
				 
			      HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, doubleWord);
				}
//        halfWord = 0xFF << 8;
//        halfWord = halfWord | writeBuff[index];
//			//HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, halfWord);
//        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, addr, halfWord);
    }
		/* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
		
		LOG_DBG(CH_GSM,"index = %d,writeLen = %d,remainderVal = %d ",index,writeLen,remainderVal);
}

/**
 *  @brief  :It will erase the Backup memory and set the write pointer to start address of 
 *           back up memory
 *  @return :none
 */
void QbootFlashBackupWrite(uint8_t *writeBuf,uint16_t writeLen)
{
    QbootFlashWrite(backupWriteAddress,writeBuf,writeLen);
    backupWriteAddress += writeLen;
		LOG_DBG(CH_GSM,"backupWriteAddress = %X,writeLen = %d",backupWriteAddress,writeLen);

}

/**
 *  @brief  :Will copy the content from the backup flash to application area
 *  @param  :[in] destAddr   address of destination
 *  @param  :[in] sourceAddr address of source
 *  @param  :[in] size       number of bytes to copy
 *  @return :Return_Description
 */
void QbootFlashCopy(uint32_t destAddr,uint32_t sourceAddr,uint32_t size)
{
    QbootFlashWrite(destAddr,(uint8_t *)sourceAddr,size);
}


/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr)
{
	
	return FLASH_BANK_1;
//	if (Addr < ((uint32_t)0x0801FFFF))
//  {
//  return FLASH_BANK_1;
//	}
//	else
//  {
//   return FLASH_BANK_2; /* Bank 2 */
//	}
}
