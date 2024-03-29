/*
 * ws2812b.h
 *
 *  Created on: 05 ���. 2014 �.
 *      Author: Kreyl
 */

#ifndef WS2812B_H_
#define WS2812B_H_

#include "stm32f10x.h"
#include "ch.h"
#include "hal.h"
#include "kl_lib_f100.h"
#include "color.h"
// Define constants here
#include "application.h"

// Smooth speed
#ifndef LED_SMOOTH_CONST
#define LED_SMOOTH_CONST    810
#endif

#define LED_DMA_MODE    DMA_PRIORITY_VERYHIGH \
                        | STM32_DMA_CR_MSIZE_BYTE \
                        | STM32_DMA_CR_PSIZE_HWORD \
                        | STM32_DMA_CR_MINC     /* Memory pointer increase */ \
                        | STM32_DMA_CR_DIR_M2P  /* Direction is memory to peripheral */ \
                        | STM32_DMA_CR_TCIE     /* Enable Transmission Complete IRQ */

// Bit Buffer
#define RST_BIT_CNT     45 // 45 zero bits to produce reset
#define DATA_BIT_CNT    (LED_CNT * 3 * 8)   // 3 channels 8 bit each
#define TOTAL_BIT_CNT   (DATA_BIT_CNT + RST_BIT_CNT)

enum ClrSetupMode_t {csmOneByOne, csmSimultaneously};

class LedWs_t {
private:
    Timer_t TxTmr;
    VirtualTimer ITmr;
    uint8_t BitBuf[TOTAL_BIT_CNT], *PBit, Indx;
    Color_t IClr[LED_CNT];
    ClrSetupMode_t IMode;
    void AppendBitsMadeOfByte(uint8_t Byte);
    void ISetCurrentColors();
    uint32_t ICalcDelay(uint16_t AValue) { return (uint32_t)((LED_SMOOTH_CONST / (AValue+4)) + 1); }
    uint32_t ICalcDelayClr();
public:
    Color_t DesiredClr[LED_CNT];
    ftVoiVoid OnSmoothStart, OnSmoothEnd;
    void Init();
    void SetCommonColor(Color_t Clr);
    void SetCommonColorSmoothly(Color_t Clr, ClrSetupMode_t AMode = csmSimultaneously);
    // Inner use
    void IStopTx() { TxTmr.SetPwm(0); TxTmr.Disable(); }
    void ITmrHandler();
};

extern LedWs_t LedWs;

#endif /* WS2812B_H_ */
