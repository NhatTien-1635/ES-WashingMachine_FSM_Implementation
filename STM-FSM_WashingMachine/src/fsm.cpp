#include "fsm.h"
#include "button.h"
#include "led_display.h"
#include "helper.h"

#define SIM_MODE    1

#if SIM_MODE
    #define WASH_DURATION_SEC 30       // 30s test duration
#else
    #define WASH_DURATION_SEC (30*60)  // 30min real-time duration
#endif

static uint16_t current_money = 0;
static uint16_t washing_time_remaining = 0;

static uint8_t timer_100ms_counter = 0;

static void EnterNextState(FSM_Event event);

static void StateFunction_INIT();
static void StateFunction_STANDBY();
static void StateFunction_READY();
static void StateFunction_RUNNING();
static void StateFunction_PAUSED();
static void StateFunction_ERROR();

static void UpdateSystemTimer();

static void (*state_function_table[STATE_COUNT])() = {0};
static FSM_State state_transition_table[STATE_COUNT * EVENT_COUNT] = {WM_STATE_ERROR};

static FSM_State current_state = WM_STATE_STANDBY;
static void (*current_state_function)();
static bool init = true;

// =====================================================================
//                              FSM TASK
// =====================================================================

void Task_FSM() 
{
    if (init)
    {
        StateFunction_INIT();
        init = false;
    }

    UpdateSystemTimer();
    current_state_function();
}

/**
 * @brief Transition to the next state based on current state and triggered event
 */
static void EnterNextState(FSM_Event event) 
{
    FSM_State next_state = state_transition_table[GET_TRANSITION_INDEX(current_state, event)];
    current_state = next_state;
    current_state_function = state_function_table[current_state];
}

static void StateFunction_INIT() 
{
    state_function_table[FSM_State::WM_STATE_STANDBY] = StateFunction_STANDBY;
    state_function_table[FSM_State::WM_STATE_READY]   = StateFunction_READY;
    state_function_table[FSM_State::WM_STATE_RUNNING] = StateFunction_RUNNING;
    state_function_table[FSM_State::WM_STATE_PAUSED]  = StateFunction_PAUSED;
    state_function_table[FSM_State::WM_STATE_ERROR]   = StateFunction_ERROR;

    // Default: unhandled events stay in current state
    for (int s = 0; s < STATE_COUNT; ++s)
    {
        for (int e = 0; e < EVENT_COUNT; ++e)
        {
            state_transition_table[GET_TRANSITION_INDEX(s, e)] = (FSM_State)s;
        }
    }

    // 1. STANDBY transitions
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_STANDBY, WM_EV_COIN)]           = WM_STATE_READY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_STANDBY, WM_EV_FAULT)]          = WM_STATE_ERROR;

    // 2. READY transitions
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_READY, WM_EV_BTN_RUN)]          = WM_STATE_RUNNING;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_READY, WM_EV_FAULT)]            = WM_STATE_ERROR;

    // 3. RUNNING transitions
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_RUNNING, WM_EV_BTN_PAUSE)]      = WM_STATE_PAUSED;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_RUNNING, WM_EV_BTN_STOP_TWICE)] = WM_STATE_STANDBY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_RUNNING, WM_EV_TIMEOUT)]        = WM_STATE_STANDBY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_RUNNING, WM_EV_FAULT)]          = WM_STATE_ERROR;

    // 4. PAUSED transitions
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_PAUSED, WM_EV_BTN_RUN)]         = WM_STATE_RUNNING;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_PAUSED, WM_EV_BTN_STOP_TWICE)]  = WM_STATE_STANDBY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_PAUSED, WM_EV_TIMEOUT)]         = WM_STATE_STANDBY;
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_PAUSED, WM_EV_FAULT)]           = WM_STATE_ERROR;

    // 5. ERROR transitions
    state_transition_table[GET_TRANSITION_INDEX(WM_STATE_ERROR, WM_EV_BTN_STOP_TWICE)]   = WM_STATE_STANDBY;

    current_state = WM_STATE_STANDBY;
    current_state_function = StateFunction_STANDBY;
    current_money = 0;
    washing_time_remaining = 0;

    LED_Display_SetRed(LED_MODE_ON);
    LED_Display_SetBlue(LED_MODE_OFF);
}

static void UpdateSystemTimer() 
{
    timer_100ms_counter++;

    if (timer_100ms_counter >= 10)
    {
        timer_100ms_counter = 0;

        // Countdown timer runs in both RUNNING and PAUSED states
        if (current_state == WM_STATE_RUNNING || current_state == WM_STATE_PAUSED)
        {
            if (washing_time_remaining > 0)
            {
                washing_time_remaining--;
                if (washing_time_remaining == 0)
                {
                    EnterNextState(WM_EV_TIMEOUT);
                }
            }
        }
    }
}

static void StateFunction_STANDBY() 
{
    LED_Display_SetRed(LED_MODE_ON);
    LED_Display_SetBlue(LED_MODE_OFF);

    // Coin inputs
    if (Button_IsSinglePressed(BTN_10_CENTS)) current_money += 10;
    if (Button_IsSinglePressed(BTN_20_CENTS)) current_money += 20;
    if (Button_IsSinglePressed(BTN_50_CENTS)) current_money += 50;

    if (current_money >= 50)
    {
        LED_Display_TurnOff7Seg(); // Turn off 7-seg when ready
        EnterNextState(WM_EV_COIN);
    }
    else
    {
        LED_Display_ShowNumber(current_money); // Display current balance (00, 10, 20, 30, 40)
    }
}

static void StateFunction_READY() 
{
    LED_Display_SetRed(LED_MODE_OFF);
    LED_Display_SetBlue(LED_MODE_ON);
    LED_Display_TurnOff7Seg(); // Turn off 7-seg in READY state

    // Accept extra coins without refund
    if (Button_IsSinglePressed(BTN_10_CENTS)) current_money += 10;
    if (Button_IsSinglePressed(BTN_20_CENTS)) current_money += 20;
    if (Button_IsSinglePressed(BTN_50_CENTS)) current_money += 50;

    // Start washing: clear balance and activate timer
    if (Button_IsSinglePressed(BTN_RUN))
    {
        current_money = 0;
        washing_time_remaining = WASH_DURATION_SEC;
        EnterNextState(WM_EV_BTN_RUN);
    }
}

static void StateFunction_RUNNING() 
{
    LED_Display_SetRed(LED_MODE_OFF);
    LED_Display_SetBlue(LED_MODE_BLINK);
    LED_Display_ShowNumber(washing_time_remaining); // Display countdown

    if (Button_IsSinglePressed(BTN_PAUSE))
    {
        EnterNextState(WM_EV_BTN_PAUSE);
    } 
    else if (Button_IsDoublePressed(BTN_STOP))
    {
        EnterNextState(WM_EV_BTN_STOP_TWICE); // Force stop
    }
}

static void StateFunction_PAUSED() 
{
    LED_Display_SetRed(LED_MODE_OFF);
    LED_Display_SetBlue(LED_MODE_ON);
    LED_Display_ShowNumber(washing_time_remaining); // Timer still counts down

    if (Button_IsSinglePressed(BTN_RUN))
    {
        EnterNextState(WM_EV_BTN_RUN); // Resume washing
    }
    else if (Button_IsDoublePressed(BTN_STOP))
    {
        EnterNextState(WM_EV_BTN_STOP_TWICE); // Force stop
    }
}

static void StateFunction_ERROR() 
{
    LED_Display_SetRed(LED_MODE_BLINK);
    LED_Display_SetBlue(LED_MODE_OFF);
    LED_Display_TurnOff7Seg();

    if (Button_IsDoublePressed(BTN_STOP)) {
        EnterNextState(WM_EV_BTN_STOP_TWICE); // Reset error
    }
}