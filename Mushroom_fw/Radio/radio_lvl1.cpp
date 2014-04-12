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

#define TX

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

#else
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
#endif
    } // while true
}
#endif // task

#if 1 // ============================
void rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init radioIC
    CC.Init();
    CC.SetTxPower(ccPwr0dBm);
    CC.SetChannel(0);
    CC.SetPktSize(RPKT_LEN);
    // Variables
    // Thread
    PThread = chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), NORMALPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
