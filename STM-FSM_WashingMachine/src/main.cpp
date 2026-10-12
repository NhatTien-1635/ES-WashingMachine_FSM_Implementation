#include "helper.h"
#include "button.h"
#include "led_display.h"
#include "scheduler.h"
#include "fsm.h"

// Heartbeat LED task
void Task_BlinkLED()
{
    HAL_GPIO_TogglePin(GPIOB, PIN_LED_BLINK);
}

int main(void)
{
    HAL_Init();
    SetupPinMode();
    InitTIM();
    LED_Display_Init();

    Scheduler_AddTask(Task_BlinkLED, 13, 100);
    Scheduler_AddTask(Task_ButtonScan, 2, 2);
    Scheduler_AddTask(Task_FSM, 5, 10);
    Scheduler_AddTask(Task_LEDDisplay, 1, 1);

    while (1)
    {
        Scheduler_Dispatch();
        __WFI(); // Wait for interrupt (reduces CPU load in Proteus)
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        Scheduler_Step();
    }
}
