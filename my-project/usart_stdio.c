/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
//#include <errno.h>
//#include <stddef.h>
//#include <sys/types.h>
//#include <stdlib.h>
#include <ctype.h>

static ssize_t _iord(void *_cookie, char *_buf, size_t _n);
static ssize_t _iowr(void *_cookie, const char *_buf, size_t _n);
void get_buffered_line(uint32_t dev);

/*
 * This is a pretty classic ring buffer for characters
 */
#define BUFLEN 127

static uint16_t start_ndx;
static uint16_t end_ndx;
static char buf[BUFLEN+1];
#define buf_len ((end_ndx - start_ndx) % BUFLEN)
static inline int inc_ndx(int n) { return ((n + 1) % BUFLEN); }
static inline int dec_ndx(int n) { return (((n + BUFLEN) - 1) % BUFLEN); }

/* back up the cursor one space */
static inline void back_up(uint32_t dev)
{
	end_ndx = dec_ndx(end_ndx);
	usart_send_blocking(dev, '\010');
	usart_send_blocking(dev, ' ');
	usart_send_blocking(dev, '\010');
}

/*
 * A buffered line editing function.
 */
void get_buffered_line(uint32_t dev)
{
	char c;

	if (start_ndx != end_ndx) {
		return;
	}
	while (1) {
		c = usart_recv_blocking(dev);
		if (c == '\r') {
			buf[end_ndx] = '\n';
			end_ndx = inc_ndx(end_ndx);
			buf[end_ndx] = '\0';
			usart_send_blocking(dev, '\r');
			usart_send_blocking(dev, '\n');
			return;
		}
		/* ^H or DEL erase a character */
		if ((c == '\010') || (c == '\177')) {
			if (buf_len == 0) {
				usart_send_blocking(dev, '\a');
			} else {
				back_up(dev);
			}
		/* ^W erases a word */
		} else if (c == 0x17) {
			while ((buf_len > 0) &&
					(!(isspace((int) buf[end_ndx])))) {
				back_up(dev);
			}
		/* ^U erases the line */
		} else if (c == 0x15) {
			while (buf_len > 0) {
				back_up(dev);
			}
		/* Non-editing character so insert it */
		} else {
			if (buf_len == (BUFLEN - 1)) {
				usart_send_blocking(dev, '\a');
			} else {
				buf[end_ndx] = c;
				end_ndx = inc_ndx(end_ndx);
				usart_send_blocking(dev, c);
			}
		}
	}
}

static ssize_t _iord(void *_cookie, char *_buf, size_t _n)
{
	uint32_t dev = (uint32_t)_cookie;
	int	my_len;

	get_buffered_line(dev);
	my_len = 0;
	while ((buf_len > 0) && (_n > 0)) {
		*_buf++ = buf[start_ndx];
		start_ndx = inc_ndx(start_ndx);
		my_len++;
		_n--;
	}
	return my_len; /* return the length we got */
}

static ssize_t _iowr(void *_cookie, const char *_buf, size_t _n)
{
	uint32_t dev = (uint32_t)_cookie;

	int written = 0;
	while (_n-- > 0) {
		usart_send_blocking(dev, *_buf++);
		written++;
	};
	return written;
}


static FILE *usart_setup(uint32_t dev)
{
	/* Setup USART2 parameters. */
	usart_set_baudrate(dev, 115200);
	usart_set_databits(dev, 8);
	usart_set_parity(dev, USART_PARITY_NONE);
	usart_set_stopbits(dev, USART_STOPBITS_1);
	usart_set_mode(dev, USART_MODE_TX_RX);
	usart_set_flow_control(dev, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(dev);

	cookie_io_functions_t stub = { _iord, _iowr, NULL, NULL };
	FILE *fp = fopencookie((void *)dev, "rw+", stub);
	/* Do not buffer the serial line */
	setvbuf(fp, NULL, _IONBF, 0);
	return fp;

}

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
	int i, j = 0, c = 0;
	FILE *fp;

	clock_setup();
	gpio_setup();

	fp = usart_setup(USART2);

	/* Blink the LED (PD13, PD15) on the board with every transmitted byte. */
	gpio_set(GPIOD,GPIO13);
	gpio_clear(GPIOC, GPIO15);
	while (1) {
		gpio_toggle(GPIOD, GPIO13 | GPIO15);	/* LED on/off */

		fprintf(fp, "Pass: %d\n\r", c);
		printf("Pass: %d\n\r", c);

		c = (c == 200) ? 0 : c + 1;	/* Increment c. */

		for (i = 0; i < 1000000; i++) {	/* Wait a bit. */
			__asm__("NOP");
		}
	}
	return 0;
}
