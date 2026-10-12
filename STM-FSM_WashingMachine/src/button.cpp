#include "button.h"
#include "helper.h"

// 75 ticks * 20ms = 1.5s double-click window
#define DOUBLE_CLICK_WINDOW_TICKS 75

struct ButtonHardware
{
    GPIO_TypeDef *port;
    uint16_t pin;
};

static const ButtonHardware BUTTON_MAP[BUTTON_COUNT] = {
    {GPIOB, PIN_BUTTON_RUN},
    {GPIOB, PIN_BUTTON_PAUSE},
    {GPIOB, PIN_BUTTON_STOP},
    {GPIOA, PIN_BUTTON_10_CENTS},
    {GPIOA, PIN_BUTTON_20_CENTS},
    {GPIOA, PIN_BUTTON_50_CENTS}
};

static uint8_t  prev_pin_state[BUTTON_COUNT]     = {GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, 
                                                    GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET};
static bool     single_press_flag[BUTTON_COUNT]  = {false};
static bool     double_press_flag[BUTTON_COUNT]  = {false};

static uint8_t  press_count[BUTTON_COUNT]        = {0};
static uint16_t double_click_timer[BUTTON_COUNT] = {0};

void Task_ButtonScan(void) 
{
    static bool is_first_scan = true;
    if (is_first_scan)
    {
        for (uint8_t btn_id = 0; btn_id < BUTTON_COUNT; ++btn_id)
        {
            prev_pin_state[btn_id] = HAL_GPIO_ReadPin(BUTTON_MAP[btn_id].port, BUTTON_MAP[btn_id].pin);
            single_press_flag[btn_id] = false;
            double_press_flag[btn_id] = false;
            press_count[btn_id] = 0;
            double_click_timer[btn_id] = 0;
        }
        is_first_scan = false;
        return;
    }

    for (uint8_t btn_id = 0; btn_id < BUTTON_COUNT; ++btn_id)
    {
        uint8_t current_state = HAL_GPIO_ReadPin(BUTTON_MAP[btn_id].port, BUTTON_MAP[btn_id].pin);

        // Detect falling edge (active-low pull-up input)
        bool is_falling_edge = (current_state == GPIO_PIN_RESET && prev_pin_state[btn_id] == GPIO_PIN_SET);
        prev_pin_state[btn_id] = current_state;

        if (is_falling_edge)
        {
            single_press_flag[btn_id] = true;

            // Double-click detection logic
            if (press_count[btn_id] == 0)
            {
                press_count[btn_id] = 1;
                double_click_timer[btn_id] = DOUBLE_CLICK_WINDOW_TICKS;
            }
            else if (press_count[btn_id] == 1 && double_click_timer[btn_id] > 0)
            {
                double_press_flag[btn_id] = true;
                press_count[btn_id] = 0;
                double_click_timer[btn_id] = 0;
            }
        }

        // Decrement double-click window timer
        if (double_click_timer[btn_id] > 0)
        {
            double_click_timer[btn_id]--;
            if (double_click_timer[btn_id] == 0)
            {
                press_count[btn_id] = 0;
            }
        }
    }
}

bool Button_IsSinglePressed(Button_ID btn_id) 
{
    if (btn_id >= BUTTON_COUNT) return false;
    if (single_press_flag[btn_id])
    {
        single_press_flag[btn_id] = false;
        return true;
    }
    return false;
}

bool Button_IsDoublePressed(Button_ID btn_id) 
{
    if (btn_id >= BUTTON_COUNT) return false;
    if (double_press_flag[btn_id])
    {
        double_press_flag[btn_id] = false;
        return true;
    }
    return false;
}