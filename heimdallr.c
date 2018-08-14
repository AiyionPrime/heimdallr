#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "github.h"
#include "sshserver.h"
#include <unistd.h>
#include <getopt.h>
#include <string.h>

/*
 * Function: synopsys
 *
 * print a small synopsys on how to use this program
 *
 * cmd: the name of the command to launch this code
 *      may depend on how the programm was started in the first place, like as installed or local instance
 */

void synopsys(char * cmd)
{
	printf("Usage: %s [-u username|-s username|-p port|-h|-V]\n", cmd);
}

/*
 * Function: help
 *
 * print a small synopsys, as well as a more detailled listing of parameters, and their meaning
 */

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

/*
 * Function: main
 *
 * creates a config directory and generates a ssh privatekey into it
 * depending on the programs arguments it:
 * - starts an open ssh server on a given port and waits for remote ssh-copy-id executions in order
 *   to show the shipped ssh-keys in terminal; is meant to be stopped by ctrl+c
 * - searches a given username on github interactively and shows ssh public keys of the user
 * - fetches a given username from github and shows ssh public keys of the given oser
 * - shows a help message
 * - shows the current version
 *
 * argc: the amount of arguments given to the program
 * 
 * argv: a list of strings which were either the programs name, or arguments to run it
 *
 * returns: an int, which shows if the execution failed in general, or not.
 */

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
	// ensure private key
	char *key_path = getpath("private.pem");
	if( !(access(( key_path), F_OK ) != -1 )) {
		generate_key();
	}
	free(key_path);
	// ensure public key
	char *pub_path = getpath("public.pem");
	if( !(access(( pub_path), F_OK ) != -1 )) {
		generate_pubkey_from_private("private.pem");
	}
	free(pub_path);

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
			free(username);
			break;
		case 'u':
			get_keys(username);
			free(username);
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
	return EXIT_SUCCESS;
}
