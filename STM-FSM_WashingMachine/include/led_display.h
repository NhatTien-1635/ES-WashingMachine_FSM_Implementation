#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "helper.h"

// Configuration: 1 = enable 7-segment display, 0 = disable
#ifndef USE_SEG_LED
#define USE_SEG_LED             0
#endif

// Indicator LED modes
enum LED_Mode : uint8_t
{
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK
};

void LED_Display_Init(void);
void LED_Display_SetRed(LED_Mode mode);
void LED_Display_SetBlue(LED_Mode mode);

#if USE_SEG_LED
// Display number (0 - 99) on 7-segment displays
void LED_Display_ShowNumber(uint8_t number);

// Turn off both 7-segment displays
void LED_Display_TurnOff7Seg(void);
#else
// Empty inline stubs when 7-segment display is disabled
static inline void LED_Display_ShowNumber(uint8_t number) {}
static inline void LED_Display_TurnOff7Seg(void) {}
#endif

// Periodic task: multiplexes 7-segments and handles LED blinking
void Task_LEDDisplay(void);

#endif // LED_DISPLAY_H