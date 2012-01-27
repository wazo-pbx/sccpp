
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

	char ip[16] = "127.0.0.1";	/* Default SCCP server IP */
	int opt;
	int mode_stress = 0;
	int mode_connect = 0;

	while ((opt = getopt(argc, argv, "sci:")) != -1) {
		switch (opt) {
		case 's':
			mode_stress = 1;
			break;
		case 'c':
			mode_connect = 1;
			break;
		case 'i':
			strncpy(ip, optarg, 16);
			break;
		}
	}

	if (!(mode_stress ^ mode_connect)) {
		fprintf(stderr, "Usage %s [-s | -c] {-i ip (default: 127.0.0.1)}\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (mode_stress) { /* Experimental */
		ret = sccpp_test_stress(ip, SCCP_PORT);
	}

	if (mode_connect) {
		ret = sccpp_test_connect(ip, SCCP_PORT);
	}

	return ret;
}
