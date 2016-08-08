#include <px4_config.h>
#include <px4_adc.h>
#include <px4_posix.h>
#include <px4_log.h>

#include <sys/types.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <arch/board/board.h>
#include <drivers/drv_hrt.h>
#include <drivers/drv_adc.h>

#include <arch/stm32/chip.h>
#include <stm32.h>
#include <stm32_gpio.h>

#include <systemlib/err.h>
#include <systemlib/perf_counter.h>

#include <uORB/topics/system_power.h>
#include <uORB/topics/adc_report.h>


//#define ADC0_DEVICE_PATH "/dev/adc0"

__EXPORT int bv_again_main(int argc, char *argv[]);
uint32_t GetBatteryVoltage(void);

uint32_t GetBatteryVoltage(void){
	int ret;
	int fd = open(ADC0_DEVICE_PATH, O_RDONLY);
	if (fd < 0){
		PX4_ERR("ERROR: can't open ADC device");
		return 1;
	}
	struct adc_msg_s data[12];
	ssize_t count = read(fd, data, sizeof(data));
	if (count < 0) {
		if (fd != 0) px4_close(fd);
		return 0;
	}
	unsigned channels = count / sizeof(data[0]);
	for (unsigned j = 0;j < channels;j++){
		//printf("%d: %u ", data[j].am_channel, data[j].am_data);
		if (data[j].am_channel == 2){
			ret = data[j].am_data;
			px4_close(fd);
			return ret;
		}
	}
	px4_close(fd);
	return 0;
}


int bv_again_main(int argc, char *argv[]){
	uint32_t BatteryVoltage;
	for(uint32_t i=0;i<5;i++){
		BatteryVoltage = GetBatteryVoltage();
		printf("Battery Voltage is %u \n", BatteryVoltage);
		usleep(500000);
	}
	
	return 0;
}
