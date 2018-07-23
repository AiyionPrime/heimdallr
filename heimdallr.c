#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "github.h"
#include "sshserver.h"
#include <unistd.h>
#include <getopt.h>
#include <string.h>


void synopsys(char * cmd)
{
        printf("Usage: %s [-u username|-s username|-p port|-h|-V]\n", cmd);
}

void help(void)
{
	char* cmd = "heimdallr";
	synopsys(cmd);
	printf("  %s -u <name> to look up known usernames\n", cmd);
	printf("  %s -s <name> to search for users interactively\n", cmd);
	printf("  %s -p <port> to open a ssh server to scrape keys locally\n", cmd);
	printf("  %s -h to open this help\n", cmd);
	printf("  %s -V to show the version\n", cmd);
	exit(0);
}

int main(int argc, char *argv[]){
	int port = -1;
	int conf_available = ensure_config_dir();
	if (!conf_available){
		printf("Info: Could not find config directory, crating one under '~/.config/heimdallr'.\n");
		conf_available = ensure_config_dir();
		if (!conf_available){
			printf("Error: Could not create config directory under '~/.config/heimdallr'.\n");
			return EXIT_FAILURE;
		}
	}
	const char *key_path = getpath("private.pem");
	if( !(access(( key_path), F_OK ) != -1 )) {
		generate_key();
	}

        int runmode = -1;
        int option = 0;
        char *username;
        while ((option = getopt(argc, argv,"hVs:u:p:")) != -1) {
                switch (option)
                {
			case 'p':
				port = valid_port(optarg);
				runmode = option;
				break;
                        case 's':
                        case 'u':
                                username = strdup(optarg);
                        case 'h':
                        case 'V':
                                runmode = option;
                                break;
                        default:
                                printf("Error: Unknown parameter.\nTake a look into 'heimdallr -h':\n");
                                synopsys("heimdallr");
                                exit(EXIT_FAILURE);
                }
        }
        switch (runmode) {
		case 'p':
			if (0 > port) {
				printf("Error: The given port is invalid. Valid ones are between %d and %d.\n", MINPORT, MAXPORT);
				return EXIT_FAILURE;
			}
			run_ssh_server(port);
			break;
                case 's':
                        find_user(username);
                        break;
                case 'u':
                        get_keys(username);
                        break;
                case 'h':
                        help();
                        break;
                case 'V':
                        printf("Version: %s\n", VERSION);
                        break;
                default:
                        synopsys("heimdallr");
        }
        free(username);
        return EXIT_SUCCESS;
}
