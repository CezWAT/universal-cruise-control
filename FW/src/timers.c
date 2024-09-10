#include "stm32f10x.h"
#include "timers.h"
#include "main.h"

#define CIRCUMFERENCE_MM 1636

volatile uint16_t timestamp_1;
volatile uint16_t timestamp_2;
extern volatile uint8_t g_speed_ready;


uint16_t get_speed(void)
{
	// 1 tick = 0,125 us;

	uint16_t diff = timestamp_2 - timestamp_1;
	uint32_t time_ms = ((PRESCALE_TMR3 * diff) / 1000) / 2;
	uint32_t speed = (CIRCUMFERENCE_MM * 1000) / (time_ms * 6 * 6);

	TIM3->CNT = 0; // reset timer value
	g_speed_ready = SPEED_NOT_READY;
	return speed;
}


void TIM3_IRQHandler(void)
{
	if (g_speed_ready == SPEED_NOT_READY)
	{
		timestamp_1 = TIM3->CCR1;
		g_speed_ready = SPEED_WORKING;
	}
	else if (g_speed_ready == SPEED_WORKING)
	{
		timestamp_2 = TIM3->CCR1;
		g_speed_ready = SPEED_READY;
	}
	// CC1IF interrupt flag auto-clear on CCR1 read
	TIM3->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF);
}
