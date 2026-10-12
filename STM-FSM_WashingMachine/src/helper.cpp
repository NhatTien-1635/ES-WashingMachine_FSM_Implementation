#include "helper.h"
#include "led_display.h"

static TIM_HandleTypeDef htim2;

extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

extern "C" void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void SetupPinMode()
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    // Disable JTAG-DP to release PA15 (50-cent button), PB3, PB4 as GPIO; keep SW-DP enabled
    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    // Indicator LEDs
    GPIO_InitTypeDef led_gpio = {0};
    led_gpio.Pin = PIN_LED_BLINK | PIN_LED_RED | PIN_LED_BLUE;
    led_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    led_gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOB, &led_gpio);
    // Turn off indicator LEDs (Active-Low: SET = OFF) and heartbeat LED (Active-High: RESET = OFF)
    HAL_GPIO_WritePin(GPIOB, PIN_LED_RED | PIN_LED_BLUE, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, PIN_LED_BLINK, GPIO_PIN_RESET);

    // State control buttons
    GPIO_InitTypeDef state_button_gpio = {0};
    state_button_gpio.Pin = PIN_BUTTON_RUN | PIN_BUTTON_PAUSE | PIN_BUTTON_STOP;
    state_button_gpio.Mode = GPIO_MODE_INPUT;
    state_button_gpio.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(GPIOB, &state_button_gpio);

    // Coin input buttons
    GPIO_InitTypeDef extra_button_gpio = {0};
    extra_button_gpio.Pin = PIN_BUTTON_10_CENTS | PIN_BUTTON_20_CENTS | PIN_BUTTON_50_CENTS;
    extra_button_gpio.Mode = GPIO_MODE_INPUT;
    extra_button_gpio.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(GPIOA, &extra_button_gpio);

#if USE_SEG_LED
    // 7-segment digit enable pins
    GPIO_InitTypeDef en_gpio = {0};
    en_gpio.Pin = PIN_EN0 | PIN_EN1;
    en_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    en_gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &en_gpio);
    HAL_GPIO_WritePin(GPIOA, en_gpio.Pin, GPIO_PIN_SET); // Default inactive (PNP cutoff)

    // 7-segment segment pins4)
    GPIO_InitTypeDef seg_led_gpio = {0};
    seg_led_gpio.Pin = PIN_SEG0 | PIN_SEG1 | PIN_SEG2 | PIN_SEG3 | PIN_SEG4 | PIN_SEG5 | PIN_SEG6;
    seg_led_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    seg_led_gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOB, &seg_led_gpio);
    HAL_GPIO_WritePin(GPIOB, seg_led_gpio.Pin, GPIO_PIN_SET); // Default inactive
#endif
}

void InitTIM()
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    // 8MHz clock: 10ms interrupt period (Prescaler=7999 -> 1kHz, Period=9 -> 100Hz)
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7999;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 9;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
    
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Start_IT(&htim2);
}