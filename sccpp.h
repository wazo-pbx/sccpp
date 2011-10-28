#ifndef SCCPP_H
#define SCCPP_H

#define SCCP_MAX_PACKET_SZ 2000
struct sccp_session {
	int sockfd;
	char inbuf[SCCP_MAX_PACKET_SZ];
	char outbuf[SCCP_MAX_PACKET_SZ];
};

#endif
