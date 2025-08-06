#include "qmath.h"
///**
// *  @brief  : get LRC
// *  @param  :[in] pucFrame pointer to array of bytes on which LRC is to be calculated
// *  @param  :[in] usLen    length of array
// *  @return :Return_Description
// */
//uint8_t GetLRC( uint8_t *pucFrame,uint16_t usLen)
//{
//    uint8_t ucLRC = 0;  /* LRC char initialized */
//
//    while(usLen--)
//    {
//        ucLRC += *pucFrame++;   /* Add buffer byte without carry */
//    }
//
//    /* Return twos complement */
//    ucLRC = (uint8_t)(-((int8_t)ucLRC));
//    return ucLRC;
//}

uint16_t GetCrc16(uint8_t* data,uint16_t len)
{
    uint16_t crc;
    uint8_t i;
    crc = 0xFFFF;
    while(len != 0)
    {
        crc ^= *data++;
        for (i = 0; i < 8; ++i)
        {
        if (crc & 1)
          crc = (crc >> 1) ^ 0xA001;
        else
          crc = (crc >> 1);
        }
    i=0;
    len--;
    }
    return crc;
}

///**
// *  @brief  : 
// *  @param  :[in] ucCharacter Parameter_Description
// *  @return :Return_Description
// */
//uint8_t ConvertHexAsciiCharToBin(uint8_t ucCharacter)
//{
//    if( ( ucCharacter >= '0' ) && ( ucCharacter <= '9' ) )
//    {
//        return (uint8_t)( ucCharacter - '0' );
//    }
//    else if( ( ucCharacter >= 'A' ) && ( ucCharacter <= 'F' ) )
//    {
//        return (uint8_t)( ucCharacter - 'A' + 0x0A );
//    }
//    else
//    {
//        return 0xFF;
//    }
//}

//uint8_t ConvertBinToHexAsciiChar(uint8_t ucByte)
//{
//    if( ucByte <= 0x09 )
//    {
//        return (uint8_t)( '0' + ucByte );
//    }
//    else if( ( ucByte >= 0x0A ) && ( ucByte <= 0x0F ) )
//    {
//        return (uint8_t)( ucByte - 0x0A + 'A' );
//    }
//    else
//    {
//        /* Programming error. */
//       
//    }
//    return '0';
//}


///**
// *  @brief  : converts the hex bytes into two byte hex ascii
// *  @param  :[in] hex   binary byte
// *  @param  :[in] ascii result stored in this array
// *                      ascii[0] = higher byte
// *                      ascii[1] = lower byte
// *  @return : none
// */
//void ConvertHexToAscii(uint8_t hex,uint8_t *ascii)
//{
//    ascii[0] = ConvertBinToHexAsciiChar((hex&0xF0) >> 4);
//    ascii[1] = ConvertBinToHexAsciiChar(hex&0x0F);
//}

///**
// *  @brief  : converts two digit decimal ascii to one byte binary
// *  @param  :[in] tensPlace 
// *  @param  :[in] unitPlace 
// *  @return : 8bit binary number
// */
//uint8_t ConvertTwoDigitDecAsciiToBin(uint8_t tensPlace,uint8_t unitPlace)
//{
//    uint8_t result;
//    result = tensPlace - '0';
//    if(result < 9)
//    {
//        result = (result*10) + (unitPlace - '0');
//    }
//    
//    return result;
//}