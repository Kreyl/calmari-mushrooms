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
//        LedWs.SetCommonColorSmoothly(clGreen, csmOneByOne);
//        chThdSleepMilliseconds(18000);
//        LedWs.SetCommonColorSmoothly(clBlue, csmSimultaneously);
//        chThdSleepMilliseconds(7002);
//        LedWs.SetCommonColorSmoothly(clBlack, csmSimultaneously);
//        chThdSleepMilliseconds(7002);

        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
//        if(EvtMsk & EVTMSK_RX) LedWs.SetCommonColorSmoothly(Clr, csmOneByOne);

#if 1 // ==== Keys ====
        // Both Up & Down keys pressed
        if(((EvtMsk & EVTMSK_KEY_UP)   and (Keys.KeyDownIsPressed())) or
           ((EvtMsk & EVTMSK_KEY_DOWN) and (Keys.KeyUpIsPressed())) ) {
            if(Clr != clBlack) {    // Proceed if not black already
                Clr = clBlack;
                SaveColor2Bkp();
                IResetSleepTmr();
                Uart.Printf("Off\r");
                if(Keys.KeyTxIsPressed()) LedWs.SetCommonColorSmoothly(LED_OFF_CLR, csmSimultaneously);
                else {
                    LedWs.SetCommonColorSmoothly(clBlack, csmSimultaneously);
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
                LedWs.SetCommonColorSmoothly(Clr, csmSimultaneously);
            }
            if(EvtMsk & EVTMSK_KEY_DOWN) {
                IResetSleepTmr();
                if(ColorN == 0) ColorN = COLOR_TABLE_SZ-1;
                else ColorN--;
                Clr = ColorTable[ColorN];
                SaveColor2Bkp();
                Uart.Printf("%u; %u %u %u\r", ColorN, Clr.Red, Clr.Green, Clr.Blue);
                LedWs.SetCommonColorSmoothly(Clr, csmSimultaneously);
            }
        }
        // TX button
        if(EvtMsk & EVTMSK_KEY_TX_PRESS) {
            IResetSleepTmr();
            Uart.Printf("TX on\r");
            DoTransmit = true;
            // Show TX-Off mode
            if(Clr == clBlack) LedWs.SetCommonColorSmoothly(LED_OFF_CLR, csmSimultaneously);
        }
        if(EvtMsk & EVTMSK_KEY_TX_RELEASE) {
            Uart.Printf("TX off\r");
            DoTransmit = false;
            // Hide TX-Off mode
            if(Clr == clBlack) {
                LedWs.SetCommonColorSmoothly(clBlack, csmSimultaneously);
                IStartSleepTmr();
            }
        }
#endif
        // Sleep timer
        if(EvtMsk & EVTMSK_SLEEP_CHECK) {
            Uart.Printf("sleep %u\r", BKP->DR1);
        }
    } // while 1
}

void App_t::Init() {
    Bkp_t::EnableWriteAccess();
    LoadColorBkp();
    LedWs.SetCommonColorSmoothly(Clr, csmSimultaneously);
    //Uart.Printf("%u; %u %u %u\r", ColorN, Clr.Red, Clr.Green, Clr.Blue);
}

void App_t::SaveColor2Bkp() { BKP->DR1 = ColorN; }
void App_t::LoadColorBkp() {
    ColorN = BKP->DR1;
    if(ColorN >= COLOR_TABLE_SZ) ColorN = 0;
    Clr = ColorTable[ColorN];
}

#endif

#if 0 // ======================= Command processing ============================
#define UART_RPL_BUF_SZ     36
static uint8_t SBuf[UART_RPL_BUF_SZ];

void Ack(uint8_t Result) { Uart.Cmd(0x90, &Result, 1); }

void UartCmdCallback(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b, b2;
    uint16_t w;
    switch(CmdCode) {
        case CMD_PING: Ack(OK); break;

#if 1 // ==== ID and type ====
        case CMD_SET_ID:
            if(Length == 2) {
                w = (PData[0] << 8) | PData[1];
                App.ID = w;
                b = App.EE.Write32(EE_DEVICE_ID_ADDR, App.ID);
                Ack(b);
            }
            else Ack(CMD_ERROR);
            break;
        case CMD_GET_ID:
            SBuf[0] = (App.ID >> 8) & 0xFF;
            SBuf[1] = App.ID & 0xFF;
            Uart.Cmd(RPL_GET_ID, SBuf, 2);
            break;

        case CMD_SET_TYPE:
            if(Length == 1) Ack(App.SetType(PData[0]));
            else Ack(CMD_ERROR);
            break;
        case CMD_GET_TYPE:
            SBuf[0] = (uint8_t)App.Type;
            Uart.Cmd(RPL_GET_TYPE, SBuf, 1);
            break;
#endif

        // ==== Pills ====
        case CMD_PILL_STATE:
            b = PData[0];   // Pill address
            if(b <= 7) SBuf[1] = PillMgr.CheckIfConnected(PILL_I2C_ADDR);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_STATE, SBuf, 2);
            break;
        case CMD_PILL_WRITE:
            b = PData[0];
            if(b <= 7) SBuf[1] = PillMgr.Write(PILL_I2C_ADDR, &PData[1], Length-1);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_WRITE, SBuf, 2);
            break;
        case CMD_PILL_READ:
            b = PData[0];           // Pill address
            b2 = PData[1];          // Data size to read
            if(b2 > (UART_RPL_BUF_SZ-2)) b2 = (UART_RPL_BUF_SZ-2);  // Check data size
            if(b <= 7) SBuf[1] = PillMgr.Read(PILL_I2C_ADDR, &SBuf[2], b2);
            SBuf[0] = b;
            if(SBuf[1] == OK) Uart.Cmd(RPL_PILL_READ, SBuf, b2+2);
            else Uart.Cmd(RPL_PILL_READ, SBuf, 2);
            break;

        default: break;
    } // switch
}
#endif
