#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sccpp.h"
#include "rtp.h"

extern char *optarg;
extern int optind, opterr, optopt;
#define SCCP_PORT "2000"

void print_help()
{
	fprintf(stderr, "\nUsage:\n\n"
	"sccpp [scenario] [options]\n\n"
	"[scenarios]\n"
	" -p\t softphone\n"
	" -c\t mass call\n"
	" -a\t answer call\n"
	" -C\t connect and exit (with return code)\n"
	" -T\t register and timeout\n"
	"\n"
	"[options]\n"
	" -i\t local ip\n"
	" -o\t remote ip (default: 127.0.0.1)\n"
	" -e\t extension to call\n"
	" -t\t number of thread\n"
	" -d\t call delay\n"
	" -s\t headset\n"
	" -m\t mac address\n"
	" -r\t range call (extension+N-1)"
	"\n"
	"Example:\n\n"
	"Launch 20 virtual phones, from 10.97.8.1 to 10.97.8.9,\n"
	"that will call the queue extension 3000 for 10sec (then wait 5sec) in loop:\n\n"
	"./sccpp -c -i 10.97.8.1 -o 10.97.8.9 -e 3000 -t 20 -d 10\n\n");
}

int main(int argc, char *argv[])
{
	int scen_stress = 0;
	int scen_softphone = 0;
	int scen_mass_call = 0;
	int scen_answer_call = 0;
	int scen_connect_exit = 0;
	int scen_register_and_timeout = 0;

	char exten[15] = "";
	char macaddr[16] = "";
	char remote_ip[16] = "127.0.0.1";	/* Default SCCP server IP */
	char local_ip[16] = "";
	int duration = 5;
	int thread = 1;
	char headset = 0;
	char range = 0;

	int opt = 0;
	int ret = 0;

	while ((opt = getopt(argc, argv, "CThsrcapm:e:t:o:i:d:")) != -1) {
		switch (opt) {
		case 'h':
			print_help();
			exit(EXIT_FAILURE);
		case 'p':
			scen_softphone = 1;
			break;
		case 'c':
			scen_mass_call = 1;
			break;
		case 'a':
			scen_answer_call = 1;
			break;
		case 'C':
			scen_connect_exit = 1;
			break;
		case 'T':
			scen_register_and_timeout = 1;
			break;
		case 'e':
			strcpy(exten, optarg);
			break;
		case 'o':
			strncpy(remote_ip, optarg, 16);
			break;
		case 'i':
			strncpy(local_ip, optarg, 16);
			break;
		case 't':
			thread = atoi(optarg);
			break;
		case 'd':
			duration = atoi(optarg);
			break;
		case 'm':
			strcpy(macaddr, optarg);
			break;
		case 's':
			headset = 1;
			break;
		case 'r':
			range = 1;
			break;
		break;
		}
	}

	if (!(scen_softphone ^ scen_mass_call ^ scen_answer_call ^ scen_connect_exit ^ scen_register_and_timeout)) {
		print_help();
		exit(EXIT_FAILURE);
	}

	rtp_init();

	if (scen_stress) { /* Experimental */
		printf("exten %s\n", exten);
		ret = sccpp_scen_stress(local_ip, remote_ip, SCCP_PORT, exten, duration);
	}

	if (scen_softphone) {
		printf("scenario softphone...\n");
		ret = sccpp_scen_softphone(local_ip, remote_ip, SCCP_PORT, exten, duration, headset, macaddr);
	}

	if (scen_mass_call) {
		printf("scenario mass call...\n");
		ret = sccpp_scen_mass_call(local_ip, remote_ip, SCCP_PORT, thread, exten, duration, range);
	}

	if (scen_answer_call) {
		printf("scenario answer call...\n");
		ret = sccpp_scen_answer_call(local_ip, remote_ip, SCCP_PORT, thread, duration, headset, macaddr);
	}

	if (scen_connect_exit) {
		printf("connect and exit...\n");
		ret = sccpp_scen_connect_exit(remote_ip, SCCP_PORT);
	}

	if (scen_register_and_timeout) {
		printf("register and timeout...\n");
		ret = sccpp_scen_register_and_timeout(local_ip, remote_ip, SCCP_PORT, macaddr);
	}

	return ret;
}
