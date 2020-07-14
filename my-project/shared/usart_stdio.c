
#define _GNU_SOURCE
#include <libopencm3/stm32/usart.h>
#include <ctype.h>
#include "usart_stdio.h"

static ssize_t _iord(void *_cookie, char *_buf, size_t _n);
static ssize_t _iowr(void *_cookie, const char *_buf, size_t _n);
static void get_buffered_line(uint32_t dev);
static inline void back_up(uint32_t dev);

/*
 * This is a pretty classic ring buffer for characters
 */
#define BUFLEN 127

static uint16_t start_ndx = 0;
static uint16_t end_ndx = 0;
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
static void get_buffered_line(uint32_t dev)
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
	int	my_len = 0;

	get_buffered_line(dev);
	if (buf_len <= 0){
		start_ndx = end_ndx = 0;
		return 0;
	}
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

FILE *usart_setup(uint32_t dev)
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