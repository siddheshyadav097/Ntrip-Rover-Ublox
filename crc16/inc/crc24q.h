/* Interface for CRC-24Q cyclic redundancy chercksum code
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * SPDX-License-Identifier: BSD-2-clause
 */
#ifndef _CRC24Q_H_
#define _CRC24Q_H_

#include <stdint.h>

extern void crc24q_sign(unsigned char *data, int len);

extern uint8_t crc24q_check(unsigned char *data, int len);

extern unsigned crc24q_hash(unsigned char *data, int len);
#endif /* _CRC24Q_H_ */
