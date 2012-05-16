#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sccpp.h"

extern char *optarg;
extern int optind, opterr, optopt;
#define SCCP_PORT "2000"

int main(int argc, char *argv[])
{
	int ret = 0;

	char exten[15] = "";
	char remote_ip[16] = "127.0.0.1";	/* Default SCCP server IP */
	char local_ip[16] = "";
	int thread = 0;
	int opt;
	int mode_connect = 0;
	int mode_load = 0;
	int mode_stress = 0;

	while ((opt = getopt(argc, argv, "lsce:t:o:i:")) != -1) {
		switch (opt) {
		case 'l':
			mode_load = 1;
			break;
		case 's':
			mode_stress = 1;
			break;
		case 'c':
			mode_connect = 1;
			break;
		case 'e':
			strcpy(exten, optarg);
			break;
		case 'i':
			strncpy(remote_ip, optarg, 16);
			break;
		case 'o':
			strncpy(local_ip, optarg, 16);
			break;
		case 't':
			thread = atoi(optarg);
			break;
		}
	}

	if (!(mode_stress ^ mode_connect ^ mode_load)) {
		fprintf(stderr, "\nSCCP profiler usage\n\n"
		"[mode]\n"
		"-s\t stress\n"
		"-c\t connect\n"
		"-l\t load\n"
		"\n[options]\n"
		"-o\t local ip\n"
		"-i\t remote ip (default: 127.0.0.1)\n"
		"-e\t extension to call\n\n");

		exit(EXIT_FAILURE);
	}

	rtp_init();

	if (mode_stress) { /* Experimental */
		printf("exten %s\n", exten);
		ret = sccpp_test_stress(local_ip, remote_ip, SCCP_PORT, exten);
	}

	if (mode_connect) {
		ret = sccpp_test_connect(local_ip, remote_ip, SCCP_PORT, exten);
	}

	if (mode_load) {
		ret = sccpp_test_load(local_ip, remote_ip, SCCP_PORT, thread, exten);
	}

	return ret;
}
