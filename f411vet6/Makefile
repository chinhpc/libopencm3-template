# /*
# * Copyright (C) 2019 ChinhPC <chinhphancong@outlook.com>
# *
# * Author: ChinhPC
# *
# * This file is free software: you can redistribute it and/or modify it
# * under the terms of the GNU General Public License as published by the
# * Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This file is distributed in the hope that it will be useful, but
# * WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# * See the GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License along
# * with this program.  If not, see <http://www.gnu.org/licenses/>.
# */

PROJECT := STM32F411VET6
BUILD_DIR := build
CFILES :=

PARALLEL_MAKE := $(shell nproc)
MAKEFLAGS += -j$(PARALLEL_MAKE)

$(info 	)
$(info [*****	MAKEFLAGS = ${MAKEFLAGS}	*****])
$(info 	)

# TODO - you will need to edit these two lines!
DEVICE=stm32f411vet6
OOCD_INTERFACE=stlink-v2
OOCD_TARGET=stm32f4x

# You shouldn't have to edit anything below here.
# OPENCM3_DIR = $(shell realpath ../libopencm3)
# FreeRTOS_DIR = $(shell realpath ../FreeRTOS)

OPENCM3_DIR = ../libopencm3
FreeRTOS_DIR = ../FreeRTOS

# All of the sources participating in the build are defined here
include rtos.mk
include user.mk

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

############# For testing coding syntax #############
TESTING_CFILES = rtc/rtc_lib.c
TESTING_OBJS = $(TESTING_CFILES:%.c=$(BUILD_DIR)/%.o)
test: $(TESTING_OBJS)
#	@echo $(TESTING_OBJS)
############# End For testing coding syntax #########
