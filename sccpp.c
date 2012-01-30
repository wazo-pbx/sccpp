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

int sccpp_test_stress(char *ip, char *port, char *exten)
{
	/**** PHONE 1 */
	struct phone *c7940 = NULL;
	c7940 = phone_new("SEP001AA289341A", 0, 1, 0xffffff, SCCP_DEVICE_7940, 0, 0, 0);
	c7940->session = session_new(ip, port);

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
	d7940 = phone_new("SEP001AA289341B", 0, 1, 0xffffff, SCCP_DEVICE_7940, 0, 0, 0);
	d7940->session = session_new(ip, port);

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

int sccpp_test_connect(char *ip, char *port)
{
	struct phone *c7940 = NULL;
	pthread_t thread;

	c7940 = phone_new("SEP001AA289341A", 0, 1, 0xffffff, 8, 0, 0, 0);
	c7940->session = session_new(ip, port);

	if (c7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	pthread_create(&thread, NULL, phone_handler, c7940);
	phone_register(c7940);

	sleep(1);
	transmit_offhook_message(c7940);
	sleep(1);
	transmit_onhook_message(c7940);

	pthread_join(thread, NULL);
	return 0;
}
