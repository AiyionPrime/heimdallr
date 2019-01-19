#ifndef CONFIG_HEIMDALLR_H_
#define CONFIG_HEIMDALLR_H_

#include <libssh/libssh.h>

#define MAXPORT 65535
#define MINPORT 1

int valid_port(char *p);

const char* homedir();

char* getpath(const char* filename);

int ensure_config_dir();

int ensure_githubuser_dir(char * username);

int ensure_directory(char *directory);

int generate_key();

int generate_pubkey_from_private(char * private);

char* get_githubuser_dir(char* username);

int ssh_pki_export_pubkey_file(const ssh_key pubkey, const char * filename);

int ensure_private_key_permission();

void print_permission_warning(char * file, int permission);

#endif //CONFIG_HEIMDALLR_H_

