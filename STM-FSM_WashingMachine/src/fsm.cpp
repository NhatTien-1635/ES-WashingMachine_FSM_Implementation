#include "fsm.h"

static void EnterNextState(FSM_Event event);

static void StateFunction_INIT();
static void StateFunction_STANDBY();
static void StateFunction_READY();
static void StateFunction_RUNNING();
static void StateFunction_PAUSED();
static void StateFunction_ERROR();

static void (*state_function_table[STATE_COUNT])() = {0};
static FSM_State state_transition_table[STATE_COUNT * EVENT_COUNT] = {WM_STATE_ERROR};

static FSM_State current_state;
static void (*current_state_function)();
static bool init = true;

void Task_FSM()
{
    if (init)
    {
        StateFunction_INIT();
        init = false;
    }

    current_state_function();
}

/**
 * @brief Enter the next state based on the current state and event
 * @return The current_state_function will be StateFunction_ERROR if the next state for the corresponding event of the current state doesn't exist
 */
static void EnterNextState(FSM_Event event)
{
    current_state_function = state_function_table[state_transition_table[GET_TRANSITION_INDEX(current_state, event)]];
}

static void StateFunction_INIT()
{
    state_function_table[FSM_State::WM_STATE_STANDBY]   = StateFunction_STANDBY;
    state_function_table[FSM_State::WM_STATE_READY]     = StateFunction_READY;
    state_function_table[FSM_State::WM_STATE_RUNNING]   = StateFunction_RUNNING;
    state_function_table[FSM_State::WM_STATE_PAUSED]    = StateFunction_PAUSED;
    state_function_table[FSM_State::WM_STATE_ERROR]     = StateFunction_ERROR;

    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_STANDBY, WM_EV_COIN)]      = WM_STATE_READY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_READY, WM_EV_BTN_RUN)]     = WM_STATE_RUNNING;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_RUNNING, WM_EV_BTN_PAUSE)] = WM_STATE_PAUSED;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_RUNNING, WM_EV_BTN_STOP)]  = WM_STATE_STANDBY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_RUNNING, WM_EV_TIMEOUT)]   = WM_STATE_STANDBY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_PAUSED, WM_EV_BTN_RUN)]    = WM_STATE_RUNNING;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_PAUSED, WM_EV_TIMEOUT)]    = WM_STATE_STANDBY;

    current_state = WM_STATE_STANDBY;
    current_state_function = StateFunction_STANDBY;
}

static void StateFunction_STANDBY()
{
    //TODO
    HAL_GPIO_TogglePin(GPIOB, PIN_LED_BLUE);    //Chạy test
    /**
     * VD chuyển state tiếp theo:
     * - Nhập đủ tiền thì gọi theo EV
     * - Gọi sai thì qua state ERROR
     */
    // EnterNextState(WM_EV_COIN);
}

static void StateFunction_READY()
{
    //TODO
}

static void StateFunction_RUNNING()
{
    //TODO
}

static void StateFunction_PAUSED()
{
    //TODO
}

static void StateFunction_ERROR()
{
    //TODO
}