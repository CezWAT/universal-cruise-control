/*
Author      : CR
Version     : 1.0
Description : main file of cruise controller
MCU         : STM32F103C8
*/

// todo
/*
[v] przygotowanie "czystego" startu
	[V] sysclk
	[v] GPIO
	[v] timer odczytu prędkości
	[V] watchdog
	[V] uart
	[V] LED
	
[ ] PID
[v] odczyt prędkości
[ X ] odczyt obrotów silnika
[V] obsługa przycisku +
[V] obsługa przycisku -
[ ] obsługa przycisku CANC
[ ] obsługa przycisku sprzęgła
[ ] obsługa przycisku hamulca
[v] pamięć prędkości w RAM
[ ] "automatyczne" 50 km/h
[ ] ustawianie silnikiem przepustnicy
[ ] zabezpieczenia (obroty silnika > max; wciśnięty na stałe przycisk; timeout pomiaru prędkości)
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

int main(void)
{
	init();

	uint32_t watchdog_timeout = g_time_tick + WATCHDOG_TIMEOUT;
	uint32_t info_timeout = 0;
	uint32_t btn_plus_pressed = 0;
	uint32_t manage_buttons_timeout = 0;
	uint32_t btn_time_tick = 0;
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
			btn_time_tick = g_time_tick;
			
			if (test_button_press(BUT_CLUTCH) || test_button_press(BUT_BREAK))
			{
				set_motor(PI_MIN); // todo nie działa jeśli nie ma sygnału prędkości
				cc_enabled = FALSE;
				printf(" CLUTCH/BREAK \r\n");
			}

			if (test_button_press(BUT_CANC))
			{
				set_motor(PI_MIN); // todo nie działa jeśli nie ma sygnału prędkości
				last_speed = target_speed;
				cc_enabled = FALSE;
				printf(" CANC \r\n");
			}

			if (test_button_press(BUT_PLUS) && !cc_enabled)
			{
				target_speed = speed++;
				btn_plus_pressed = 1;
				cc_enabled = TRUE;
				printf("++\r\n");

				if (btn_plus_pressed && test_button_press(BUT_PLUS) && g_time_tick >= (btn_time_tick + 50)) // ustaw 50 km/h // TODO
				{
					target_speed = 50;
					printf(" set 50 \r\n");
					btn_plus_pressed = 0;
				}
			}
			else if (test_button_press(BUT_PLUS) && cc_enabled)
			{
				target_speed++;
				// target speed clamping
				target_speed > MAXIMUM_SPEED ? target_speed = MAXIMUM_SPEED : FALSE;
				cc_enabled = TRUE;
				printf("++\r\n");
			}
			
			if (test_button_press(BUT_MINUS))
			{
				if (test_button_press(BUT_MINUS) && (btn_time_tick + 120 > g_time_tick) 
					&& (cc_enabled == FALSE)) // todo czas ktory uplynal do poprawy
				{
					target_speed = last_speed;
					cc_enabled = TRUE;
				}
				else
				{
					target_speed--;
					// target speed clamping
					target_speed < MINIMUM_SPEED ? target_speed = MINIMUM_SPEED : FALSE;
				}
				printf("--\r\n");
			}

			manage_buttons_timeout = g_time_tick + BTN_MANAGE_TIMEOUT;
		}
	}
}
