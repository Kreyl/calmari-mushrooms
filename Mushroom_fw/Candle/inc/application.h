/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_f100.h"
#include "ch.h"
#include "evt_mask.h"
#include "color.h"

#define APP_NAME_CANDLE

// ==== LED ====
#define LED_CNT             1
#define LED_TMR             TIM1
#define LED_TMR_CHNL        1
#define LED_GPIO            GPIOA
#define LED_PIN             8
#define LED_DMA_STREAM      STM32_DMA1_STREAM5  // TIM1_UPD

#define LED_SMOOTH_CONST    360 // Lower = faster
#define LED_OFF_CLR         (Color_t){0, 0, 7}

// ==== Timings ====
#define CHECK_SLEEP_PERIOD_MS   4500

// ==== Application class ====
void TmrCheckSleepCallback(void *p);
class App_t {
private:
    uint32_t ColorN = 32;   // Green
    VirtualTimer ITmrSleepCheck;
    void IStartSleepTmr() {
        chSysLock();
        chVTSetI(&ITmrSleepCheck, MS2ST(CHECK_SLEEP_PERIOD_MS), TmrCheckSleepCallback, nullptr);
        chSysUnlock();
    }
    void IResetSleepTmr() { chVTReset(&ITmrSleepCheck); }
    void SaveColor2Bkp();
    void LoadColorBkp();

    void EnterLowPower() {
        Clk.SetupBusDividers(ahbDiv8, apbDiv1, apbDiv1);
    }
    void EnterNormalPower() {
        Clk.SetupBusDividers(ahbDiv2, apbDiv1, apbDiv1);
    }
public:
    Color_t Clr;
    Thread *PThd;
    void Init();
    void SendEvtRx(uint8_t R, uint8_t G, uint8_t B) {
        Clr.Set(R, G, B);
        chEvtSignal(PThd, EVTMSK_RX);
    }
    bool DoTransmit;
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
