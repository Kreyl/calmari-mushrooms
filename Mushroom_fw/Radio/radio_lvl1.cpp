/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include "radio_lvl1.h"
#include "evt_mask.h"
#include "application.h"
#include "cc2500.h"
#include "cmd_uart_f10x.h"
#include <cstdlib>

//#define TX
//#define RX
#define MESH

#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOB
#define DBG_PIN1    10
#define DBG1_SET()  PinSet(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinClear(DBG_GPIO1, DBG_PIN1)
#endif

rLevel1_t Radio;

#if 1 // ================================ Task =================================
static WORKING_AREA(warLvl1Thread, 256);
__attribute__((noreturn))
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    while(true) Radio.ITask();
}

__attribute__((noreturn))
void rLevel1_t::ITask() {
    while(true) {
#ifdef TX
        PktTx.Clr.Red = 0;
        PktTx.Clr.Green = 255;
        PktTx.Clr.Blue = 255;
        PktTx.Brightness = 255;
        DBG1_SET();
        CC.TransmitSync(&PktTx);
        DBG1_CLR();

#elif defined RX
        // ======== RX cycle ========
        int8_t Rssi;
        uint8_t RxRslt;
        RxRslt = CC.ReceiveSync(RX_T_MS, &PktRx, &Rssi);
        if(RxRslt == OK) {
            Uart.Printf("Rx: %u %u %u, %u; Rssi=%d\r", PktRx.Clr.Red, PktRx.Clr.Green, PktRx.Clr.Blue, PktRx.Brightness, Rssi);
            App.SendEvtRx();
        }
        CC.Sleep();
        chThdSleepMilliseconds(SLEEP_T_MS);
#elif defined MESH
        if(SelfID == ID_ABSENT) IIDAquisition();
        else ISuperCycle();
#endif
//        chThdSleepMilliseconds(999);    // DEBUG
    } // while true
}
#endif // task

// Supercycle handler
void rLevel1_t::ISuperCycle() {
    // Determine number of RX cycle
    uint8_t RxCycleNum = rand() % CYCLE_CNT;
    for(uint8_t i=0; i<CYCLE_CNT; i++) {
        CycleNTotal++;  // Increase inner cycle counter
        if(i == RxCycleNum) IRxCycle();    // RX cycle: listen only
        else {  // Ordinal cycle
            uint32_t CycleStartTime = chTimeNow();
            uint32_t CycleEndTime = CycleStartTime + MS2ST(CYCLE_DURATION_MS);
            // Sleep until self TX slot
            uint32_t TimeToSleep = (SelfID - 1) * SLOT_DURATION_MS;
            if(TimeToSleep != 0) chThdSleepMilliseconds(MS2ST(TimeToSleep));
            // Prepare and Transmit
            PktTx.CycleNTotal = CycleNTotal;
            if(PktTx.TimeOwnerID != SelfID) {
                // When TimeOwnerID == SelfID, TimeAge does not make sense
                if(++PktTx.TimeAgeCyc > TIME_AGE_MAX_CYC) PktTx.TimeOwnerID = SelfID;
            }
            DBG1_SET();
            CC.TransmitSync(&PktTx);
            DBG1_CLR();
            // Sleep until end of cycle
            if(chTimeNow() < CycleEndTime) chThdSleepUntil(CycleEndTime);
        } // if ordinal
    } // for i
}

// Beware: here assumed that systick is 1 ms.
void rLevel1_t::IRxCycle() {
//    uint32_t SlotN = 0;
    uint32_t ListenTime = CYCLE_DURATION_MS;
    while(true) {
        uint32_t RxStartTime = chTimeNow();
        if(CC.ReceiveSync(ListenTime, &PktRx, nullptr) == OK) {

        }
        // Recalculate listen time
        uint32_t ElapsedTime = chTimeNow() - RxStartTime;
        if(ListenTime > ElapsedTime) ListenTime -= ElapsedTime;
        else break;
    }
}

void rLevel1_t::IIDAquisition() {
    // Clear table
    for(uint32_t i=0; i<DEVICE_CNT; i++) rxIdTable[i] = false;
    // Listen for several cycles to determine unoccupied ID
    uint32_t ListenEndTime = chTimeNow() + AQ_RX_TIME_MS;
    do {
        if(CC.ReceiveSync(AQ_RX_TIME_MS, &PktRx, nullptr) == OK) {
            if((PktRx.ID >= DEVICE_ID_MIN) and (PktRx.ID <= DEVICE_ID_MAX))
                rxIdTable[PktRx.ID] = true;
        }
    } while(chTimeNow() < ListenEndTime);
    // Table filled, extract ID
    id_t tmpID = ID_ABSENT;
    for(uint32_t i=DEVICE_ID_MIN; i<DEVICE_ID_MAX; i++) {   // No need to check ID_MAX
        // If last ID is occupied, take first empty one
        if(rxIdTable[DEVICE_ID_MAX]) {
            if(rxIdTable[i] == false) {
                SelfID = i;
                break;
            }
        }
        // Otherwise, take next after top ID
        else if(rxIdTable[i]) tmpID = i+1; // i'th ID occupied => take next
    }
    SelfID = tmpID;
    PktTx.ID = SelfID;
    PktTx.TimeOwnerID = SelfID;
    CycleNTotal = 0;
}

#if 1 // ============================
void rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init random generator using MCU uniq ID as seed
//    uint32_t s = GetUniqID32();
//    Uart.Printf("%X\r", s);
    srand(GetUniqID32());

    // Variables
    SelfID = ID_ABSENT;     // Initially, ID is absent
    CycleNTotal = 0;

    // DEBUG
    PktTx.Payload.R = 0;
    PktTx.Payload.G = 255;
    PktTx.Payload.B = 255;

    // Init radioIC
    CC.Init();
    CC.SetTxPower(ccPwr0dBm);
    CC.SetChannel(MESH_CHANNEL);
    CC.SetPktSize(RPKT_LEN);
    // Variables
    // Thread
    PThread = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
