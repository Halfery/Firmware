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
 * @file px4_simple_app.c
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
#include <drivers/drv_hrt.h>

#include <uORB/uORB.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_attitude.h>
#include <uORB/topics/vehicle_gps_position.h>

extern volatile uint32_t g_system_timer;
extern volatile uint32_t g_tickbias;

uint64_t utc_usec = 1475307183000000;//1470972573119610;

typedef struct {
	uint32_t year;
	uint8_t	month;
	uint16_t day;
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
	uint16_t msec;
	uint16_t usec;
} date;

__EXPORT int tgps_main(int argc, char *argv[]);

int tgps_main(int argc, char *argv[])
{
	PX4_INFO("Hello Sky!");
	hrt_abstime now_t;
	uint8_t tmpmonth;
	uint64_t time_diff;
	date utc_date;
	utc_date.usec = utc_usec % 1000;
	utc_usec = utc_usec / 1000;
	utc_date.msec = utc_usec % 1000;
	utc_usec = utc_usec / 1000;
	utc_date.sec = utc_usec % 60;
	utc_usec = utc_usec / 60;
	utc_date.min = utc_usec % 60;
	utc_usec = utc_usec / 60;
	utc_date.hr = utc_usec % 24;
	utc_usec = utc_usec / 24;
	utc_date.day = utc_usec % 365;
	printf("utc_date.day is %d\n", utc_date.day);
	utc_usec = utc_usec / 365;
	utc_date.month = 0;
	//utc_sec = utc_usec / 12;
	utc_date.year = utc_usec + 1970;
	if ((utc_date.year % 4) == 0){
		utc_date.day = utc_date.day - (utc_date.year - 1972)/4 + 1;
		//printf("utc_date.day is %d\n", utc_date.day);
		tmpmonth = utc_date.day/31;
		switch(tmpmonth){
			case 0:
				utc_date.month = 1;
				break;
			case 1:
				if ((utc_date.day % 31) == 0){
					utc_date.month = 1;
					break;
				}
				utc_date.month = 2;
				utc_date.day = utc_date.day - 31;
				if (utc_date.day > 29) {
					utc_date.month = 3;
					utc_date.day = utc_date.day - 29;
				}
				break;
			case 2:
				utc_date.month = 3;
				utc_date.day = utc_date.day - 31 - 29;
				if (utc_date.day > 31) {
					utc_date.month = 4;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 3:
				utc_date.month = 4;
				utc_date.day = utc_date.day - 31 * 2 - 29;
				if (utc_date.day > 30) {
					utc_date.month = 5;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 4:
				utc_date.month = 5;
				utc_date.day = utc_date.day - 31 * 2 - 29 -30;
				if (utc_date.day > 31) {
					utc_date.month = 6;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 5:
				utc_date.month = 6;
				utc_date.day = utc_date.day - 31 * 3 - 29 -30;
				if (utc_date.day > 30) {
					utc_date.month = 7;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 6:
				utc_date.month = 7;
				utc_date.day = utc_date.day - 31 * 3 - 29 -30 * 2;
				if (utc_date.day > 31) {
					utc_date.month = 8;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 7:
				utc_date.month = 8;
				utc_date.day = utc_date.day - 31 * 4 - 29 -30 * 2;
				if (utc_date.day > 31) {
					utc_date.month = 9;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 8:
				utc_date.month = 9;
				utc_date.day = utc_date.day - 31 * 5 - 29 -30 * 2;
				if (utc_date.day > 30) {
					utc_date.month = 10;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 9:
				utc_date.month = 10;
				utc_date.day = utc_date.day - 31 * 5 - 29 -30 * 3;
				if (utc_date.day > 31) {
					utc_date.month = 11;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 10:
				utc_date.month = 11;
				utc_date.day = utc_date.day - 31 * 6 - 29 -30 * 3;
				if (utc_date.day > 30) {
					utc_date.month = 12;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 11:
				utc_date.month = 12;
				utc_date.day = utc_date.day - 31 * 6 - 29 -30 * 4;
				break;
		}
	}
	else{
		utc_date.day = utc_date.day - (utc_date.year - 1972)/4;
		tmpmonth = utc_date.day/31;
		switch(tmpmonth){
			case 0:
				utc_date.month = 1;
				break;
			case 1:
				if ((utc_date.day % 31) == 0){
					utc_date.month = 1;
					break;
				}
				utc_date.month = 2;
				utc_date.day = utc_date.day - 31;
				if (utc_date.day > 28) {
					utc_date.month = 3;
					utc_date.day = utc_date.day - 28;
				}
				break;
			case 2:
				utc_date.month = 3;
				utc_date.day = utc_date.day - 31 - 28;
				if (utc_date.day > 31) {
					utc_date.month = 4;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 3:
				utc_date.month = 4;
				utc_date.day = utc_date.day - 31 * 2 - 28;
				if (utc_date.day > 30) {
					utc_date.month = 5;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 4:
				utc_date.month = 5;
				utc_date.day = utc_date.day - 31 * 2 - 28 -30;
				if (utc_date.day > 31) {
					utc_date.month = 6;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 5:
				utc_date.month = 6;
				utc_date.day = utc_date.day - 31 * 3 - 28 -30;
				if (utc_date.day > 30) {
					utc_date.month = 7;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 6:
				utc_date.month = 7;
				utc_date.day = utc_date.day - 31 * 3 - 28 -30 * 2;
				if (utc_date.day > 31) {
					utc_date.month = 8;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 7:
				utc_date.month = 8;
				utc_date.day = utc_date.day - 31 * 4 - 28 -30 * 2;
				if (utc_date.day > 31) {
					utc_date.month = 9;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 8:
				utc_date.month = 9;
				utc_date.day = utc_date.day - 31 * 5 - 28 -30 * 2;
				if (utc_date.day > 30) {
					utc_date.month = 10;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 9:
				utc_date.month = 10;
				utc_date.day = utc_date.day - 31 * 5 - 28 -30 * 3;
				if (utc_date.day > 31) {
					utc_date.month = 11;
					utc_date.day = utc_date.day - 31;
				}
				break;
			case 10:
				utc_date.month = 11;
				utc_date.day = utc_date.day - 31 * 6 - 28 -30 * 3;
				if (utc_date.day > 30) {
					utc_date.month = 12;
					utc_date.day = utc_date.day - 30;
				}
				break;
			case 11:
				utc_date.month = 12;
				utc_date.day = utc_date.day - 31 * 6 - 28 -30 * 4;
				break;
		}
	}
	
	printf("year = %d, month = %d, day = %d, hr = %d\n, min = %d, sec = %d, msec = %d, usec = %d\n", utc_date.year, utc_date.month, utc_date.day, \
		utc_date.hr, utc_date.min, utc_date.sec, utc_date.msec, utc_date.usec);

	/* subscribe to sensor_combined topic */
	//int sensor_sub_fd = orb_subscribe(ORB_ID(sensor_combined));
	//orb_set_interval(sensor_sub_fd, 1000);

	int vehicle_gps_fd = orb_subscribe(ORB_ID(vehicle_gps_position));
	orb_set_interval(vehicle_gps_fd, 100);

	/* advertise attitude topic */
	//struct vehicle_attitude_s att;
	//memset(&att, 0, sizeof(att));
	//orb_advert_t att_pub = orb_advertise(ORB_ID(vehicle_attitude), &att);

	/* one could wait for multiple topics with this technique, just using one here */
	px4_pollfd_struct_t fds[] = {
		{ .fd = vehicle_gps_fd,   .events = POLLIN },
		/* there could be more file descriptors here, in the form like:
		 * { .fd = other_sub_fd,   .events = POLLIN },
		 */
	};

	int error_counter = 0;

	for (int i = 0; i < 5; i++) {
		/* wait for sensor update of 1 file descriptor for 1000 ms (1 second) */
		int poll_ret = px4_poll(fds, 1, 1000);

		/* handle the poll result */
		if (poll_ret == 0) {
			/* this means none of our providers is giving us data */
			PX4_ERR("[px4_simple_app] Got no data within a second");

		} else if (poll_ret < 0) {
			/* this is seriously bad - should be an emergency */
			if (error_counter < 10 || error_counter % 50 == 0) {
				/* use a counter to prevent flooding (and slowing us down) */
				PX4_ERR("[px4_simple_app] ERROR return value from poll(): %d"
					, poll_ret);
			}

			error_counter++;

		} else {

			if (fds[0].revents & POLLIN) {
				/* obtained data for the first file descriptor */
				struct vehicle_gps_position_s raw;
				memset(&raw, 0, sizeof(raw));
				/* copy sensors raw data into local buffer */
				orb_copy(ORB_ID(vehicle_gps_position), vehicle_gps_fd, &raw);
				now_t = hrt_absolute_time();
				printf("[tgps] now time is:\t%"PRIu64"\n", now_t);
				printf("[tgps] g_system_timer is:\t%d\n", g_system_timer);
				printf("[tgps] g_tickbias is:\t%d\n", g_tickbias);
				printf("[tgps] timestamp:\t%"PRIu64"\n", raw.timestamp);
				time_diff = now_t - raw.timestamp;
				printf("[tgps] time diff:\t%"PRIu64"\n", time_diff);
				printf("[tgps] Latitude:\t%d\n", raw.lat);
				printf("[tgps] Longitude:\t%d\n", raw.lon);
				printf("[tgps] time_utc_usec:\t%"PRIu64"\n", raw.time_utc_usec);

				/* set att and publish this information for other apps */
				//att.roll = raw.accelerometer_m_s2[0];
				//att.pitch = raw.accelerometer_m_s2[1];
				//att.yaw = raw.accelerometer_m_s2[2];
				//orb_publish(ORB_ID(vehicle_attitude), att_pub, &att);
			}

			/* there could be more file descriptors here, in the form like:
			 * if (fds[1..n].revents & POLLIN) {}
			 */
		}
	}

	PX4_INFO("exiting");

	return 0;
}
