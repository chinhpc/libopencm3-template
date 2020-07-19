//#define _GNU_SOURCE
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <errno.h>   /* ERROR Number Definitions           */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "uart.h"

int serialport_setting(char* dev)
{
//int fd;/*File Descriptor*/
int fd;
printf("\n +----------------------------------+");
printf("\n |       Serial Port Setting        |");
printf("\n +----------------------------------+");
/*------------------------------- Opening the Serial Port -------------------------------*/
/* Change /dev/ttyAMA0 to the one corresponding to your system */
fd = open(dev,O_RDWR | O_NOCTTY | O_NDELAY);	/* ttyAMA0 is the FT232 based USB2SERIAL Converter   */
/* O_RDWR Read/Write access to serial port           */
/* O_NOCTTY - No terminal will control the process   */
/* O_NDELAY -Non Blocking Mode,Does not care about-  */
/* -the status of DCD line,Open() returns immediatly */
if(fd == -1)
/* Error Checking */
printf("\n  Error! in Opening %s\n", dev);
else
printf("\n  %s Opened Successfully\n", dev);
/*---------- Setting the Attributes of the serial port using termios structure --------- */
struct termios SerialPortSettings;	/* Create the structure                          */
tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */
cfsetispeed(&SerialPortSettings,B115200); /* Set Read  Speed as 115200                       */
cfsetospeed(&SerialPortSettings,B115200); /* Set Write Speed as 115200                       */
SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */
SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */
SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
printf("\n  ERROR ! in Setting attributes\n");
else
printf("\n  BaudRate = 115200 \n  StopBits = 1 \n  Parity   = none\n");
return fd;
}

int serialport_write_string(int dev, char* str)
{
	char* data;
	size_t len = strlen(str);
	uint8_t bytes_written = 0;

	data = malloc(len + 2);
	strcpy(data, str);
	data[len + 1] = '\0';
	data[len] = '\r';
	len = strlen(data);
	bytes_written = write(dev, data, len);
	free(data);
	return bytes_written;
}