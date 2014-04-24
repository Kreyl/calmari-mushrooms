/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart_f10x.h"
#include "evt_mask.h"
#include "radio_lvl1.h"
#include "ch.h"
#include "ws2812b.h"
#include "keys.h"

App_t App;
IWDG_t Iwdg;

#if 1 // ============================ Timers ===================================
//static VirtualTimer ITmrSleepCheck;
void TmrCheckSleepCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_SLEEP_CHECK);
    chSysUnlockFromIsr();
}
#endif

#if 1 // ========================= Application =================================
__attribute__((noreturn))
void App_t::ITask() {
    chRegSetThreadName("App");
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);

#if 1 // ==== Keys ====
        if(!IsCharging) {   // Do not react on keys while charging
            // Both Up & Down keys pressed
            if(((EvtMsk & EVTMSK_KEY_UP)   and (Keys.KeyDownIsPressed())) or
               ((EvtMsk & EVTMSK_KEY_DOWN) and (Keys.KeyUpIsPressed())) ) {
                if(Clr != clBlack) {    // Proceed if not black already
                    Clr = clBlack;
                    SaveColor2Bkp();
                    IResetSleepTmr();
                    Uart.Printf("Off\r");
                    if(Keys.KeyTxIsPressed()) LedWs.SetCommonColorSmoothly(LED_OFF_CLR);
                    else {
                        LedWs.SetCommonColorSmoothly(clBlack);
                        IStartSleepTmr();
                    }
                } // if not black
            }
            else {
                if(EvtMsk & EVTMSK_KEY_UP) {
                    IResetSleepTmr();
                    if(++ColorN >= COLOR_TABLE_SZ) ColorN = 0;
                    Clr = ColorTable[ColorN];
                    SaveColor2Bkp();
                    Uart.Printf("%u; %u %u %u\r", ColorN, Clr.Red, Clr.Green, Clr.Blue);
                    LedWs.SetCommonColorSmoothly(Clr);
                }
                if(EvtMsk & EVTMSK_KEY_DOWN) {
                    IResetSleepTmr();
                    if(ColorN == 0) ColorN = COLOR_TABLE_SZ-1;
                    else ColorN--;
                    Clr = ColorTable[ColorN];
                    SaveColor2Bkp();
                    Uart.Printf("%u; %u %u %u\r", ColorN, Clr.Red, Clr.Green, Clr.Blue);
                    LedWs.SetCommonColorSmoothly(Clr);
                }
            }
            // TX button
            if(EvtMsk & EVTMSK_KEY_TX_PRESS) {
                IResetSleepTmr();
                Uart.Printf("TX on\r");
                DoTransmit = true;
                // Show TX-Off mode
                if(Clr == clBlack) LedWs.SetCommonColorSmoothly(LED_OFF_CLR);
            }
            if(EvtMsk & EVTMSK_KEY_TX_RELEASE) {
                Uart.Printf("TX off\r");
                DoTransmit = false;
                // Hide TX-Off mode
                if(Clr == clBlack) {
                    LedWs.SetCommonColorSmoothly(clBlack);
                    IStartSleepTmr();
                }
            }
        } // if !charging
#endif

#if 1 // ==== Charge start-end ====
        if(EvtMsk & EVTMSK_CHARGE_START) {
            IsCharging = true;
            DoTransmit = false;
            IResetSleepTmr();
            EnterNormalPower();
            LedWs.SetCommonColorSmoothly(LED_CHARGING_CLR);
            Uart.Printf("Charge start\r");
        }
        if(EvtMsk & EVTMSK_CHARGE_END) {
            IsCharging = false;
            EnterNormalPower();
            LedWs.SetCommonColorSmoothly(LED_CHARGING_DONE);
            Uart.Printf("Charge end\r");
        }
#endif
        // Sleep timer
        if(EvtMsk & EVTMSK_SLEEP_CHECK) {
            Uart.Printf("Sleep %u\r", BKP->DR1);
            chThdSleepMilliseconds(99);
            Iwdg.GoSleepFor(SLEEP_TIME_MS);
        }
    } // while 1
}

void App_t::Init() {
    Bkp_t::Enable();
    Bkp_t::EnableWriteAccess();
    LoadColorBkp();
    LedWs.SetCommonColorSmoothly(Clr);
    //Uart.Printf("%u; %u %u %u\r", ColorN, Clr.Red, Clr.Green, Clr.Blue);
}

void App_t::SaveColor2Bkp() {
    BKP->DR1 = ColorN;
    BKP->DR2 = (Clr == clBlack)? 1 : 0;
}
void App_t::LoadColorBkp() {
    ColorN = BKP->DR1;
    if(ColorN >= COLOR_TABLE_SZ) ColorN = 0;
    Clr = (BKP->DR2 == 1)? clBlack : ColorTable[ColorN];
}

#endif
