
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include "device.h"
#include "message.h"
#include "phone.h"
#include "rtp.h"
#include "utils.h"

int handle_register_ack_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	phone->keepAlive = letohl(msg->data.regack.keepAlive);
	phone->dateTemplate = strdup(msg->data.regack.dateTemplate);
	phone->secondaryKeepAlive = letohl(msg->data.regack.secondaryKeepAlive);

	phone->auth = 1;

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
	phone->rtp_send = 0;
	return 0;
}

int handle_open_receive_channel_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

int handle_start_media_transmission_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);

	phone->remote_rtp_port = letohl(msg->data.startmedia.remotePort);

	printf("phone->remote_rtp_port %d\n", phone->remote_rtp_port);
	phone->rtp_send = 1;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x82400);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_t thread_send;
	pthread_create(&thread_send, &attr, start_rtp_send, phone);

	pthread_attr_destroy(&attr);

	return 0;
}

int handle_close_receive_channel_message(struct sccp_msg *msg, struct phone *phone)
{
	fprintf(stdout, "%s\n", __func__);
	phone->rtp_recv = 0;
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

		case OPEN_RECEIVE_CHANNEL_MESSAGE:
			handle_open_receive_channel_message(msg, phone);
			transmit_open_receive_channel_ack_message(phone);
			break;

		case CLOSE_RECEIVE_CHANNEL_MESSAGE:
			handle_close_receive_channel_message(msg, phone);
			break;

		case STOP_MEDIA_TRANSMISSION_MESSAGE:
			handle_stop_media_transmission_message(msg, phone);
			break;

		case START_MEDIA_TRANSMISSION_MESSAGE:
			handle_start_media_transmission_message(msg, phone);
			//transmit_start_media_transmission_ack_message(phone);
			break;

		default:
			//fprintf(stdout, "Unknown message %x\n", msg->id);
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

	nfds = poll(fds, 1, 5000); /* millisecond */
	if (nfds == -1) { /* something wrong happend */
		fprintf(stdout, "Failed to poll socket: %s\n", strerror(errno));
		return -1;

	} else if (nfds == 0) { /* the file descriptor is not ready */
		return 0;

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

void do_dial_extension(struct phone *phone, char *exten)
{
	int exten_len = 0;
	int i;
	int digit;

	exten_len = strlen(exten);
	for (i = 0; i < exten_len; i++) {

		if (exten[i] == '*') {
			digit = 14;
		} else if (exten[i] == '#') {
			digit = 15;
		} else {
			digit = exten[i] - 48;
		}

		transmit_keypad_button_message(phone, digit);
	}
}

void *phone_handler_connect(void *data)
{
	struct phone *phone = data;
	struct sccp_msg *msg = NULL;
	int connected = 1;
	int ret = 0;
	int toggle = 1;

	time_t start, now;
	time(&start);

	while (connected) {

		ret = fetch_data(phone->session);

		if (ret > 0) {
			msg = (struct sccp_msg *)phone->session->inbuf;
			ret = handle_message(msg, phone);
		}

		if (ret == -1) {
			connected = 0;
		}

		time(&now);

		if (now > start + 5) {
			transmit_keep_alive_message(phone);
			time(&start);
		}

		if (phone->auth && toggle) {

			transmit_offhook_message(phone);

			usleep(500);
			do_dial_extension(phone, phone->exten);

			toggle = 0;
		}
	}

	return NULL;
}

void *phone_handler(void *data)
{
	struct phone *phone = data;
	struct sccp_msg *msg = NULL;
	int connected = 1;
	int ret = 0;

	int toggle = 1;
	//int auth = 0;

	time_t start_keepalive, start_duration, start_breath;
	time_t now;

	time(&start_keepalive);
	time(&start_breath);

	while (connected) {

		ret = fetch_data(phone->session);

		if (ret > 0) {
			msg = (struct sccp_msg *)phone->session->inbuf;
			ret = handle_message(msg, phone);
		}

		if (ret == -1) {
			connected = 0;
		}

		time(&now);

		if (now > start_keepalive + 5) {
			transmit_keep_alive_message(phone);
			time(&start_keepalive);
		}

		if (toggle == 1 && now > start_breath + 5) {

			transmit_offhook_message(phone);

			usleep(500);
			do_dial_extension(phone, phone->exten);

			toggle = 0;
			time(&start_duration);
		}

		if (toggle == 0 && now > start_duration + phone->call_duration) {
			transmit_onhook_message(phone);
			toggle = 1;
			time(&start_breath);
		}
	}

	return NULL;
}

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
		int duration)
{

	struct phone *phone = NULL;
	phone = calloc(1, sizeof(struct phone));

	memcpy(phone->name, name, sizeof(phone->name));
	phone->userId = userId;
	phone->instance = instance;

	phone->local_ip = strdup(local_ip);
	phone->remote_ip = strdup(remote_ip);

	phone->type = type;
	phone->maxStreams = maxStreams;
	phone->activeStreams = activeStreams;
	phone->protoVersion = protoVersion;

	phone->call_duration = duration;

	strcpy(phone->exten, exten);

	return phone;
}

int phone_register(struct phone *phone)
{
	transmit_register_message(phone);

	return 0;
}
