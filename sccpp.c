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

#include "message.h"
#include "sccpp.h"
#include "utils.h"

int sp_connect(char *ip, char *port)
{
	struct addrinfo hints, *res;
	int sockfd;
	int ret; 

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, port, &hints, &res);

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1)
		return -1;

	ret = connect(sockfd, res->ai_addr, res->ai_addrlen); 
	if (ret == -1)
		return -1;

	return sockfd;
}

int create_device_7940(char *ip, char *port)
{
	int sockfd = 0;
	struct sccp_msg *msg = NULL;
	struct sccp_session session;
	int ret = 0;

	session.sockfd = sp_connect(ip, port);

	msg = msg_alloc(sizeof(struct register_message), REGISTER_MESSAGE);
	if (msg == NULL)
		return -1;

	memcpy(msg->data.reg.name, "SEP001AA289343B", sizeof(msg->data.reg.name));
	msg->data.reg.userId = htolel(0);  
	msg->data.reg.instance = htolel(0);
	msg->data.reg.ip = letohl(0x0a610864);
	msg->data.reg.type = htolel(115);
	msg->data.reg.maxStreams = htolel(5);
	msg->data.reg.protoVersion = htolel(0);

	ret = transmit_message(msg, &session);	
	if (ret == -1)
		return -1;

	sleep(1);

	char buf[2000];
	ret = read(session.sockfd, buf, 2000);
	printf("got %d\n", ret);

	msg = (struct sccp_msg *)buf;

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

void usage()
{
	printf("Usage: sccpp <Ip> <Port> <SleepTime>\n");
}

int main(int argc, char *argv[])
{
	int ret = 0;


	if (argc < 4) {
		usage();
		return -1;
	}
	char *ip = strdup(argv[1]);
	char *port = strdup(argv[2]);

	ret = create_device_7940(ip, port);

//	ret = sp_connect(argv[1], argv[2]);

	sleep(atoi(argv[3]));

	return ret;
}
