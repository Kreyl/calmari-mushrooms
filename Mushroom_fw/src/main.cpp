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
#if LED_RGB_ENABLE
    Led.Init();
#endif
#if LED_WS_ENABLE
    LedWs.Init();
#endif
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

// ==== Flash load/save ====
#define FLASH_PAGE_SIZE     1024
#define FLASH_KEY1          ((uint32_t)0x45670123)
#define FLASH_KEY2          ((uint32_t)0xCDEF89AB)
#define CR_LOCK_Set         ((uint32_t)0x00000080)
#define CR_PER_Set          ((uint32_t)0x00000002)
#define CR_PER_Reset        ((uint32_t)0x00001FFD)
#define CR_STRT_Set         ((uint32_t)0x00000040)
#define CR_PG_Set           ((uint32_t)0x00000001)
#define CR_PG_Reset         ((uint32_t)0x00001FFE)
#define EraseTimeout        ((uint32_t)0x000B0000)
#define ProgramTimeout      ((uint32_t)0x00002000)

const uint32_t MyBigUint __attribute__ ((section("MyFlash"), aligned(FLASH_PAGE_SIZE))) = 0x00040B04; // 00 BB GG RR
Color_t *PSavedClr = (Color_t*)&MyBigUint;

void FLASH_Unlock() {
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
}
void FLASH_Lock(void) { FLASH->CR |= CR_LOCK_Set; }
void FLASH_ClearFlag(uint32_t FLASH_FLAG) { FLASH->SR = FLASH_FLAG; }

uint8_t FLASH_GetBank1Status(void) {
    if(FLASH->SR & FLASH_SR_BSY) return BUSY;
    else if(FLASH->SR & FLASH_SR_PGERR) return FAILURE;
    else if(FLASH->SR & FLASH_SR_WRPRTERR) return FAILURE;
    else return OK;
}
uint8_t FLASH_WaitForLastOperation(uint32_t Timeout) {
    uint8_t status = OK;
    // Wait for a Flash operation to complete or a TIMEOUT to occur
    do {
        status = FLASH_GetBank1Status();
        Timeout--;
    } while((status == BUSY) and (Timeout != 0x00));
    if(Timeout == 0x00) status = TIMEOUT;
    return status;
}

uint8_t FLASH_ErasePage(uint32_t PageAddress) {
    uint8_t status = FLASH_WaitForLastOperation(EraseTimeout);
    if(status == OK) {
        FLASH->CR |= CR_PER_Set;
        FLASH->AR = PageAddress;
        FLASH->CR |= CR_STRT_Set;
        // Wait for last operation to be completed
        status = FLASH_WaitForLastOperation(EraseTimeout);
        // Disable the PER Bit
        FLASH->CR &= CR_PER_Reset;
    }
    return status;
}

uint8_t FLASH_ProgramWord(uint32_t Address, uint32_t Data) {
    uint8_t status = FLASH_WaitForLastOperation(ProgramTimeout);
    if(status == OK) {
        FLASH->CR |= CR_PG_Set; // program the new first half word
        *(__IO uint16_t*)Address = (uint16_t)Data;
        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation(ProgramTimeout);
        if(status == OK) {
            // program the new second half word
            uint32_t tmp = Address + 2;
            *(__IO uint16_t*)tmp = Data >> 16;
            /* Wait for last operation to be completed */
            status = FLASH_WaitForLastOperation(ProgramTimeout);
            /* Disable the PG Bit */
            FLASH->CR &= CR_PG_Reset;
        }
        else FLASH->CR &= CR_PG_Reset;  // Disable the PG Bit
    }
    return status;
}


void Load(Color_t *PClr) {
    PClr->Red = PSavedClr->Red;
    PClr->Green = PSavedClr->Green;
    PClr->Blue = PSavedClr->Blue;
}

void Save(Color_t *PClr) {
    uint8_t status = OK;
    uint32_t FAddr = (uint32_t)&MyBigUint;
    FLASH_Unlock();
    // Erase flash
    FLASH_ClearFlag(FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR);   // Clear All pending flags
    status = FLASH_ErasePage(FAddr);
    //klPrintf("  Flash erase %u: %u\r", i, FLASHStatus);
    if(status != OK) {
        Uart.Printf("  Flash erase error\r");
        return;
    }
    uint32_t *PRAM = (uint32_t*)PClr;    // What to write
    uint32_t DataWordCount = (sizeof(Color_t) + 3) / 4;
    chSysLock();
    for(uint32_t i=0; i<DataWordCount; i++) {
        status = FLASH_ProgramWord(FAddr, *PRAM);
        if(status != OK) {
            Uart.Printf("  Flash write error\r");
            return;
        }
        //else klPrintf("#");
        FAddr += 4;
        PRAM++;
    }
    FLASH_Lock();
    chSysUnlock();
}

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
