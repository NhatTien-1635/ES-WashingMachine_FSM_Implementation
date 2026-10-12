#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

// Button identifiers
enum Button_ID : uint8_t
{
    BTN_RUN = 0,
    BTN_PAUSE,
    BTN_STOP,
    BTN_10_CENTS,
    BTN_20_CENTS,
    BTN_50_CENTS,
    BUTTON_COUNT
};

// Periodic scan task (call every 20ms)
void Task_ButtonScan(void);

// Button event queries (self-clearing flags)
bool Button_IsSinglePressed(Button_ID id);
bool Button_IsDoublePressed(Button_ID id);

#endif // BUTTON_H