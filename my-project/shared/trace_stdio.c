/*
 * support for stdio output to a trace port
 */
#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/memorymap.h>
#include <libopencm3/cm3/itm.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#ifndef STIMULUS_STDIO
#define STIMULUS_STDIO 0
#endif

void trace_send_blocking8(int stimulus_port, char c);
int _write(int file, char *ptr, int len);

void trace_send_blocking8(int stimulus_port, char c)
{
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	while (!(ITM_STIM8(stimulus_port) & ITM_STIM_FIFOREADY));
	ITM_STIM8(stimulus_port) = c;
}

int _write(int file, char *ptr, int len)
{
	int i;

	if (file == STDOUT_FILENO || file == STDERR_FILENO) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n') {
				trace_send_blocking8(STIMULUS_STDIO, '\r');
			}
			trace_send_blocking8(STIMULUS_STDIO, ptr[i]);
		}
		return i;
	}
	errno = EIO;
	return -1;
}


