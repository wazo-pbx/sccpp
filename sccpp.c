/* 
 * Nicolas Bouliane <nbouliane@avencall.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 */

/* Proof of concept of a tool 
 * used by Asterisk testsuite
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "device.h"
#include "message.h"
#include "phone.h"
#include "sccpp.h"
#include "utils.h"

void do_dial_extension(struct phone *phone, char *exten)
{
	int exten_len = 0;
	int i;
	int digit;

	exten_len = strlen(exten);
	for (i = 0; i < exten_len; i++) {

		if (exten[i] == '*') {
			digit = 14;
		} else if (exten[i] == '#') {
			digit = 15;
		} else {
			digit = exten[i] - 48;
		}

		transmit_keypad_button_message(phone, digit);
	}
}

void *caller(void *data)
{
	struct phone *phone = data;
	char *exten = NULL;
	int exten_len = 0;
	int i = 0;

	exten = strdup(phone->exten);
	exten_len = strlen(exten);
	srand(time(NULL));

	while (1) {

		transmit_offhook_message(phone);
		for (i = 0; i < exten_len; i++) {
			transmit_keypad_button_message(phone, exten[i] - 48);
		}
		sleep(rand() % (3 - 1 + 1) + 1);

		transmit_onhook_message(phone);
	}
}

int sccpp_test_stress(char *local_ip, char *remote_ip, char *remote_port, char *exten, int duration)
{
	/**** PHONE 1 */
	struct phone *c7940 = NULL;
	c7940 = phone_new("SEP001AA289341A", 0, 1, local_ip, remote_ip, SCCP_DEVICE_7940, 0, 0, 0, exten, duration);
	c7940->session = session_new(remote_ip, remote_port);

	if (c7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	pthread_t thread_register, thread_caller;
	pthread_create(&thread_register, NULL, phone_handler, c7940);
	phone_register(c7940);

	/* dial! */
	strcpy(c7940->exten, exten);
	pthread_create(&thread_caller, NULL, caller, c7940);

	/**** PHONE 2 */
	struct phone *d7940 = NULL;
	d7940 = phone_new("SEP001AA289341B", 0, 1, local_ip, remote_ip, SCCP_DEVICE_7940, 0, 0, 0, exten, duration);
	d7940->session = session_new(remote_ip, remote_port);

	if (d7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	phone_register(d7940);
	pthread_t thread_register2, thread_caller2;
	pthread_create(&thread_register2, NULL, phone_handler, d7940);

	/* dial! */
	strcpy(d7940->exten, exten);
	pthread_create(&thread_caller2, NULL, caller, d7940);

	pthread_join(thread_caller2, NULL);
	pthread_join(thread_caller, NULL);
	return 0;
}

int sccpp_test_connect(char *local_ip, char *remote_ip, char *remote_port, char *exten, int duration)
{
	struct phone *c7940 = NULL;
	pthread_t thread;

	c7940 = phone_new("SEP64AE0C5F9718", 0, 1, local_ip, remote_ip, 369, 0, 0, 0, exten, duration);

	//c7940 = phone_new("SEP00163E0BBA63", 0, 1, local_ip, remote_ip, 369, 0, 0, 0, new);
	c7940->session = session_new(remote_ip, remote_port);

	if (c7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	/* start thread msg handler */
	pthread_create(&thread, NULL, phone_handler, c7940);

	/* fire registration */
	phone_register(c7940);

	/* wait for the registration to be established */
	sleep(2);

	transmit_offhook_message(c7940);
	sleep(2);
	do_dial_extension(c7940, exten);
/*
	sleep(3);
	transmit_onhook_message(c7940);
*/
	pthread_join(thread, NULL);
	return 0;
}

int sccpp_test_load(char *local_ip, char *remote_ip, char *remote_port, int thread, char *exten, int duration)
{
	struct phone *c7940 = NULL;

	char name[16];
	uint32_t userId = 0;
	uint32_t instance = 1;

	uint32_t type = 369;
	uint32_t maxStreams = 0;
	uint32_t activeStreams = 0;
	uint8_t protoVersion = 0;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x82400);

	FILE *f = NULL;
	char *mac, *line;
	size_t linesz = 256;

	int ret = 0;
	int i = 0;

	mac = malloc(linesz + 1);

	f = fopen("./sccp.conf.simple", "r");
	if (f == NULL) {
		fprintf(stderr, "./sccp.conf.simple not found\n");
		return -1;
	}

	do {
		ret = getline(&mac, &linesz, f);

		if (ret > 0) {
			line = strchr(mac, ',');
			if (line != NULL) {
				*line = '\0';
				line++;
			}

			c7940 = phone_new(mac, userId, instance, local_ip, remote_ip, type,
					maxStreams, activeStreams, protoVersion, exten, duration);

			c7940->session = session_new(remote_ip, remote_port);

			pthread_create(&c7940->session->thread, &attr, phone_handler, c7940);
			phone_register(c7940);

			usleep(300);

			printf("%s => %s", mac, line);
		}

	} while (ret > 0 && ++i < thread);

	fclose(f);

	while(1) {sleep(1);}

	return 0;
}
