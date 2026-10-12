#ifndef FSM_H
#define FSM_H

#include <stdint.h>
#include "helper.h"

#define STATE_COUNT 5
enum FSM_State : uint8_t
{
    WM_STATE_STANDBY = 0,         /* RLED on        - available, balance < 50              */
    WM_STATE_READY,               /* BLED on        - balance >= 50, waiting for RUN       */
    WM_STATE_RUNNING,             /* BLED blinking  - washing, countdown active            */
    WM_STATE_PAUSED,              /* BLED on        - paused, countdown still active       */
    WM_STATE_ERROR,               /* RLED blinking  - error, waiting for STOP reset        */
};

#define EVENT_COUNT 6
enum FSM_Event : uint8_t
{
    WM_EV_COIN = 0,               /* Coin inserted                                         */
    WM_EV_BTN_RUN,                /* RUN button pressed                                    */
    WM_EV_BTN_PAUSE,              /* PAUSE button pressed                                  */
    WM_EV_BTN_STOP_TWICE,         /* STOP button pressed twice (force stop)                */
    WM_EV_TIMEOUT,                /* 30-minute washing cycle expired                       */
    WM_EV_FAULT                   /* Hardware fault detected                               */
};

// Row-major matrix index mapping: state * EVENT_COUNT + event
#define GET_TRANSITION_INDEX(state, event) ((state) * EVENT_COUNT + (event))

void Task_FSM();

#endif // FSM_H