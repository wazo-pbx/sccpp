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

	free(exten);
}

int sccpp_scen_connect_exit(char *remote_ip, char *remote_port)
{
	struct sccp_session *session = NULL;
	session = session_new(remote_ip, remote_port);
	if (session == NULL) {
		fprintf(stdout, "can't connect to %s:%s\n", remote_ip, remote_port);
		exit(1);
	}

	exit(0);
}

int sccpp_scen_stress(char *local_ip, char *remote_ip, char *remote_port, char *exten, int duration)
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

int sccpp_scen_softphone(char *local_ip, char *remote_ip, char *remote_port, char *exten, int duration, char headset, char *macaddr)
{
	struct phone *c7940 = NULL;
	pthread_t thread;

	c7940 = phone_new(macaddr, 0, 1, local_ip, remote_ip, 369, 0, 0, 0, exten, duration);
	c7940->headset = headset;

	c7940->session = session_new(remote_ip, remote_port);
	if (c7940->session == NULL) {
		fprintf(stderr, "can't create a new session\n");
		return -1;
	}

	pthread_create(&thread, NULL, phone_handler_connect, c7940);
	phone_register(c7940);

	pthread_join(thread, NULL);

	return 0;
}

int sccpp_scen_answer_call(char *local_ip, char *remote_ip, char *remote_port, int thread, int duration, char headset, char *macaddr)
{
#if 0
	struct phone *c7940 = NULL;
	pthread_t thread_answer;

	c7940 = phone_new(macaddr, 0, 1, local_ip, remote_ip, 369, 0, 0, 0, "noexten", duration);
	c7940->headset = headset;

	c7940->session = session_new(remote_ip, remote_port);
	if (c7940->session == NULL) {
		fprintf(stderr, "can't create a new session\n");
		return -1;
	}

	pthread_create(&thread_answer, NULL, phone_handler_answer, c7940);
	phone_register(c7940);

	pthread_join(thread_answer, NULL);

	return 0;
#endif

	struct phone *c7940 = NULL;

	uint32_t userId = 0;
	uint32_t instance = 1;

	uint32_t type = 369;
	uint32_t maxStreams = 0;
	uint32_t activeStreams = 0;
	uint8_t protoVersion = 0;

	FILE *f = NULL;
	char *mac, *line;
	size_t linesz = 256;

	int ret = 0;
	int i = 0;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x82400);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	mac = malloc(linesz + 1);

	f = fopen("./sccp.conf.simple", "r");
	if (f == NULL) {
		fprintf(stderr, "./sccp.conf.simple not found\n");
		goto end;
	}

	do {
		ret = getline(&mac, &linesz, f);
		if (ret > 0 && i % 2 == 0) {
			line = strchr(mac, ',');
			if (line != NULL) {
				*line = '\0';
				line++;
			}

			c7940 = phone_new(mac, userId, instance, local_ip, remote_ip, type,
					maxStreams, activeStreams, protoVersion, "noexten", duration);

			c7940->session = session_new(remote_ip, remote_port);
			if (c7940->session == NULL)
				goto end;

			pthread_create(&c7940->session->thread, &attr, phone_handler_answer, c7940);
			phone_register(c7940);

			usleep(300);
			fprintf(stdout, "%s => %s", mac, line);
		}

	} while (ret > 0 && ++i < thread);

	/* XXX catch signal */
	while(1) {sleep(1);}

end:
	pthread_attr_destroy(&attr);
	fclose(f);
	free(mac);

	return 0;
}

int sccpp_scen_mass_call(char *local_ip, char *remote_ip, char *remote_port, int thread, char *exten, int duration, char range)
{
	struct phone *c7940 = NULL;

	uint32_t userId = 0;
	uint32_t instance = 1;

	uint32_t type = 369;
	uint32_t maxStreams = 0;
	uint32_t activeStreams = 0;
	uint8_t protoVersion = 0;

	FILE *f = NULL;
	char *mac, *line;
	size_t linesz = 256;

	int ret = 0;
	int i = 0;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x82400);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	mac = malloc(linesz + 1);

	f = fopen("./sccp.conf.simple", "r");
	if (f == NULL) {
		fprintf(stderr, "./sccp.conf.simple not found\n");
		goto end;
	}

	do {
		ret = getline(&mac, &linesz, f);
		if (ret > 0 && i % 2 != 0) {
			line = strchr(mac, ',');
			if (line != NULL) {
				*line = '\0';
				line++;
			}

			c7940 = phone_new(mac, userId, instance, local_ip, remote_ip, type,
					maxStreams, activeStreams, protoVersion, exten, duration);

			if (range)
				sprintf(c7940->exten, "%d", atoi(exten)+i-1);

			c7940->session = session_new(remote_ip, remote_port);
			if (c7940->session == NULL)
				goto end;

			pthread_create(&c7940->session->thread, &attr, phone_handler, c7940);
			phone_register(c7940);

			usleep(300);
			fprintf(stdout, "%s => %s", mac, line);
		}

	} while (ret > 0 && ++i < thread);

	/* XXX catch signal */
	while(1) {sleep(1);}

end:
	pthread_attr_destroy(&attr);
	fclose(f);
	free(mac);

	return 0;
}
