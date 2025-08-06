#ifndef __QBOOT_H
#define __QBOOT_H

#include "qboot_port.h"
#include "debug_log.h"

#define PRODUCT_ID      8



typedef  struct
__attribute__((packed))
{
    uint8_t verNumMajor;            // range from 0 to 255
    uint8_t verNumMinor;            // range from 0 to 255
    uint8_t verNumMicro;            // range from 0 to 255
    uint16_t verNumCrc;             // 16 bit CRC of the three bytes (major, minor and micro)
    uint16_t productId;     
    uint32_t appSize;               // size of the application firmware 
    uint16_t appCrc;           // CRC of the complete application
    uint16_t appHeaderCrc;     // CRC of productId, appSize and appCrc
    uint8_t reserved;
}appHeader_st;


typedef enum
{
  APP_VER_IS_LOWER = 0,
  HEADER_OK,   //1
  INVALID_PRODUCT_ID,
  APP_HEADER_CRC_ERROR,
  VER_NUM_CRC_ERROR,        //verNumCrc not matching
  
}fotaHeaderErr_et;
/**
 *  @brief  : It will read the application header and compare it with downloadHeader
 *            if the application header is less than the download header
 *  
 *  @param  :[in] header - 
 *  @return : 1 - if the download header is valid and greater than app header
 *            0 - if the download header is invalid or it is less than app header
 */
//uint8_t QbootCheckUpdateRequired(appHeader_st *downloadHeader);
fotaHeaderErr_et QbootCheckUpdateRequired(appHeader_st *downloadHeader);

/**
 *  @brief  : Write the back up header to the flash memory the flash memory should have been erased 
 *            before writing
 *  @param  :[in] header 
 *  @return :none
 */
void QbootWriteBackupHeader(appHeader_st *header);

/**
 *  @brief  :It will read the version number of both the application and backup memory
 *  @return :1 - when app version is less than the backup version
 *               when app version number is invalid and back up version number is valid
 *           0 - when app version number is valid and backup version number is invalid
 *               when app version number is equal to back up version number
 */
uint8_t QbootIsUpgradeRequired(void);

/**
 *  @brief  :It will erase the application flash memory
 *           will copy the firmware file from backup to application memory first
 *           will verify the check sum after copying 
 *           will copy the app header from backup memory to app memory
 *  @return :Return_Description
 */
void QbootCopyBackupToFlash();

appHeader_st* QbootGetBackUpAPPVer(void);
//void QbootJumpToApp(void);

uint16_t QbootBackupGetCrc(uint32_t size);

#endif