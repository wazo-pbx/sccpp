#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "message.h"
#include "sccpp.h"
#include "rtp.h"
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

	//free(msg);

	return nbyte;
}

int transmit_register_message(struct phone *phone)
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

int transmit_keep_alive_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(0, KEEP_ALIVE_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}

int transmit_open_receive_channel_ack_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	static int port = 1000;

	msg = msg_alloc(sizeof(struct open_receive_channel_ack_message), OPEN_RECEIVE_CHANNEL_ACK_MESSAGE);
	if (msg == NULL)
		return -1;

	msg->data.openreceivechannelack.status = htolel(1);
	msg->data.openreceivechannelack.ipAddr = letohl(phone->ip);
	msg->data.openreceivechannelack.port = htolel(port);
	msg->data.openreceivechannelack.passThruId = htolel(999);

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	phone->local_rtp_port = port++;
	phone->rtp_recv = 1;

	pthread_t thread_recv;
	pthread_create(&thread_recv, NULL, start_rtp_recv, phone);

	return 0;

}

int transmit_start_media_transmission_ack_message(struct phone *phone)
{
	int ret = 0;
	struct sccp_msg *msg;

	msg = msg_alloc(0, START_MEDIA_TRANSMISSION_ACK_MESSAGE);
	if (msg == NULL)
		return -1;

	ret = transmit_message(msg, phone->session);
	if (ret == -1)
		return -1;

	return 0;
}
