#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "command.h"
#include "uart.h"

char **global_argv;
int global_argc;
FILE *fp;
int serial_port;

int main(int argc, char *argv[]) {

	char local_buf[256];
	char* local_ptr;
	global_argv = argv;
	global_argc = argc;
	fp = fopen(argv[2], "r");
	serial_port = serialport_setting(argv[1]);

	int_command();

	while(1) {

		printf("Please enter your messages\n");
		fflush(stdin);
		local_ptr = fgets(local_buf, 256, stdin);

		// uint8_t len = strlen(command_ptr);
		// if(command_ptr[len-1] == '\n')
		// 	command_ptr[len-1] = '\r';

		if (local_ptr != NULL){
			printf("You entered %hu character: %s\n", (uint8_t)strlen(local_ptr), local_ptr);
			if(!run_command(local_ptr)){
				printf("Not a command\nReceived messages: %s\n", local_ptr);
			}
		}
		else{
			printf("Can\'t get any character\n\r");
		}

		// FILE *file;
		// file = fopen("test", "r");
	
		// fseek(file, 6, SEEK_CUR);
	
		// int len = ftell(file);
	
		// printf("size = %d\n", len);

		}

	fclose(fp);
	close(serial_port); /* Close the serial port */
	return 0;
}