
#include "stm32f10x.h"
#include "main.h"
#include "math.h"
#include "printf.h"

#define GPIDINT_MAX 950
#define GPIDINT_MIN 110

int16_t g_pid_integral = PI_MIN; // starting position to remove delay
int16_t pid_output = PI_MIN;


uint16_t calculate_pid(uint16_t current_value, uint16_t set_point)
{
	if (current_value > MINIMUM_SPEED && current_value < MAXIMUM_SPEED)
	{
		// PI regulator begin
		int16_t current_value_error = set_point - current_value;
		int16_t p_out = (KP * current_value_error) / 10; // 10 - KP should be float but not allowed
		g_pid_integral += current_value_error * (LOOP_PERIOD_MS / 1000); // 1000 - from millisec to sec
		int16_t i_out = (KI * g_pid_integral) / 10; // 10 - KI should be float but not allowed

		// g_pid_integral limits
		g_pid_integral < GPIDINT_MIN ? g_pid_integral = GPIDINT_MIN : FALSE;
		g_pid_integral > GPIDINT_MAX ? g_pid_integral = GPIDINT_MAX : FALSE;

		// p_out limits
		p_out < -P_MAX ? p_out = -P_MAX : FALSE;
		p_out > P_MAX ? p_out = P_MAX : FALSE;

		// i_out limits
		i_out < PI_MIN ? i_out = PI_MIN : FALSE;
		i_out > PI_MAX ? i_out = PI_MAX : FALSE;
		
		// i_out limits
		i_out < PI_MIN ? i_out = PI_MIN : FALSE;
		i_out > PI_MAX ? i_out = PI_MAX : FALSE;

		pid_output = p_out + i_out;
		// PI regulator end

		// PI limits
		pid_output < PI_MIN ? pid_output = PI_MIN : FALSE;
		pid_output > PI_MAX ? pid_output = PI_MAX : FALSE;

		printf("p %d i %d\r\n", p_out, i_out);
	}
	return (uint16_t)pid_output;
}
