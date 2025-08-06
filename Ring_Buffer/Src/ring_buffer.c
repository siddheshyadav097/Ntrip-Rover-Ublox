#include "ring_buffer.h"

void RingBufferInit(ringBuffer_st *ring,uint8_t *buff,uint16_t buffMaxLen)
{
    ring->buff = buff;
    ring->maxLen = buffMaxLen;
    ring->head  = 0;
    ring->tail  = 0;
}
/**
 *  @brief  : It fills the circular buffer the maximum data to be stored should always
 *              be one less than the buffer size as it is required to find whether the buffer is full 
 *              or empty
 *           It will not fill if the buffer is full 
 *           The buffer full condition is checked by checking the next fill should not be same as tail index
 *              
 *  @param  :[in] ring - circular buffer handle
 *  @param  :[in] fill - fill data 
 *  @return : 1 - successful fill
 *            0 - buffer full first it should be drained before filling
 */
uint8_t RingBufferFill(ringBuffer_st *ring,uint8_t fill)
{
    // next is where head will point to after this write.
    uint16_t next = ring->head + 1;
    if (next >= ring->maxLen)
        next = 0;

    if (next == ring->tail) // check if circular buffer is full
        return 0;       // and return with an error.

    ring->buff[ring->head] = fill; // Load data and then move
    ring->head = next;            // head to next data offset.
    return 1;  // return success to indicate successful push.
}

/**
 *  @brief  :Brief
 *  @param  :[in] received data is obtained here
 *  @return :   0 - no data in circular buffer
 *              1 - data availabe in circular buffer;
 */
uint8_t RingBufferDrain(ringBuffer_st *ring,uint8_t *result)
{
     if (ring->head == ring->tail) // check if circular buffer is empty
        return 0;          // and return with an error

    // next is where tail will point to after this read.
    uint16_t next = ring->tail + 1;
    if(next >= ring->maxLen)
        next = 0;

    *result = ring->buff[ring->tail]; // Read data and then move
    ring->tail = next;             // tail to next data offset.
    return 1;  // return success to indicate successful push.
}

uint16_t RingBufferGetAvlLen(ringBuffer_st *ring)
{
    // check the length of the remaining available data
    if (ring->head == ring->tail) // check if circular buffer is empty
    {
        return 0;
    }
        
    if(ring->head > ring->tail)
    {
        return(ring->head - ring->tail);
    }
    
    if(ring->head < ring->tail)
    {
        return ((ring->maxLen - ring->tail) + ring->head);
    }
    
    return 0;
}

void RingBufferReset(ringBuffer_st *ring)
{
    ring->head  = 0;
    ring->tail  = 0;
}