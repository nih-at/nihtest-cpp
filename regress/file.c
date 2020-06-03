#ifdef _MSC_VER
// We're okay with using the incredibly insecure functions fopen() and sterror().
// (Yes, it's not thread save, and we're not multi-threaded.)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* supported commands
 *
 * delete name - deletes "name"
 * new name [content] - creates "name" and writes "content" to it (if defined)
 */

int main(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
	if (strcmp(argv[i], "delete") == 0) {
	    if (++i == argc) {
		fprintf(stderr, "not enough arguments for delete");
		return 1;
	    }
	    if (remove(argv[i]) != 0) {
		fprintf(stderr, "error deleting '%s': %s", argv[i], strerror(errno));
		return 1;
	    }
	}
	else if (strcmp(argv[i], "new") == 0) {
	    FILE *out;
	    if (++i == argc) {
		fprintf(stderr, "not enough arguments for new");
		return 1;
	    }
	    if ((out=fopen(argv[i], "w")) == NULL) {
		fprintf(stderr, "error creating '%s': %s", argv[i], strerror(errno));
		return 1;
	    }
	    if (++i < argc) {
		if (fprintf(out, "%s", argv[i]) < 0) {
		    fprintf(stderr, "error writing to '%s': %s", argv[i], strerror(errno));
		    return 1;
		}
	    }
	    if (fclose(out) < 0) {
		fprintf(stderr, "error closing '%s': %s", argv[i], strerror(errno));
		return 1;
	    }
	}
    }
    return 0;
}
