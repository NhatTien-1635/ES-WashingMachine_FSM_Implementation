#include "helper.h"

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

  // Set up LED pins
  GPIO_InitTypeDef led_gpio = {0};
  led_gpio.Pin = PIN_LED_BLINK | PIN_LED_RED | PIN_LED_BLUE;
  led_gpio.Mode = GPIO_MODE_OUTPUT_PP;
  led_gpio.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOB, &led_gpio);
  HAL_GPIO_WritePin(GPIOB, led_gpio.Pin, GPIO_PIN_RESET);

  // Set up state button
  GPIO_InitTypeDef state_button_gpio = {0};
  state_button_gpio.Pin = PIN_BUTTON_RUN | PIN_BUTTON_PAUSE | PIN_BUTTON_STOP;
  state_button_gpio.Mode = GPIO_MODE_INPUT;
  state_button_gpio.Pull = GPIO_PULLUP;

  HAL_GPIO_Init(GPIOB, &state_button_gpio);

  // Set up extra button
  GPIO_InitTypeDef extra_button_gpio = {0};
  extra_button_gpio.Pin = PIN_BUTTON_10_CENTS | PIN_BUTTON_20_CENTS | PIN_BUTTON_50_CENTS;
  extra_button_gpio.Mode = GPIO_MODE_INPUT;
  extra_button_gpio.Pull = GPIO_PULLUP;

  HAL_GPIO_Init(GPIOA, &extra_button_gpio);
}

void InitTIM()
{
  __HAL_RCC_TIM2_CLK_ENABLE();

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

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