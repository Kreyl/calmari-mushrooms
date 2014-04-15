/*
 * rlvl1_defins.h
 *
 *  Created on: Nov 21, 2013
 *      Author: kreyl
 */

#ifndef RLVL1_DEFINS_H_
#define RLVL1_DEFINS_H_

#if 1 // ========================== Packet =====================================
struct rPayload_t {
    uint8_t R, G, B;
} __attribute__ ((__packed__));

#define id_t    uint8_t

struct rPkt_t {
    id_t ID;
    uint32_t CycleNTotal;
    id_t TimeOwnerID;
    uint8_t TimeAgeCyc;
    rPayload_t Payload;
} __attribute__ ((__packed__));
#define RPKT_LEN    sizeof(rPkt_t)
#endif

// ============================= MESH settings =================================
/*
 *  |_|_|_|_..._|_|_|_|_|_..._|_|_|_|_|_..._|_|   SLOTS
 *  |_____________|_______...___|_____________|   CYCLES
 *  |_____________________..._________________|   SUPER_CYCLE
 */

//#define SELF_MESH_ID        1
#if 1 // ======================== Address space ================================
#define ID_ABSENT           0   // Id when device does not have self id
#define DEVICE_CNT          100 // Number of abonents
#define DEVICE_ID_MIN       1
#define DEVICE_ID_MAX       DEVICE_CNT
#define MESH_CHANNEL        1   // mesh RF channel
#endif

#if 1 // =========================== Timings ===================================
#define SLOT_DURATION_MS    4   // Single slot duration
#define CYCLE_CNT           5   // Count of cycles in supercycle
#define CYCLE_DURATION_MS   (SLOT_DURATION_MS * DEVICE_CNT)
#define TIME_AGE_MAX_CYC    20  // Cycles. After this, device will consider its time as self one.
// ID aquisition
#define AQ_RX_TIME_CYC      2   // listen N cycles until "noone near" decision
#define AQ_RX_TIME_MS       (CYCLE_DURATION_MS * AQ_RX_TIME_CYC)

//#define GET_RND_VALUE(Top)  ( ( (Random(chTimeNow()) ) % Top ))
//#define GET_RND_VALUE(Top)    ( ((rand() % Top) + 1) )
//#define END_OF_EPOCH        4294967295 // ms = 2^32
//#define END_OF_EPOCH        65536       // max cycle counter

#endif

#endif /* RLVL1_DEFINS_H_ */
