#ifndef SCCP_PHONE_H
#define SCCP_PHONE_H

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#define SCCP_MAX_PACKET_SZ 2000
struct sccp_session {
	int sockfd;
	char inbuf[SCCP_MAX_PACKET_SZ];
	char outbuf[SCCP_MAX_PACKET_SZ];

	pthread_t thread;
};

struct phone {

	struct sccp_session *session;

	char name[16];
	uint32_t userId;
	uint32_t instance;
	uint32_t type;
	uint32_t maxStreams;
	uint32_t activeStreams;
	uint8_t protoVersion;

	uint32_t keepAlive;
	char *dateTemplate;
	uint32_t secondaryKeepAlive;

	char exten[15];
	char *remote_ip;
	char *local_ip;

	uint32_t remote_rtp_port;
	uint32_t local_rtp_port;

	uint8_t rtp_send;
	uint8_t rtp_recv;

	int call_duration;
};

struct phone *phone_new(char name[16],
                uint32_t userId,
                uint32_t instance,
                char *local_ip,
		char *remote_ip,
                uint32_t type,
                uint32_t maxStreams,
                uint32_t activeStreams,
                uint8_t protoVersion,
		char *exten,
		int duration);

int phone_register(struct phone *phone);
void *phone_handler(void *data);
struct sccp_session *session_new(char *ip, char *port);

#endif /* SCCP_PHONE_H_ */
