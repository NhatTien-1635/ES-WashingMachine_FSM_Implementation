#ifndef FSM_H
#define FSM_H

#include "stdint.h"

#include "helper.h"

#define STATE_COUNT 5
enum FSM_State : uint8_t
{
    WM_STATE_STANDBY = 0,         /* RLED on        - available, money < 50                */
    WM_STATE_READY,               /* BLED on        - money >= 50, waiting for RUN         */
    WM_STATE_RUNNING,             /* BLED blinking  - washing, timer counting down         */
    WM_STATE_PAUSED,              /* BLED on        - washing paused, timer still counting */
    WM_STATE_ERROR,               /* RLED blinking  - fault, waiting for STOP (reset)      */
};

#define EVENT_COUNT 6
enum FSM_Event : uint8_t
{
    WM_EV_COIN = 0,     /* a coin was inserted (value passed in 'coin')    */
    WM_EV_BTN_RUN,
    WM_EV_BTN_PAUSE,
    WM_EV_BTN_STOP,
    WM_EV_TIMEOUT,      /* 30-minute timer expired (raised by wm_tick())   */
    WM_EV_FAULT         /* hardware fault detected                         */
};

#define GET_TRANSITION_INDEX(state, event) ((state) * STATE_COUNT + (event))

void Task_FSM();

#endif