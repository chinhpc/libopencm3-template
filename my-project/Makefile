PROJECT = STM32F411VET6
BUILD_DIR = build
CURRENT_DIR = $(shell pwd)
#CURRENT_DIR = $(notdir $(shell pwd))
SWO_PARSER_DIR = ../SWO-Parser
TRACE_CFG_FILE = ../configure-trace.openocd

SHARED_DIR = include shared
CFILES = usart_stdio.c trace_stdio.c command.c tick.c my-project.c

# TODO - you will need to edit these two lines!
DEVICE=stm32f411vet6
#OOCD_FILE = board/stm32f4discovery.cfg
OOCD_INTERFACE=stlink-v2
OOCD_TARGET=stm32f4x

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR =../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

############# For testing coding syntax #############
TESTING_CFILES = rtc_lib.c
TESTING_OBJS = $(TESTING_CFILES:%.c=$(BUILD_DIR)/%.o)
test: $(TESTING_OBJS)
#	@echo $(TESTING_OBJS)
############# End For testing coding syntax #########
