/*
 * rlvl1_defins.h
 *
 *  Created on: Nov 21, 2013
 *      Author: kreyl
 */

#ifndef RLVL1_DEFINS_H_
#define RLVL1_DEFINS_H_

#include "color.h"

#if 1 // ========================== Packet =====================================
struct rPayload_t {
    uint8_t R, G, B, Brightness;
} __attribute__ ((__packed__));

struct rPkt_t {
    uint8_t ID;
    uint32_t CycleN;
    uint8_t TimeOwnerID;
    uint8_t TimeAge;
    rPayload_t Payload;
} __attribute__ ((__packed__));
#define RPKT_LEN    sizeof(rPkt_t)
#endif

#if 1 // =========================== Timings ===================================
#define RX_T_MS     45
#define SLEEP_T_MS  504

#endif



#endif /* RLVL1_DEFINS_H_ */
