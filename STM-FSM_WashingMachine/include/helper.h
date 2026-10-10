/**
 * Không được động k là máy nổ :D
 */


#ifndef HELPER_H
#define HELPER_H

#include "stm32f1xx_hal.h"

#define PIN_LED_BLINK           GPIO_PIN_15
#define PIN_LED_RED             GPIO_PIN_13
#define PIN_LED_BLUE            GPIO_PIN_14

#define PIN_BUTTON_RUN          GPIO_PIN_0
#define PIN_BUTTON_PAUSE        GPIO_PIN_1
#define PIN_BUTTON_STOP         GPIO_PIN_2

#define PIN_BUTTON_10_CENTS     GPIO_PIN_1
#define PIN_BUTTON_20_CENTS     GPIO_PIN_2
#define PIN_BUTTON_50_CENTS     GPIO_PIN_15


void SetupPinMode();
void InitTIM();

#endif  