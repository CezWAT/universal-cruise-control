#include "stm32f10x.h"
#include "main.h"
#include "hal.h"
#include "printf.h"

volatile uint8_t g_tim_speed_flag = 0;

void set_motor(uint16_t pwm)
{
	if (pwm >= PI_MIN && pwm <= PI_MAX)
	{
		TIM1->CCR4 = pwm;
	}
	else
	{
		TIM1->CCR4 = PI_MIN;
	}
	g_tim_speed_flag = 0;
}


uint8_t test_button_press(uint8_t gpio)
{
	uint8_t pressed = GPIOA->IDR & (1 << gpio);
	return pressed;
}


static void disable_interrupts(void)
{
	__set_PRIMASK(1);
}


static void enable_interrupts(void)
{
	__set_PRIMASK(0);
}


void watchdog_feed(void)
{
	disable_interrupts();
	IWDG->KR |= IWDG_REFRESH;
	enable_interrupts();
}


void simple_putchar(char message)
{
	USART1->DR = message; // fill the buffer with message
	while ((USART1->SR & USART_SR_TC) == 0); // wait until transfer complete
}


static void gpio_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	// BUT_CANC (PA0)
	GPIOA->CRL &= ~GPIO_CRL_CNF0_0;
	GPIOA->CRL |= GPIO_CRL_CNF0_1;
	// GPIOA->ODR &= ~GPIO_ODR_ODR0; // floating input to pull-down

	// BUT+ (PA1)
	GPIOA->CRL &= ~GPIO_CRL_CNF1_0; // floating input to pull-down
	GPIOA->CRL |= GPIO_CRL_CNF1_1;

	// BUT- (PA2)
	GPIOA->CRL &= ~GPIO_CRL_CNF2_0; // floating input to pull-down
	GPIOA->CRL |= GPIO_CRL_CNF2_1;

	// CLUTCH (PA3)
	GPIOA->CRL &= ~GPIO_CRL_CNF3_0; // floating input to pull-down
	GPIOA->CRL |= GPIO_CRL_CNF3_1;

	// BREAK (PA4)
	GPIOA->CRL &= ~GPIO_CRL_CNF4_0; // floating input to pull-down
	GPIOA->CRL |= GPIO_CRL_CNF4_1;

	// SPD (PA6)
	// leave as after reset - floating input

	// LED (PA5)
	GPIOA->CRL &=  ~(GPIO_CRL_CNF5_0 | GPIO_CRL_CNF5_1);
	GPIOA->CRL |= GPIO_CRL_MODE5_1; // set as output, max 2 MHz

	// UART
	// UART_TXO (PA9)
	GPIOA->CRH &= ~(GPIO_CRH_CNF9_0);
	GPIOA->CRH |= GPIO_CRH_CNF9_1;
	GPIOA->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0;

	// UART_RXI (PA10) floating input - reset state

	// TIM1 (PA11)
	GPIOA->CRH &= ~(GPIO_CRH_CNF11  | GPIO_CRH_MODE11);
	GPIOA->CRH |= (GPIO_CRH_MODE11_1 | GPIO_CRH_MODE11_1); // set as output, max 50 MHz
	GPIOA->CRH &= ~GPIO_CRH_CNF11_0; // set push-pull AF
	GPIOA->CRH |= GPIO_CRH_CNF11_1; // set push-pull AF

	// SPEED CONTROL
	// LEFT (PB12)
	GPIOB->CRH &= ~(GPIO_CRH_CNF12_0 | GPIO_CRH_CNF12_1);
	GPIOB->CRH |= GPIO_CRH_MODE12_1; // set as output, max 2 MHz

	// RIGHT (PB13)
	GPIOB->CRH &= ~(GPIO_CRH_CNF13_0 | GPIO_CRH_CNF13_1);
	GPIOB->CRH |= GPIO_CRH_MODE13_1; // set as output, max 2 MHz
}


// Set Timer 1 for setting motor position, PWM
static void timer_1_init(void)
{
	#define PRESCALE (SystemCoreClock / PRESCALE_1MHZ)
	#define PERIOD (SystemCoreClock / PRESCALE / PULSE_FREQ)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	TIM1->PSC = PRESCALE;
	TIM1->ARR = PERIOD; // PWM frequency
	TIM1->CCR4 = PI_MIN; // PWM duty cycle <---  270 ... 2390 --->
	TIM1->CCMR2 &= ~(TIM_CCMR2_CC4S_0 | TIM_CCMR2_CC4S_1); // CC4 as output
	TIM1->CCMR2 |= (TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2); // PWM Mode 1
	TIM1->CCMR2 |= TIM_CCMR2_OC4PE; // preload enable
	TIM1->CR1 |= TIM_CR1_ARPE; // auto-reload preload
	TIM1->EGR |= TIM_EGR_UG; // init all previous settings
	TIM1->CCER |= TIM_CCER_CC4E;
	TIM1->BDTR |= TIM_BDTR_MOE; // main output enable
	TIM1->CR1 |= TIM_CR1_CEN;
}


// Set Timer 3 for speed measurement
static void timer_3_init(void)
{
	TIM3->CCER &= ~TIM_CCER_CC1E;
	TIM3->CR2 &= ~TIM_CR2_TI1S; // make sure it is TI1
	TIM3->CCMR1 &= ~TIM_CCMR1_CC1S_1; // make sure to zero out CC1S_1
	TIM3->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_IC1F_2; // select Capture 1 | filter 2
	TIM3->CCER &= ~TIM_CCER_CC1P & ~TIM_CCER_CC1NP;
	TIM3->CCER |= TIM_CCER_CC1E; // enable Capture input
	TIM3->PSC = PRESCALE_TMR3;
	TIM3->EGR |= TIM_EGR_UG;
	TIM3->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE; // interrupt
	TIM3->CR1 |= TIM_CR1_CEN;
	NVIC_EnableIRQ(TIM3_IRQn);
}


static void watchdog_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_WWDGEN;
	IWDG->KR |= IWDG_EN;
	IWDG->PR |= IWDG_PRESCALER;
	IWDG->KR |= IWDG_REFRESH;
}


// configure USART1 for baudrate 115200, 8N1, FIFO
static void usart1_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	USART1->BRR = UART_BAUD;
	USART1->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
}


// use HSE clock
static void sysclk_init(void)
{
	RCC->CR |= RCC_CR_HSEON;
	RCC->CFGR = RCC_CFGR_SW_HSE;
	SystemCoreClockUpdate();
}


// execute initializations of all used peripherals
void init(void)
{
	sysclk_init();
	gpio_init();
	watchdog_init();
	timer_1_init(); // setting motor position
	timer_3_init(); // measurement
	usart1_init(); // UART
	SysTick_Config(SYSTICK_DELAY);

	simple_printf("Init done\r\n");
}


__attribute__((section(".after_vectors"))) void TIM1_IRQHandler()
{
	g_tim_speed_flag = 1;
}


__attribute__((section(".after_vectors"))) void HardFault_Handler()
{
	uint32_t lr = 0;
	__asm("MOV %0, LR\n" : "=r" (lr));
	uint32_t pc = 0;
	__asm("MOV %0, PC\n" : "=r" (pc));

	simple_printf("\tR0 (MSP): %x\r\n", __get_MSP());
	simple_printf("\tLR: %x\r\n", lr);
	simple_printf("\tPC: %x\r\n", pc);
	simple_printf("\txPSR: %x\r\n", __get_xPSR());
	simple_printf("\tAPSR: %x\r\n", __get_APSR());
	simple_printf("\tIPSR: %x, Exception: Hard Fault\r\n", __get_IPSR());

	NVIC_SystemReset();
}


__attribute__((section(".after_vectors"))) void SysTick_Handler()
{
	g_time_tick++;
}
