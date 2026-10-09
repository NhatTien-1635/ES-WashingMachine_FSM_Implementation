#include "helper.h"

bool event = false;
int main(void)
{
    HAL_Init();

    SetupPinMode();
    InitTIM();

    while (1)
    {
        if(HAL_GPIO_ReadPin(GPIOB, PIN_BUTTON_STOP) == GPIO_PIN_RESET){
            event = true;
        }
    }
}

// Interupt every 10ms <- DO NOT FORGET
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if (event)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13);
            event = false;
        }
    }
}

