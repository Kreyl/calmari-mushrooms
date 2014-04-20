/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "led.h"
#include "kl_lib_f100.h"
#include "ch.h"
#include "hal.h"
#include "adc_f100.h"
#include "cmd_uart_f10x.h"
#include "ws2812b.h"
#include "application.h"
#include "radio_lvl1.h"

//static void Load(Color_t *PClr);

int main(void) {
    // ==== Init clock system ====
    Clk.SetupBusDividers(ahbDiv2, apbDiv1, apbDiv1);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
    JtagDisable();
    Uart.Init(115200);

    //srand(GetUniqID32());       // Init random generator using MCU uniq ID as seed
    LedWs.Init();
    LedWs.SetCommonColorSmoothly(clBlue, csmSimultaneously);
    Radio.Init();
    App.PThd = chThdSelf();
    App.Init();

    Uart.Printf("\rMushroom  AHB=%u; APB1=%u; APB2=%u\r\n", Clk.AHBFreqHz, Clk.APB1FreqHz, Clk.APB2FreqHz);
    while(true) App.ITask();
}

#if AUTO_OFF
void GoSleep() {
    // Start LSI
    Clk.LsiEnable();
    // Start IWDG
    Iwdg.SetTimeout(4500);
    Iwdg.Enable();
    // Enter standby mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP;
    PWR->CR = PWR_CR_PDDS;
    PWR->CR |= PWR_CR_CWUF;
    __WFI();
}
#endif

// ==== Uart cmd ====
#if UART_RX_ENABLED
void UartCmdCallback(uint8_t R, uint8_t G, uint8_t B) {
    Uart.Printf("R=%u; G=%u; B=%u\r", R, G, B);
    Clr.Red = R;
    Clr.Green = G;
    Clr.Blue = B;
    //Led.SetColorNow(Clr);
    //Save(&Clr);
}
#endif
