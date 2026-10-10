#include "helper.h"
#include "scheduler.h"
#include "fsm.h"

void Task_BlinkLED()
{
    HAL_GPIO_TogglePin(GPIOB, PIN_LED_BLINK);
}

int main(void)
{
    HAL_Init();
    SetupPinMode();
    InitTIM();

    Scheduler_AddTask(Task_BlinkLED, 100, 100); 
    Scheduler_AddTask(Task_FSM, 1, 100);

    while (1)
    {
        Scheduler_Dispatch();
        __WFI(); // Cho Proteus đỡ bùng nổ
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        Scheduler_Step();
    }
}
