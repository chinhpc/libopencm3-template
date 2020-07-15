
#include <stdlib.h>
#include <ctype.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <string.h>
#include "usart_stdio.h"
#include "command.h"

FILE *fp;

extern volatile uint32_t system_s;
void systick_setup(void);

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
	//rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
	/* Enable GPIOC clock for LED & USARTs. */
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Enable clocks for USART2. */
	rcc_periph_clock_enable(RCC_USART2);
}

static void gpio_setup(void)
{
	/* Setup GPIO pin GPIO8/9 on GPIO port D for LEDs. */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13 | GPIO15);

	/* Setup GPIO pins for USART2 transmit and receive. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3);

	/* Setup USART2 TX pin as alternate function. */
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3);
}

int main(void)
{
	int i, j = 0;
	char local_buf[100];
	char* local_ptr;
	uint32_t old_system_s = 0;
	int_command();
	clock_setup();
	systick_setup();
	gpio_setup();
	fp = usart_setup(USART2);
	/* Blink the LED (PD13, PD15) on the board with every transmitted byte. */
	gpio_set(GPIOD,GPIO13);
	gpio_clear(GPIOC, GPIO15);
	while (1) {
		if(old_system_s != system_s) {
			gpio_toggle(GPIOD, GPIO13 | GPIO15);	/* LED on/off */
			old_system_s = system_s;
		}
		printf("Please enter your messages\n");
		fprintf(fp, "\n\rEnter your messages: ");
		fflush(fp);
		local_ptr = fgets(local_buf, 100, fp);

		if (local_ptr != NULL){
			printf("Get %hu character\n", (uint8_t)strlen(local_ptr));
			if(!run_command(local_ptr)){
				printf("Not a command\n");
				fprintf(fp, "Received messages: %s", local_ptr);
			}
		}
		else{
			fprintf(fp, "Can\'t get any character\n\r");
			printf("Can\'t get any character\n\r");
		}

		// c = (c == 200) ? 0 : c + 1;	/* Increment c. */

		// for (i = 0; i < 1000000; i++) {	// Wait a bit
		// 	__asm__("NOP");
		// }
	}
	return 0;
}