#include "receiver.h"

static void *receiver(void *arg);
static void *receiver(void *arg){
	parameter *parg;
	parg = (parameter *)arg;
	read(parg->fd, parg->buf, parg->bytes_num);
	return 0;
}

__EXPORT pthread_t receiver_thread(int fd, uint8_t *buf, uint16_t bytes_num){
	parameter receiver_arg;
	
	receiver_arg.fd = fd;
	receiver_arg.buf = buf;
	receiver_arg.bytes_num = bytes_num;

	pthread_t receive_thread;
	pthread_create(&receive_thread, NULL, receiver, &receiver_arg);	
	//pthread_join(receive_thread, NULL);
	return receive_thread;
}
