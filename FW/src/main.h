#ifndef MAIN_H
#define MAIN_H

// clock and prescalers
#define PCLK 8000000 // system clock from HSE
#define PRESCALE_PWM ((PCLK / (PCLK/100)) - 1) // counter
#define PRESCALE_TMR3 ((PCLK / (PCLK/100)) - 1) // counter
#define PRESCALE_TMR21 (PCLK / (PCLK/100))
#define SYSTICK_DELAY (PCLK / (PCLK/8000))

// timers config
#define LOOP_PERIOD_MS 3000 // main loop
#define WATCHDOG_TIMEOUT 5
#define INFO_LOOP 3000
#define BTN_LOOP 5
#define BTN_MANAGE_TIMEOUT 150

// HW values
#define ERROR_TR 3000 // critical error treshold value for ALARM
#define FLASH_PAGES 64
#define IWDG_EN 0xCCCC
#define IWDG_PRESCALER 1
#define IWDG_REFRESH 0xAAAA
#define UART_BAUD (PCLK/115200)
#define PULSE_FREQ 56 // @50 it was 44, compensated
#define PRESCALE_1MHZ 1000000

enum speed_reading
{
	SPEED_NOT_READY,
	SPEED_WORKING,
	SPEED_READY
};

enum buttons
{
    BUT_CANC,
    BUT_PLUS,
    BUT_MINUS,
    BUT_CLUTCH,
    BUT_BREAK
};

// PI loop values
#define KP 20 // proportional gain
#define KI 25 // integral gain
#define PI_MAX 2390
#define PI_MIN 270
#define P_MAX 32767
#define I_MAX 32767

#define MINIMUM_SPEED 25
#define MAXIMUM_SPEED 145

#define TRUE 1
#define FALSE 0

#endif
