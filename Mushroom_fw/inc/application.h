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

#if 1 // ==== Timings ====

#endif

// ==== Application class ====
class App_t {
private:
public:
    Thread *PThd;
    void Init();
    void SendEvtRx() {}
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
