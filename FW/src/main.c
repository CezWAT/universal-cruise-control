/*
Author      : CR
Version     : 1.0
Description : main file of cruise controller
MCU         : STM32F103C8
*/

// todo
/*
[v] "clean" start
	[V] sysclk
	[v] GPIO
	[v] timer for calculating speed
	[V] watchdog
	[V] uart
	[V] LED
	
[ ] PID
[v] speed reading
[ X ] read RPM
[?] PWM
[V] PLUS button working
[V] MINUS button working
[ ] CANCEL button working
[ ] clutch button working
[ ] break button working
[v] speed saved in RAM
[ ] "automatic" 50 km/h
[ ] setting throttle with el. motor
[ ] safety (RPM > max; button permanently pressed; speed measurement timeout)
*/

#include "stm32f10x.h"
#include "main.h"
#include "hal.h"
#include "timers.h"
#include "math.h"
#include "printf.h"


volatile uint32_t g_time_tick = LOOP_PERIOD_MS; // overflow for first start to happen right away
volatile uint32_t last_speed = 0;
volatile uint32_t target_speed = 0;
volatile uint8_t g_speed_ready;
volatile uint8_t click_count = 0;
volatile uint32_t last_click_time = 0;

int main(void)
{
	init();

	uint32_t watchdog_timeout = g_time_tick + WATCHDOG_TIMEOUT;
	uint32_t info_timeout = 0;
	uint32_t manage_buttons_timeout = 0;
	uint8_t cc_enabled = 0;
	uint32_t speed = 0;

	for (;;)
	{
		if (g_time_tick > watchdog_timeout)
		{
			watchdog_feed();
			watchdog_timeout = g_time_tick + WATCHDOG_TIMEOUT;
		}

		if (g_speed_ready == SPEED_READY)
		{
			uint32_t pwm_value = 0;
			speed = get_speed();

			// set servo/throttle
			if (target_speed >= MINIMUM_SPEED && target_speed <= MAXIMUM_SPEED  && cc_enabled && speed > 0)
			{
				pwm_value = calculate_pid(speed, target_speed);
				printf("target: %u speed: %u  pid %d\r\n", target_speed, speed, pwm_value); // debug	
				if (speed != target_speed)
				{
					set_motor(pwm_value);
				}
			}
			else if (!cc_enabled)
			{
				set_motor(PI_MIN); // disable cruise control
			}
		}

		if (g_time_tick > info_timeout)
		{
			GPIOA->ODR ^= (1 << 5);
			info_timeout = g_time_tick + INFO_LOOP;
		}

		// reaction to buttons
		if (g_time_tick > manage_buttons_timeout)
		{
// clutch & break switches
			if (gpio_read(BUT_CLUTCH) || gpio_read(BUT_BREAK))
			{
				set_motor(PI_MIN); // todo not working if there is no speed signal
				cc_enabled = FALSE;
				printf(" CLUTCH/BREAK \r\n");
			}
// cancel
			if (gpio_read(BUT_CANC))
			{
				set_motor(PI_MIN); // todo not working if there is no speed signal
				last_speed = target_speed;
				cc_enabled = FALSE;
				printf(" CANC \r\n");
			}
// plus
			if (multiclick_buttons(BUT_PLUS) == 1 && !cc_enabled)
			{
				target_speed = speed++;
				cc_enabled = TRUE;
				printf("++\r\n");
			}

			if (multiclick_buttons(BUT_PLUS) == 1 && cc_enabled)
			{
				target_speed++;
				// target speed clamping
				target_speed > MAXIMUM_SPEED ? target_speed = MAXIMUM_SPEED : FALSE;
				cc_enabled = TRUE;
				printf("++\r\n");
			}

			if (multiclick_buttons(BUT_PLUS) == 2)
			{
				target_speed = 50;
				printf(" set 50 \r\n");
			}
// minus
			if (gpio_read(BUT_MINUS) == 1 && !cc_enabled)
			{
				target_speed = last_speed;
				cc_enabled = TRUE;
				printf(" set last speed \r\n");
			}

			if (gpio_read(BUT_MINUS) == 1 && cc_enabled)
			{
				target_speed--;
				// target speed clamping
				target_speed < MINIMUM_SPEED ? target_speed = MINIMUM_SPEED : FALSE;
				printf("--\r\n");
			}
			manage_buttons_timeout = g_time_tick + BTN_MANAGE_TIMEOUT;
		}
	}
}
