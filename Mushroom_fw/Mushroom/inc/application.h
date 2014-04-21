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

#define APP_NAME_CRYSTAL

// ==== LED ====
#define LED_CNT         5
#define LED_TMR         TIM15
#define LED_TMR_CHNL    2
#define LED_GPIO        GPIOB
#define LED_PIN         15
#define LED_DMA_STREAM  STM32_DMA1_STREAM5  // TIM15_UPD
#define LED_REMAP_TIM15 TRUE

#if 1 // ==== Timings ====

#endif

// ==== Application class ====
class App_t {
private:
    Color_t IClr;
public:
    Thread *PThd;
    void Init();
    void SendEvtRx(uint8_t R, uint8_t G, uint8_t B) {
        IClr.Red = R;
        IClr.Green = G;
        IClr.Blue = B;
        chEvtSignal(PThd, EVTMSK_RX);
    }
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
