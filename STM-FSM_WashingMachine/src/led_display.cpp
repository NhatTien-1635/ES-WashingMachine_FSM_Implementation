#include "led_display.h"

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================
// Indicator LEDs (RLED, BLED): Common Anode -> Active-LOW
#define INDICATOR_LED_ON   GPIO_PIN_RESET
#define INDICATOR_LED_OFF  GPIO_PIN_SET

#if USE_SEG_LED
#define EN_ACTIVE          GPIO_PIN_RESET
#define EN_INACTIVE        GPIO_PIN_SET
#define SEG_ACTIVE         GPIO_PIN_RESET
#define SEG_INACTIVE       GPIO_PIN_SET

// 7-segment font table (0 - 9): bit 0=a .. bit 6=g
static const uint8_t DIGIT_MAP[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// Segment pins
static const uint16_t SEG_PINS[7] = {
    PIN_SEG0, PIN_SEG1, PIN_SEG2, PIN_SEG3, PIN_SEG4, PIN_SEG5, PIN_SEG6
};

// 7-segment display states
static bool    seg7_enabled = true;
static uint8_t digit_tens = 0;
static uint8_t digit_units = 0;
static uint8_t scan_digit = 0; // 0: Left digit (EN0), 1: Right digit (EN1)

// Output 7-bit segment pattern to PB5..PB11
static void OutputSegments(uint8_t seg_pattern)
{
    for (uint8_t i = 0; i < 7; i++)
    {
        uint8_t is_on = (seg_pattern >> i) & 0x01;
        HAL_GPIO_WritePin(GPIOB, SEG_PINS[i], is_on ? SEG_ACTIVE : SEG_INACTIVE);
    }
}
#endif

// Indicator LED states
static LED_Mode rled_mode = LED_MODE_OFF;
static LED_Mode bled_mode = LED_MODE_OFF;

static uint16_t blink_tick_counter = 0;
static bool     blink_state = false;

void LED_Display_Init(void)
{
    rled_mode = LED_MODE_OFF;
    bled_mode = LED_MODE_OFF;

    HAL_GPIO_WritePin(GPIOB, PIN_LED_RED, INDICATOR_LED_OFF);
    HAL_GPIO_WritePin(GPIOB, PIN_LED_BLUE, INDICATOR_LED_OFF);

#if USE_SEG_LED
    // Disable both PNP transistors
    HAL_GPIO_WritePin(GPIOA, PIN_EN0, EN_INACTIVE);
    HAL_GPIO_WritePin(GPIOA, PIN_EN1, EN_INACTIVE);
    OutputSegments(0);

    LED_Display_ShowNumber(0);
#endif
}

void LED_Display_SetRed(LED_Mode mode)
{
    rled_mode = mode;
}

void LED_Display_SetBlue(LED_Mode mode)
{
    bled_mode = mode;
}

#if USE_SEG_LED
void LED_Display_ShowNumber(uint8_t number)
{
    seg7_enabled = true;
    if (number > 99) number = 99;
    digit_tens  = number / 10;
    digit_units = number % 10;
}

void LED_Display_TurnOff7Seg(void)
{
    seg7_enabled = false;
    // Disable PNP transistors (3.3V) and segments
    HAL_GPIO_WritePin(GPIOA, PIN_EN0, EN_INACTIVE);
    HAL_GPIO_WritePin(GPIOA, PIN_EN1, EN_INACTIVE);
    OutputSegments(0);
}
#endif

// Periodic display task (runs every 10ms)
void Task_LEDDisplay(void)
{
    // ------------------------------------------------------------------------
    // Indicator LEDs (RLED & BLED)
    // ------------------------------------------------------------------------
    blink_tick_counter++;
    if (blink_tick_counter >= 50)
    {
        blink_tick_counter = 0;
        blink_state = !blink_state;
    }

    // Red LED
    switch (rled_mode)
    {
        case LED_MODE_ON:
            HAL_GPIO_WritePin(GPIOB, PIN_LED_RED, INDICATOR_LED_ON);
            break;
        case LED_MODE_BLINK:
            HAL_GPIO_WritePin(GPIOB, PIN_LED_RED, blink_state ? INDICATOR_LED_ON : INDICATOR_LED_OFF);
            break;
        case LED_MODE_OFF:
        default:
            HAL_GPIO_WritePin(GPIOB, PIN_LED_RED, INDICATOR_LED_OFF);
            break;
    }

    // Blue LED
    switch (bled_mode)
    {
        case LED_MODE_ON:
            HAL_GPIO_WritePin(GPIOB, PIN_LED_BLUE, INDICATOR_LED_ON);
            break;
        case LED_MODE_BLINK:
            HAL_GPIO_WritePin(GPIOB, PIN_LED_BLUE, blink_state ? INDICATOR_LED_ON : INDICATOR_LED_OFF);
            break;
        case LED_MODE_OFF:
        default:
            HAL_GPIO_WritePin(GPIOB, PIN_LED_BLUE, INDICATOR_LED_OFF);
            break;
    }

#if USE_SEG_LED
    // ------------------------------------------------------------------------
    // 7-Segment Multiplexing
    // ------------------------------------------------------------------------
    if (!seg7_enabled)
    {
        HAL_GPIO_WritePin(GPIOA, PIN_EN0, EN_INACTIVE);
        HAL_GPIO_WritePin(GPIOA, PIN_EN1, EN_INACTIVE);
        OutputSegments(0);
        return;
    }

    // Disable both digits before updating segments
    HAL_GPIO_WritePin(GPIOA, PIN_EN0, EN_INACTIVE);
    HAL_GPIO_WritePin(GPIOA, PIN_EN1, EN_INACTIVE);

    if (scan_digit == 0)
    {
        // Tens digit on left 7-seg-led
        OutputSegments(DIGIT_MAP[digit_tens]);
        HAL_GPIO_WritePin(GPIOA, PIN_EN0, EN_ACTIVE);
        HAL_GPIO_WritePin(GPIOA, PIN_EN1, EN_INACTIVE);
        scan_digit = 1;
    }
    else
    {
        // Units digit on right 7-seg-led
        OutputSegments(DIGIT_MAP[digit_units]);
        HAL_GPIO_WritePin(GPIOA, PIN_EN1, EN_ACTIVE);
        HAL_GPIO_WritePin(GPIOA, PIN_EN0, EN_INACTIVE);
        scan_digit = 0;
    }
#endif
}
