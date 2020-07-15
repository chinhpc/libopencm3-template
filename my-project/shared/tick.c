
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

volatile uint32_t system_millis;
volatile uint32_t system_s;

/* Called when systick fires */
void sys_tick_handler(void)
{
	if(++system_millis > 1000)
	{
		system_s++;
		system_millis = 0;
	}
}

// /* sleep for delay milliseconds */
// static void msleep(uint32_t delay)
// {
// 	uint32_t wake = system_millis + delay;
// 	while (wake > system_millis);
// }

/* Set up a timer to create 1mS ticks. */
void systick_setup(void);
void systick_setup(void)
{
	/* clock rate / 1000 to get 1mS interrupt rate */
	systick_set_reload(84000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();
	/* this done last */
	systick_interrupt_enable();
}