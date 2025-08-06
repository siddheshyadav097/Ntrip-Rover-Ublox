#include "qboot.h"
#include "crc16.h"

#define QBOOT_UPGRADE_RETRY         3

appHeader_st appHeader;
appHeader_st backupHeader;
fotaHeaderErr_et headerErr = HEADER_OK;

void QbootReadAppHeader(void)
{
    uint8_t i;
    uint8_t *ptr;
    ptr = (uint8_t *)&appHeader;
    // read the application header
    for(i = 0; i < sizeof(appHeader_st); i++)
    {
        ptr[i] = *(uint8_t *)(APP_VER_START_ADDR + i);
    }
}

void QbootReadBackupHeader(void)
{
    uint8_t i;
    uint8_t *ptr;
    ptr = (uint8_t *)&backupHeader;
    // read the application header
    for(i = 0; i < sizeof(appHeader_st); i++)
    {
        ptr[i] = *(uint8_t *)(BACKUP_VER_START_ADDR + i);
    }
}



/**
 *  @brief  : Write the  header to the flash memory the flash memory should have been erased 
 *            before writing
 *  @param  :[in] header 
 *  @return :none
 */
void QbootWriteAppHeader(appHeader_st *header)
{
    QbootFlashWrite(APP_VER_START_ADDR,(uint8_t *)header,sizeof(appHeader_st));
}

/**
 *  @brief  : Write the back up header to the flash memory the flash memory should have been erased 
 *            before writing
 *  @param  :[in] header 
 *  @return :none
 */
void QbootWriteBackupHeader(appHeader_st *header)
{
    QbootFlashWrite(BACKUP_VER_START_ADDR,(uint8_t *)header,sizeof(appHeader_st));
}

/**
 *  @brief  :It will verify the checksum of the app version no.
 *           before calling this function QbootReadAppHeader should be called
 *  @return : 1 - app version checksum matching 
 *            0 - app version checksum not matching
 */
uint8_t QbootIsAppVerNumValid(void)
{
    if(Crc16Get((uint8_t *)&appHeader,5) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 *  @brief  :It will verify the checksum of the backup version no.
 *           before calling this function QbootReadBackupHeader should be called
 *  @return : 1 - backup version checksum matching 
 *            0 - backup version checksum not matching
 */
uint8_t QbootIsBackupVerNumValid(void)
{
    if(Crc16Get((uint8_t *)&backupHeader,5) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 *  @brief  :It will check whether app version num and backup version num are same
 *  @return : 1 - backup version checksum matching 
 *            0 - backup version checksum not matching
 */
uint8_t QbootIsAppVerEqualToBackupVer(void)
{
    if(appHeader.verNumMajor == backupHeader.verNumMajor && \
       appHeader.verNumMinor == backupHeader.verNumMinor && \
       appHeader.verNumMicro == backupHeader.verNumMicro)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/**
 *  @brief  :It will read the version number of both the application and backup memory
 *  @return :1 - when app version is less than the backup version
 *               when app version number is invalid and back up version number is valid
 *           0 - when app version number is valid and backup version number is invalid
 *               when app version number is equal to back up version number
 */
uint8_t QbootIsUpgradeRequired(void)
{
    QbootReadAppHeader();
    QbootReadBackupHeader();
    
    if(QbootIsAppVerNumValid() == 1)
    {
        // this is the case when after manufacturing only the app and bootloader is present
        // and the backup memory is empty
        if(QbootIsBackupVerNumValid() == 0)
        {
            // no need for upgrade
            return 0;
        }
        else
        {
            // if both the version number are valid then check whether both are equal
            if(QbootIsAppVerEqualToBackupVer() == 1)
            {
                // no need for upgrade
                return 0;
            }
            else
            {
                // update as the backup version will always be greater than the current application
                return 1;
            }
        }
    }
    else
    {
        // app version number should always be valid
        // app version number can become invalid only when the system was switched OFF while 
        // writing from backup to flash memory
        if(QbootIsBackupVerNumValid() == 1)
        {
            // upgrade required
            return 1;
        }
        else
        {
            /* critical this should never happen as there is no application as well as
             no backup application available */
            LOG_CRITS(CH_BOOT,"App header and back up header not available");
            return 0;
        }
    }
}


/**
 *  @brief  : It will check the application
 *  @return : none
 */
uint8_t QbootAppFlashMemValidate(void)
{
    // read the CRC
    uint16_t crc;
    QbootReadBackupHeader();
    crc = Crc16Get((uint8_t *)APP_START_ADDR,backupHeader.appSize);
    if(crc == backupHeader.appSize)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint16_t QbootBackupGetCrc(uint32_t size)
{
    return Crc16Get((uint8_t *)BACKUP_START_ADDR,size);
}


/**
 *  @brief  :It will erase the application flash memory
 *           will copy the firmware file from backup to application memory first
 *           will verify the check sum after copying 
 *           will copy the app header from backup memory to app memory
 *  @return :Return_Description
 */
void QbootCopyBackupToFlash()
{
    uint8_t retryCount = 0;
    // erase the app memory
    while(retryCount < QBOOT_UPGRADE_RETRY)
    {
        QbootFlashAppErase();
        QbootFlashCopy(APP_START_ADDR,BACKUP_START_ADDR,backupHeader.appSize);
        if(QbootAppFlashMemValidate() == 1)
        {
            QbootFlashCopy(APP_VER_START_ADDR,BACKUP_VER_START_ADDR,sizeof(appHeader_st));
            //TODO 
            LOG_ERRS(CH_BOOT,"Upgrade success");
            break;
        }
        else
        {
            LOG_ERRS(CH_BOOT,"Upgrade crc failed");
            retryCount++;
        }
    }
}

//void QbootJumpToApp(void)
//{
//    Jump(APP_START_ADDR);
//}

appHeader_st* QbootGetBackUpAPPVer(void)
{
   return (&appHeader);
} 

/**
 *  @brief  : It will read the application header and compare it with downloadHeader
 *            if the application header is less than the download header
 *  
 *  @param  :[in] header - 
 *  @return : 1 - if the download header is valid and greater than app header
 *            0 - if the download header is invalid or it is less than app header
 */
fotaHeaderErr_et QbootCheckUpdateRequired(appHeader_st *downloadHeader)
{
    uint16_t crc;
    QbootReadAppHeader();
    //validate the version number
    crc = Crc16Get((uint8_t *)downloadHeader,3);
    if(crc == downloadHeader->verNumCrc)
    {
        //validate the app header
        crc = Crc16Get((uint8_t *)&downloadHeader->productId,8);
        if(crc == downloadHeader->appHeaderCrc)
        {
            if(downloadHeader->productId == PRODUCT_ID)
            {
                // now check whether the version is greater than app version num
                if(QbootIsAppVerNumValid() == 1)
                {
                    if(downloadHeader->verNumMajor > appHeader.verNumMajor)
                    {
                          headerErr =  HEADER_OK;
//                        return 1;
                    }
                    else if(downloadHeader->verNumMinor > appHeader.verNumMinor)
                    {
//                        return 1;
                          headerErr =  HEADER_OK;
                    }
                    else if(downloadHeader->verNumMicro > appHeader.verNumMicro)
                    {
//                        return 1;
                          headerErr =  HEADER_OK;
                    }
                    else
                    {
                        LOG_ERR(CH_BOOT,"Check update version number not greater");
                        headerErr =  APP_VER_IS_LOWER;
                    }
                }
                else
                {
//                    return 1;
                      headerErr =  HEADER_OK;
                }
                
            }
            else
            {
                LOG_ERR(CH_BOOT,"Check update Invalid product ID");
                headerErr =  INVALID_PRODUCT_ID;
            }
        }
        else
        {
            LOG_ERR(CH_BOOT,"Check update error app header CRC");
            headerErr = APP_HEADER_CRC_ERROR;
        }
    }
    else
    {
        LOG_ERR(CH_BOOT,"Check update error VER no. CRC");
        headerErr = VER_NUM_CRC_ERROR;
    }
     return headerErr;
//    return 0;
}


