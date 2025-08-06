#ifndef __QBOOT_PORT_H
#define __QBOOT_PORT_H

#include "stm32g0xx_hal.h"

// address used by qbootloader
// application version number start address
#define APP_VER_START_ADDR      ((uint32_t)0x0801FFF0)  //((uint32_t)0x08021FF0)    
// application start address
#define APP_START_ADDR          ((uint32_t)0x08004000)

#define APP_END_ADDR            ((uint32_t)0x0801F800) /* Base @ of Page 63, 2 Kbytes */


// Backup application version number start address
#define BACKUP_VER_START_ADDR   ((uint32_t)0x0803FFF0)
// Backup application start address
#define BACKUP_START_ADDR       ((uint32_t)0x08020000)//((uint32_t)0x08022000)                          /* Base @ of Page 68, 2 Kbytes */

#define FLASH_END_ADDR          ((uint32_t)0x0803F800) /* Base @ of Page 127, 2 Kbytes */ //((uint32_t)0x08040000)//(((uint32_t)0x0803F800) + FLASH_PAGE_SIZE - 1)  /* Base @ of Page 127, 2 Kbytes */

/**
 *  @brief  :Will erase the app memory
 *  @return :none
 */
void QbootFlashAppErase(void);

/**
 *  @brief  :It will erase the Backup memory and set the write pointer to start address of 
 *           back up memory
 *  @return :none
 */
void QbootFlashBackupOpen(void);

/**
 *  @brief  :Write the data to flash memory
 *  @param  :[in] addr      start address
 *  @param  :[in] writeData data to be written 
 *  @param  :[in] writeLen  length of the data to be written
 *  @return :none
 */
void QbootFlashWrite(uint32_t addr, uint8_t *writeBuff,uint32_t writeLen);

void QbootFlashBackupWrite(uint8_t *writeBuf,uint16_t writeLen);

/**
 *  @brief  :Will copy the content from the backup flash to application area
 *  @param  :[in] destAddr   address of destination
 *  @param  :[in] sourceAddr address of source
 *  @param  :[in] size       number of bytes to copy
 *  @return :Return_Description
 */
void QbootFlashCopy(uint32_t destAddr,uint32_t sourceAddr,uint32_t size);

void Jump(uint32_t vtor);

#endif