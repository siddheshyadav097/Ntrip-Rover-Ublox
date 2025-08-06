/**
 *  \file lib_api.h
 *  \brief Includes public function declarations for data conversion library APIs.
 */

#ifndef __LIB_API_H__
#define __LIB_API_H__


 /*----includes-----*/
#include "lib_port.h"


//uint8_t CalculateLRC(uint8_t *WtBuffPtr, uint16_t WtBuffLen);
//uint16_t GetSum(uint8_t *data,uint16_t count);


 /*----public function declarations-----*/

//uint8_t m_ctoh(uint8_t ch);
uint32_t m_atoh(uint8_t *ptr);
//void m_htoa(uint8_t *ptr, uint8_t mch);
//uint16_t getInteger(uint8_t *ptr);
//void m_itoa (uint16_t i,char *ptr);
//void m_i2a(uint16_t val,uint8_t *ptr);
//unsigned long int m_a2l(uint8_t *ptr);
//uint8_t m_a2b(uint8_t *ptr);

#endif