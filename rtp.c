
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <ortp/ortp.h>

#include <ortp/ortp.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#include "phone.h"
#include "rtp.h"

void rtp_init()
{
	ortp_init();
	ortp_scheduler_init();
	ortp_set_log_level_mask(ORTP_DEBUG | ORTP_MESSAGE | ORTP_WARNING | ORTP_ERROR);
}

void *start_rtp_recv(void *data)
{
	RtpSession *session = NULL;
	struct phone *phone = NULL;
	int ret = 0;

	phone = (struct phone *)data;
	printf("phone->headset %d\n", phone->headset);

	session = rtp_session_new(RTP_SESSION_RECVONLY);

	rtp_session_set_scheduling_mode(session, 1);
	rtp_session_set_blocking_mode(session, 1);
	ret = rtp_session_set_local_addr(session, phone->local_ip, phone->local_rtp_port);
	printf("ret %d::%d\n", ret, __LINE__);
	rtp_session_set_connected_mode(session, 1);
	rtp_session_set_symmetric_rtp(session, 1);
	rtp_session_enable_adaptive_jitter_compensation(session, 1);
	rtp_session_set_jitter_compensation(session, 1);
	ret = rtp_session_set_payload_type(session, 0);
	printf("ret %d::%d\n", ret, __LINE__);
	ret = rtp_session_signal_connect(session, "ssrc_changed", (RtpCallback)rtp_session_reset, 0);
	printf("ret %d::%d\n", ret, __LINE__);

	printf("start rtp recv (%d)\n", phone->local_rtp_port);

	int err = 0;
	int have_more = 0;
	int stream_received = 0;
	unsigned char buffer[160];
	uint32_t ts = 0;

	int sound_fd;

	if (phone->headset) {
		/* XXX use asound instead... */
		sound_fd = open("/dev/audio", O_WRONLY);
	}

	if (sound_fd == -1) {
		printf("error can't open /dev/audio\n");
		return NULL;
	}

	while (phone->rtp_recv) {

		have_more = 1;

		while (have_more) {

			err = rtp_session_recv_with_ts(session, buffer, 160, ts, &have_more);
			if (err > 0)
				stream_received = 1;

			if (stream_received && err > 0) {
				if (phone->headset)
					ret = write(sound_fd, buffer, err);
				if (ret == -1) {
					printf("error writing the sound device!\n");
				}
			}
		}
		ts+=160;
	}

	close(sound_fd);

	rtp_session_destroy(session);
	ortp_global_stats_display();


	printf("stop rtp recv\n");

    return NULL;
}

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames;
int size;

void init_mic()
{
	int rc;
	unsigned int val;
	int dir;

	/* Open PCM device for recording (capture). */
	rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);

	if (rc < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		exit(-1);
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_MU_LAW);

	/* channels (mono) */
	snd_pcm_hw_params_set_channels(handle, params, 1);

	/* bits/second sampling rate */
	val = 8000;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

	/* Set period size to 32 frames. */
	frames = 32;
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		exit(-1);
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);

	size = frames;

	/* We want to loop for 5 seconds */
	snd_pcm_hw_params_get_period_time(params, &val, &dir);
}

int read_mic(char *buffer)
{
	int rc;
	rc = snd_pcm_readi(handle, buffer, frames);

	if (rc == -EPIPE) {

		/* EPIPE means overrun */
		fprintf(stderr, "overrun occurred\n");
		snd_pcm_prepare(handle);

	} else if (rc < 0) {
		fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
	} else if (rc != (int)frames) {
		fprintf(stderr, "short read, read %d frames\n", rc);
	}

	return rc;
}

void close_mic()
{
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
}

void *start_rtp_send(void *data)
{
	RtpSession *session = NULL;
	unsigned char buffer[160];
	int i = 0;
	FILE *infile = NULL;
	uint32_t user_ts = 0;
	int ret = 0;

	struct phone *phone;
	phone = (struct phone *)data;

	printf("start rtp send (%d)\n", phone->remote_rtp_port);

	session = rtp_session_new(RTP_SESSION_SENDONLY);

	rtp_session_set_scheduling_mode(session, 1);
	rtp_session_set_blocking_mode(session, 1);
	rtp_session_set_connected_mode(session, TRUE);
	ret = rtp_session_set_remote_addr(session, phone->remote_ip, phone->remote_rtp_port);
	printf("ret %d::%d\n", ret, __LINE__);
	if (ret == -1) {
		goto end;
	}

	ret = rtp_session_set_payload_type(session, 0);
	printf("ret %d::%d\n", ret, __LINE__);
	if (ret == -1) {
		goto end;
	}

	printf("phone->headset %d\n", phone->headset);

	if (phone->headset)
		init_mic();
	else
		infile=fopen("./music.raw","r");

	if (infile == NULL)
		goto end;

	while (phone->rtp_send) {

		if (phone->headset)
			i = read_mic((char *)buffer);
		else
			i = fread(buffer, 1, 160, infile);

		ret = rtp_session_send_with_ts(session, buffer, i, user_ts);
		if (!(ret > 0))
			printf("ret %d\n", ret);

		user_ts+=i;
	}

	if (phone->headset)
		close_mic();
	else
		fclose(infile);
end:
	rtp_session_destroy(session);
	ortp_global_stats_display();

	return NULL;
}
