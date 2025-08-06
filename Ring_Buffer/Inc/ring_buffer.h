#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "stdint.h"

typedef struct
{
    uint8_t *buff;
    uint16_t head;         //pointer to gsmbuf  //uint8_t
    uint16_t tail;
    uint16_t maxLen;
}ringBuffer_st;

void RingBufferInit(ringBuffer_st *buf,uint8_t *buff,uint16_t buffMaxLen);
uint8_t RingBufferFill(ringBuffer_st *buf,uint8_t fill);
uint8_t RingBufferDrain(ringBuffer_st *buf,uint8_t *result);
uint16_t RingBufferGetAvlLen(ringBuffer_st *ring);
//void RingBufferReset(ringBuffer_st *buf);
void RingBufferReset(ringBuffer_st *ring);

#endif