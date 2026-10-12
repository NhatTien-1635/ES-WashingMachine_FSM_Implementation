#ifndef HELPER_H
#define HELPER_H

#include "stm32f1xx_hal.h"

// Indicator LEDs (GPIOB)
#define PIN_LED_BLINK           GPIO_PIN_15
#define PIN_LED_RED             GPIO_PIN_13
#define PIN_LED_BLUE            GPIO_PIN_14

// State control buttons (GPIOB)
#define PIN_BUTTON_RUN          GPIO_PIN_0
#define PIN_BUTTON_PAUSE        GPIO_PIN_1
#define PIN_BUTTON_STOP         GPIO_PIN_2

// Coin input buttons (GPIOA)
#define PIN_BUTTON_10_CENTS     GPIO_PIN_1
#define PIN_BUTTON_20_CENTS     GPIO_PIN_2
#define PIN_BUTTON_50_CENTS     GPIO_PIN_15

// 7-segment digit enable pins (GPIOA)
#define PIN_EN0                 GPIO_PIN_4
#define PIN_EN1                 GPIO_PIN_5

// 7-segment segment pins (GPIOB)
#define PIN_SEG0                GPIO_PIN_5
#define PIN_SEG1                GPIO_PIN_6
#define PIN_SEG2                GPIO_PIN_7
#define PIN_SEG3                GPIO_PIN_8
#define PIN_SEG4                GPIO_PIN_9
#define PIN_SEG5                GPIO_PIN_10
#define PIN_SEG6                GPIO_PIN_11

void SetupPinMode();
void InitTIM();

#endif // HELPER_H