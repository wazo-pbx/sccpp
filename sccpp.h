#ifndef SCCPP_H
#define SCCPP_H

int sccpp_scen_stress(char *local_ip, char *remote_ip, char *remote_port, char *exten, int duration);
int sccpp_scen_softphone(char *local_ip, char *remote_ip, char *remote_port, char *exten, int duration, char headset, char *macaddr);
int sccpp_scen_mass_call(char *local_ip, char *remote_ip, char *remote_port, int thread, char *exten, int duration, char range);
int sccpp_scen_answer_call(char *local_ip, char *remote_ip, char *remote_port, int thread, int duration, char headset, char *macaddr);

#endif
