#ifndef __QMATH_H
#define __QMATH_H

#include "stm32g0xx_hal.h"

uint8_t GetLRC(uint8_t *pucFrame,uint16_t usLen);
uint16_t GetCrc16(uint8_t* data,uint16_t len);
uint8_t ConvertHexAsciiCharToBin(uint8_t ucCharacter);
uint8_t ConvertBinToHexAsciiChar(uint8_t ucByte);
void ConvertHexToAscii(uint8_t hex,uint8_t *ascii);
uint8_t ConvertTwoDigitDecAsciiToBin(uint8_t tensPlace,uint8_t unitPlace);

#endif