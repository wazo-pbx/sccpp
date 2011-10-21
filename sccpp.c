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

#include <stdio.h>
#include <string.h>

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
	if (session == NULL)
		return NULL;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, port, &hints, &res);

	session->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (session->sockfd == -1) {
		free(session);
		return NULL;
	}

	ret = connect(session->sockfd, res->ai_addr, res->ai_addrlen); 
	if (ret == -1) {
		free(session);
		return NULL;
	}

	return session;
}

int handle_register_ack(struct phone *phone, struct sccp_msg *msg)
{
	printf("keepalive %d\n", letohl(msg->data.regack.keepAlive));
	printf("dateTemplate %s\n", msg->data.regack.dateTemplate);
	printf("secondaryKeepAlive %d\n", letohl(msg->data.regack.secondaryKeepAlive));

	if (letohl(msg->data.regack.keepAlive) != 33)
		return -1;

	if (!strcmp(msg->data.regack.dateTemplate, "D.M.Y"))
		return -1;

	if (letohl(msg->data.regack.secondaryKeepAlive) != 33)
		return -1;

	return 0;
}
/*
static void *thread_session(void *data)
{
	struct phone *phone = data;
	struct sccp_session *session = phone->session;
	struct sccp_msg *msg = NULL;
	int connected = 1;
	int ret = 0;

	while (connected) {

		ret = read(session.sockfd, session->inbuf, 2000);
		msg = (struct sccp_msg *)session->inbuf;

		msg = (struct sccp_msg *)session->inbuf;
		ret = handle_message(msg, session);

	}

	return NULL
}
*/

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

void usage()
{
	printf("Usage: sccpp <Ip> <Port> <SleepTime>\n");
}

int main(int argc, char *argv[])
{
	int ret = 0;
	
	if (argc < 3) {
		usage();
		return -1;
	}

	char *ip = strdup(argv[1]);
	char *port = strdup(argv[2]);

	struct phone *c7940 = NULL;
	c7940 = phone_new("SEP001AA289343B", 0, 1, 0xffffff, SCCP_DEVICE_7940, 0, 0, 0);
	c7940->session = session_new(ip, port);

	phone_register(c7940);	
/*
	pthread_t thread;
	pthread_create(&thread, NULL, thread_phone, phone);
*/
	return ret;
}
