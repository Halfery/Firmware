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

typedef struct {
	int fd;
	uint16_t bytes_num;
	uint8_t *buf;
} parameter;


extern pthread_t receiver_thread(int fd, uint8_t *buf, uint16_t bytes_num);
