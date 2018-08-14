#ifndef CONFIG_HEIMDALLR_H_
#define CONFIG_HEIMDALLR_H_

#include <libssh/libssh.h>

#define MAXPORT 65535
#define MINPORT 1

int valid_port(char *p);

const char* homedir();

char* getpath(const char* filename);

int ensure_config_dir();

int generate_key();

int generate_pubkey_from_private(char * private);

int ssh_pki_export_pubkey_file(const ssh_key pubkey, const char * filename);

#endif //CONFIG_HEIMDALLR_H_

