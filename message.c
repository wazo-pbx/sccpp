#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "message.h"
#include "sccpp.h"
#include "utils.h"

struct sccp_msg *msg_alloc(size_t data_length, int message_id)
{
	struct sccp_msg *msg;

	msg = calloc(1, 12 + 4 + data_length);	
	if (msg == NULL) {
		fprintf(stdout, "Memory allocation failed\n");
		return NULL;
	}

	msg->length = htolel(4 + data_length);
	msg->id = message_id;

	return msg;
}

int transmit_message(struct sccp_msg *msg, struct sccp_session *session)
{
	ssize_t nbyte;

	memcpy(session->outbuf, msg, 12);
	memcpy(session->outbuf+12, &msg->data, letohl(msg->length));

	nbyte = write(session->sockfd, session->outbuf, letohl(msg->length)+8);	
	if (nbyte == -1) {
		fprintf(stderr, "Message transmit failed %s\n", strerror(errno));
	}

	free(msg);

	return nbyte;
}

