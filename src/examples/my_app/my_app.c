/****************************************************************************
 *
 *   Copyright (c) 2012-2015 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file my_app.c
 * Minimal application example for PX4 autopilot
 *
 * @author Example User <mail@example.com>
 */

#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
//#include <irq.h>
#include <stm32_exti.h>
#include <termios.h>
#include <arch/board/board.h>

#include "receiver.h"
#include "wifi_usr.h"

#include <uORB/uORB.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_attitude.h>

__EXPORT int my_app_main(int argc, char *argv[]);

/*

static int PowerOff_isr(int irq, void *context);

static int PowerOff_isr(int irq, void *context){
	px4_arch_gpiowrite(GPIO_Power_On, 0);
	return 0;
}


typedef struct {
	int fd;
	uint16_t bytes_num;
	uint8_t *buf;
} parameter;

static void *receiver(void *arg){
	parameter *parg;
	parg = (parameter *)arg;
	read(parg->fd, parg->buf, parg->bytes_num);
	return 0;
}
*/

int my_app_main(int argc, char *argv[])
{
	//uint8_t buf[100];
	//parameter receiver_arg;
	PX4_INFO("In my_test_app");
	int uart1 = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (uart1 < 0) {
		printf("ERROR opening UART1, aborting..\n");
		return uart1;
	}
	struct termios uart1_config;
	int set = fcntl(uart1, F_SETFL, 0);
/*
	receiver_arg.fd = uart1;
	receiver_arg.buf = buf;
	receiver_arg.bytes_num = 1;
*/	

	//struct termios uart1_config_original;

	int termios_state = 0;

	int ret;

	if ((termios_state = tcgetattr(uart1, &uart1_config)) < 0) {
		printf("ERROR getting termios config for UART1: %d\n", termios_state);
		ret = termios_state;
		close(uart1);
		return ret;
	}

	if (cfsetspeed(&uart1_config, B115200) < 0) {
		printf("ERROR setting termios config for UART1: %d\n", termios_state);
		ret = ERROR;
		close(uart1);
		return ret;
	}

	if ((termios_state = tcsetattr(uart1, TCSANOW, &uart1_config)) < 0) {
		printf("ERROR setting termios config for UART1\n");
		ret = termios_state;
		close(uart1);
		return ret;
	}

	printf("c_iflag: %x, c_oflag: %x, c_cflag: %x, c_lflag: %x, speed: %d\n", uart1_config.c_iflag, uart1_config.c_oflag, uart1_config.c_cflag, uart1_config.c_lflag, uart1_config.c_speed);

	int r=0;

	wifi_init(uart1);
	//iniRD();
	r++;
	set++;
	close(uart1);
	return 0;
}
