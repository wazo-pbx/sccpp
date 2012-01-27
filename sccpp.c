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

//#include "device.h"
#include "message.h"
#include "phone.h"
#include "sccpp.h"
#include "utils.h"

void *thread_call(void *data)
{

	struct phone *phone = data;

	srand(time(NULL));

	while (1) {

		transmit_offhook_message(phone);
		transmit_keypad_button_message(phone, 1);
		transmit_keypad_button_message(phone, 0);
		transmit_keypad_button_message(phone, 7);
		sleep(2);// rand() % 3 + 1);
		transmit_onhook_message(phone);

		transmit_offhook_message(phone);
		transmit_keypad_button_message(phone, 2);
		transmit_keypad_button_message(phone, 0);
		transmit_keypad_button_message(phone, 3);
		sleep(2);// rand() % 3 + 1);
		transmit_onhook_message(phone);

	}
}

void *thread_call2(void *data)
{

	struct phone *phone = data;

	srand(time(NULL));

	while (1) {

		transmit_offhook_message(phone);
		transmit_keypad_button_message(phone, 1);
		transmit_keypad_button_message(phone, 0);
		transmit_keypad_button_message(phone, 3);
		sleep(1);// rand() % 3 + 1);
		transmit_onhook_message(phone);

		transmit_offhook_message(phone);
		transmit_keypad_button_message(phone, 1);
		transmit_keypad_button_message(phone, 0);
		transmit_keypad_button_message(phone, 2);
		sleep(1);// rand() % 3 + 1);
		transmit_onhook_message(phone);

	}
}

int sccpp_test_stress(char *ip, char *port)
{
	/* PHONE 1 */
	struct phone *c7940 = NULL;
	c7940 = phone_new("SEP001AA289341A", 0, 1, 0xffffff, 8, 0, 0, 0);
	c7940->session = session_new(ip, port);

	if (c7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	phone_register(c7940);
	pthread_t thread;
	pthread_create(&thread, NULL, thread_phone, c7940);

	/* PHONE 2 */
	struct phone *d7940 = NULL;
	d7940 = phone_new("SEP001AA289341B", 0, 1, 0xffffff, 8, 0, 0, 0);
	d7940->session = session_new(ip, port);

	if (d7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	phone_register(d7940);
	pthread_t thread2, thread3;
	pthread_create(&thread2, NULL, thread_phone, d7940);

	/* dial! */
	pthread_create(&thread3, NULL, thread_call, c7940);
	pthread_create(&thread3, NULL, thread_call2, d7940);

	pthread_join(thread, NULL);
	pthread_join(thread2, NULL);

	return 0;
}

int sccpp_test_connect(char *ip, char *port)
{
	struct phone *c7940 = NULL;
	c7940 = phone_new("SEP001AA289341A", 0, 1, 0xffffff, 8, 0, 0, 0);
	c7940->session = session_new(ip, port);

	if (c7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	phone_register(c7940);

	return 0;
}
