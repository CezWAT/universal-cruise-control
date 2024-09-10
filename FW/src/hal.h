#ifndef INIT_H
#define INIT_H

extern volatile uint32_t g_time_tick;
extern volatile uint32_t target_speed;
extern volatile uint32_t last_speed;

void set_motor(uint16_t pwm);
uint8_t test_button_press(uint8_t gpio);
void watchdog_feed(void);
void simple_putchar(char message);
void init(void);

// exception handlers
void NMI_Handler(void);
void HardFault_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void IntDefaultHandler(void);

#endif
