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

void print_remote_help(int port, char * ip);

void free_glob(void);

int process_client();

int authenticate(struct connection *c);

int ReadExec(ssh_channel chan, void *vptr, int maxlen);

int show_content(struct connection *c, char* command);

size_t calcDecodeLength(const char* b64input);

int base64Decode(char* b64message, unsigned char** buffer, size_t* length);

char *base64Encode(const unsigned char *input, int length);

#endif //SSHSERVER_H_

