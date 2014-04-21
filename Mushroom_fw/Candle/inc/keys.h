/*
 * keys.h
 *
 *  Created on: 07.02.2013
 *      Author: kreyl
 */

#ifndef KEYS_H_
#define KEYS_H_

#include "hal.h"
#include "kl_lib_f100.h"
#include "evt_mask.h"

#if 1 // ========= Keys config =========
struct KeyData_t {
    GPIO_TypeDef *PGpio;
    uint16_t Pin;
    eventmask_t EvtMskPress, EvtMskRelease, EvtMskRepeat, EvtMskLongPress;
};

const KeyData_t KeyData[] = {
        {GPIOB, 6, EVTMSK_KEY_UP,       EVTMSK_NOTHING,        EVTMSK_KEY_UP,   EVTMSK_NOTHING},
        {GPIOB, 7, EVTMSK_KEY_DOWN,     EVTMSK_NOTHING,        EVTMSK_KEY_DOWN, EVTMSK_NOTHING},
        {GPIOA, 1, EVTMSK_KEY_TX_PRESS, EVTMSK_KEY_TX_RELEASE, EVTMSK_NOTHING,  EVTMSK_NOTHING},
};
#define KEYS_CNT    countof(KeyData)
#endif

// Timings
#define KEYS_POLL_PERIOD_MS             72
#define KEYS_KEY_BEFORE_REPEAT_DELAY_MS 999
#define KEY_REPEAT_PERIOD_MS            504
#define KEY_LONGPRESS_DELAY_MS          3006

// Key status
struct Key_t {
    bool IsPressed, IsRepeating, IsLongPress;
    systime_t RepeatTimer, LongPressTimer;
};

class Keys_t {
private:
    Key_t Key[KEYS_CNT];
public:
    void Init();
    void Shutdown() { for(uint8_t i=0; i<KEYS_CNT; i++) PinSetupAnalog(KeyData[i].PGpio, KeyData[i].Pin); }
    bool KeyUpIsPressed()   { return Key[0].IsPressed; }
    bool KeyDownIsPressed() { return Key[1].IsPressed; }
    bool KeyTxIsPressed()   { return Key[2].IsPressed; }
    // Inner use
    void ITask();
};

extern Keys_t Keys;

#endif /* KEYS_H_ */
