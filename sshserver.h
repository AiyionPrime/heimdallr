#ifndef SSHSERVER_H_
#define SSHSERVER_H_

#include <libssh/libssh.h>

struct connection {
	ssh_session session;
	ssh_message message;
	char client_ip[96];
	char con_time[96];
	char *user;
	char *pass;
};

int run_ssh_server(int port);

void free_glob(void);

int process_client();

int authenticate(struct connection *c);

int ReadExec(ssh_channel chan, void *vptr, int maxlen);

int show_content(struct connection *c, char* command);

#endif //SSHSERVER_H_

