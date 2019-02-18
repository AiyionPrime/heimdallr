#include "keys_fileops.h"

/*
 * Function: import_pubkey_comment
 *
 * Import a comment of a public key from the given filename
 *
 * filename: the path to the public key
 *
 * returns: a pointer to the allocated comment
 * the caller needs to free this
 */

char *import_pubkey_comment(const char *filename)
{
	char *res=NULL;
	char *key_buf;
	int rc;
	struct stat sb;
	FILE *file;
	off_t size;

	if (filename == NULL || *filename == '\0') {
		return NULL;
	}

	file = fopen(filename, "rb");
	if (file == NULL) {
		return NULL;
	}
	rc = fstat(fileno(file), &sb);
	if (rc < 0) {
		fclose(file);
		switch (errno) {
			case ENOENT:
			case EACCES:
				return NULL;
		}
		return NULL;
	}

	key_buf = malloc(sb.st_size + 1);
	if (key_buf == NULL) {
		fclose(file);
		return NULL;
	}

	size = fread(key_buf, 1, sb.st_size, file);
	fclose(file);

	if (size != sb.st_size) {
		free(key_buf);
		return NULL;
	}
	key_buf[size] = '\0';

	res = read_comment_oneline(key_buf);

	free(key_buf);

	return res;
}

