/* 
 * Nicolas Bouliane <nbouliane@avencall.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 */
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* Proof of concept of a tool 
 * used by Asterisk testsuite
 */
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

	ret = sp_connect(argv[1], argv[2]);
	sleep(atoi(argv[3]));

	return ret;
}
