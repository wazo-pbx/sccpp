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

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "device.h"
#include "message.h"
#include "sccpp.h"
#include "utils.h"

struct phone {

	struct sccp_session *session;

	char name[16];
	uint32_t userId;
	uint32_t instance;
	uint32_t ip;
	uint32_t type;
	uint32_t maxStreams;
	uint32_t activeStreams;
	uint8_t protoVersion;

	uint32_t keepAlive;
	char *dateTemplate;
	uint32_t secondaryKeepAlive;
};

int sccp_register_message(struct phone *phone)
{
	struct sccp_msg *msg = NULL;
	int ret = 0;

	msg = msg_alloc(sizeof(struct register_message), REGISTER_MESSAGE);
	if (msg == NULL)
		return -1;

	memcpy(msg->data.reg.name, phone->name, sizeof(msg->data.reg.name));
	msg->data.reg.userId = htolel(phone->userId);
	msg->data.reg.instance = htolel(phone->instance);
	msg->data.reg.ip = letohl(phone->ip);
	msg->data.reg.type = htolel(phone->type);
	msg->data.reg.maxStreams = htolel(phone->maxStreams);
	msg->data.reg.protoVersion = htolel(phone->protoVersion);

	ret = transmit_message(msg, phone->session);	
	if (ret == -1)
		return -1;

	return 0;
}

struct phone *phone_new(char name[16],
		uint32_t userId,
		uint32_t instance,
		uint32_t ip,
		uint32_t type,
		uint32_t maxStreams,
		uint32_t activeStreams,
		uint8_t protoVersion)
{

	struct phone *phone = NULL;
	phone = calloc(1, sizeof(struct phone));

	memcpy(phone->name, name, sizeof(phone->name));
	phone->userId = userId;
	phone->instance = instance;
	phone->ip = ip;
	phone->type = type;
	phone->maxStreams = maxStreams;
	phone->activeStreams = activeStreams;
	phone->protoVersion = protoVersion;

	return phone;
}

struct sccp_session *session_new(char *ip, char *port)
{
	struct addrinfo hints, *res = NULL;
	struct sccp_session *session = NULL;
	int ret = 0;

	session = calloc(1, sizeof(struct sccp_session));
	if (session == NULL) {
		fprintf(stdout, "calloc() failed\n");
		return NULL;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, port, &hints, &res);

	printf("connection to %s:%s...\n", ip, port);

	session->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (session->sockfd == -1) {
		fprintf(stdout, "socket() failed\n");
		free(session);
		return NULL;
	}

	ret = connect(session->sockfd, res->ai_addr, res->ai_addrlen); 
	if (ret == -1) {
		fprintf(stdout, "connect() failed: %s\n", strerror(errno));
		free(session);
		return NULL;
	}

	return session;
}

int transmit_keypad_button_message(struct phone *phone, uint32_t dtmf)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(sizeof(struct keypad_button_message), KEYPAD_BUTTON_MESSAGE);
	if (msg == NULL)
		return -1;

	msg->data.keypad.button = htolel(dtmf);
	msg->data.keypad.instance = htolel(1);
	msg->data.keypad.callId = htolel(0);

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_onhook_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(sizeof(struct onhook_message), ONHOOK_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_offhook_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(sizeof(struct offhook_message), OFFHOOK_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_time_date_req_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(0, TIME_DATE_REQ_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_register_available_lines_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(0, REGISTER_AVAILABLE_LINES_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_line_status_req_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(sizeof(struct line_status_req_message), LINE_STATUS_REQ_MESSAGE);
	if (msg == NULL)
		return -1;

	msg->data.line.lineNumber = htolel(1);

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_softkey_set_req_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(0, SOFTKEY_SET_REQ_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_softkey_template_req_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(0, SOFTKEY_TEMPLATE_REQ_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_button_template_req_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(0, BUTTON_TEMPLATE_REQ_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int handle_register_ack_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	phone->keepAlive = letohl(msg->data.regack.keepAlive);
	phone->dateTemplate = strdup(msg->data.regack.dateTemplate);
	phone->secondaryKeepAlive = letohl(msg->data.regack.secondaryKeepAlive);

	return 0;
}

int handle_forward_status_res_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_date_time_res_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_stop_media_transmission_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_close_receive_channel_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_call_state_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_start_tone_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_set_lamp_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_line_status_res_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_select_soft_keys_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_softkey_set_res_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_softkey_template_res_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_button_template_res_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_capabilities_req_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	int ret = 0;

	msg = msg_alloc(sizeof(struct capabilities_res_message), CAPABILITIES_RES_MESSAGE);
	if (msg == NULL)
		return -1;

	msg->data.caps.count = htolel(8);

	/* Wideband 256k */
	msg->data.caps.caps[0].codec = htolel(25);
	msg->data.caps.caps[0].frames = htolel(120);

	/* G.711 u-law 64k */
	msg->data.caps.caps[1].codec = htolel(4);
	msg->data.caps.caps[1].frames = htolel(40);

	/* G7.11 A-law 64k */
	msg->data.caps.caps[2].codec = htolel(2);
	msg->data.caps.caps[2].frames = htolel(40);

	/* G.729 Annex B */
	msg->data.caps.caps[3].codec = htolel(15);
	msg->data.caps.caps[3].frames = htolel(60);

	/* G.729 Annex A+Annex B */
	msg->data.caps.caps[4].codec = htolel(16);
	msg->data.caps.caps[4].frames = htolel(60);

	/* G.729 */
	msg->data.caps.caps[5].codec = htolel(11);
	msg->data.caps.caps[5].frames = htolel(60);

	/* G.729 Annex A */
	msg->data.caps.caps[6].codec = htolel(12);
	msg->data.caps.caps[6].frames = htolel(60);

	/* RFC2833_DynPayload */
	msg->data.caps.caps[7].codec = htolel(257);
	msg->data.caps.caps[7].frames = htolel(4);

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int phone_register(struct phone *phone)
{
	struct sccp_msg *msg = NULL;
	int ret = 0;

	msg = msg_alloc(sizeof(struct register_message), REGISTER_MESSAGE);
	if (msg == NULL)
		return -1;

	memcpy(msg->data.reg.name, phone->name, sizeof(msg->data.reg.name));
	msg->data.reg.userId = htolel(phone->userId);  
	msg->data.reg.instance = htolel(phone->instance);
	msg->data.reg.ip = letohl(phone->ip);
	msg->data.reg.type = htolel(phone->type);
	msg->data.reg.maxStreams = htolel(phone->maxStreams);
	msg->data.reg.protoVersion = htolel(phone->protoVersion);

	ret = transmit_message(msg, phone->session);	
	if (ret == -1)
		return -1;

	return 0;
}

static int handle_message(struct sccp_msg *msg, struct phone *phone)
{
	int ret = 0;

	switch (msg->id) {

		case REGISTER_ACK_MESSAGE:
			handle_register_ack_message(msg, phone);
			break;

		case CAPABILITIES_REQ_MESSAGE:
			handle_capabilities_req_message(msg, phone);
			transmit_button_template_req_message(phone);
			break;

		case BUTTON_TEMPLATE_RES_MESSAGE:
			handle_button_template_res_message(msg, phone);
			transmit_softkey_template_req_message(phone);
			break;

		case SOFTKEY_TEMPLATE_RES_MESSAGE:
			handle_softkey_template_res_message(msg, phone);
			transmit_softkey_set_req_message(phone);
			break;

		case SOFTKEY_SET_RES_MESSAGE:
			handle_softkey_set_res_message(msg, phone);
			transmit_line_status_req_message(phone);
			break;

		case SELECT_SOFT_KEYS_MESSAGE:
			handle_select_soft_keys_message(msg, phone);
			break;

		case LINE_STATUS_RES_MESSAGE:
			handle_line_status_res_message(msg, phone);
			break;

		case FORWARD_STATUS_RES_MESSAGE:
			handle_forward_status_res_message(msg, phone);
			transmit_register_available_lines_message(phone);
			transmit_time_date_req_message(phone);
			break;

		case DATE_TIME_RES_MESSAGE:
			handle_date_time_res_message(msg, phone);
			break;

		case SET_LAMP_MESSAGE:
			handle_set_lamp_message(msg, phone);
			break;

		case START_TONE_MESSAGE:
			handle_start_tone_message(msg, phone);
			break;

		case CALL_STATE_MESSAGE:
			handle_call_state_message(msg, phone);
			break;

		case CLOSE_RECEIVE_CHANNEL_MESSAGE:
			handle_close_receive_channel_message(msg, phone);
			break;

		case STOP_MEDIA_TRANSMISSION_MESSAGE:
			handle_stop_media_transmission_message(msg, phone);
			break;

		default:
			fprintf(stdout, "Unknown message %x\n", msg->id);
			break;
	}

	return ret;
}

static int fetch_data(struct sccp_session *session)
{
        struct pollfd fds[1] = {{0}};
        int nfds = 0;
        ssize_t nbyte = 0;
        int msg_len = 0;

        fds[0].fd = session->sockfd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;

        nfds = poll(fds, 1, -1); /* millisecond */
        if (nfds == -1) { /* something wrong happend */
                fprintf(stdout, "Failed to poll socket: %s\n", strerror(errno));
                return -1;

        } else if (nfds == 0) { /* the file descriptor is not ready */
                fprintf(stdout, "Device has timed out\n");
                return -1;

        } else if (fds[0].revents & POLLERR || fds[0].revents & POLLHUP) {
                fprintf(stdout, "Device has closed the connection\n");
                return -1;

        } else if (fds[0].revents & POLLIN || fds[0].revents & POLLPRI) {

                /* fetch the field that contain the packet length */
                nbyte = read(session->sockfd, session->inbuf, 4);
                if (nbyte < 0) { /* something wrong happend */
                        fprintf(stdout, "Failed to read socket: %s\n", strerror(errno));
                        return -1;

                } else if (nbyte == 0) { /* EOF */
                        fprintf(stdout, "Device has closed the connection\n");
                        return -1;

                } else if (nbyte < 4) {
                        fprintf(stdout, "Client sent less data than expected. Expected at least 4 bytes but got %d\n", nbyte);
                        return -1;
		}

                msg_len = letohl(*((int *)session->inbuf));
                if (msg_len > SCCP_MAX_PACKET_SZ || msg_len < 0) {
                        fprintf(stdout, "Packet length is out of bounds: 0 > %d > %d\n", msg_len, SCCP_MAX_PACKET_SZ);
                        return -1;
                }

                /* bypass the length field and fetch the payload */
                nbyte = read(session->sockfd, session->inbuf+4, msg_len+4);
                if (nbyte < 0) {
			fprintf(stdout, "Failed to read socket: %s\n", strerror(errno));
                        return -1;

                } else if (nbyte == 0) { /* EOF */
                        fprintf(stdout, "Device has closed the connection\n");
                        return -1;
                }

                return nbyte;
        }

        return -1;
}

void *thread_phone(void *data)
{
	struct phone *phone = data;
	struct sccp_msg *msg = NULL;
	int connected = 1;
	int ret = 0;

	while (connected) {

		ret = fetch_data(phone->session);
		if (ret > 0) {
			msg = (struct sccp_msg *)phone->session->inbuf;
			ret = handle_message(msg, phone);
		}

		if (ret == -1) {
			connected = 0;
		}
	}

	return NULL;
}

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
	c7940 = phone_new("SEP001AA289341A", 0, 1, 0xffffff, SCCP_DEVICE_7940, 0, 0, 0);
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
	d7940 = phone_new("SEP001AA289341B", 0, 1, 0xffffff, SCCP_DEVICE_7940, 0, 0, 0);
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
	c7940 = phone_new("SEP001AA289341A", 0, 1, 0xffffff, SCCP_DEVICE_7940, 0, 0, 0);
	c7940->session = session_new(ip, port);

	if (c7940->session == NULL) {
		fprintf(stdout, "can't create a new session\n");
		return -1;
	}

	phone_register(c7940);

	return 0;
}

extern char *optarg;
extern int optind, opterr, optopt;
#define SCCP_PORT "2000"

int main(int argc, char *argv[])
{
	int ret = 0;

	char ip[16] = "127.0.0.1";
	int opt;
	int mode_stress = 0;
	int mode_connect = 0;

	while ((opt = getopt(argc, argv, "sci:")) != -1) {
		switch (opt) {
		case 's':
			mode_stress = 1;
			break;
		case 'c':
			mode_connect = 1;
			break;
		case 'i':
			strncpy(ip, optarg, 16);
			break;
		}
	}

	if (!(mode_stress ^ mode_connect)) {
		fprintf(stderr, "Usage %s [-s | -c] {-i ip (default: 127.0.0.1)}\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (mode_stress) { /* Experimental */
		ret = sccpp_test_stress(ip, SCCP_PORT);
	}

	if (mode_connect) {
		ret = sccpp_test_connect(ip, SCCP_PORT);
	}

	return ret;
}
