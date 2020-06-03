#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
	if (strcmp(argv[i], "delete") == 0) {
	    if (++i == argc) {
		fprintf(stderr, "not enough arguments for delete");
		return 1;
	    }
	    if (unlink(argv[i]) != 0) {
		fprintf(stderr, "error deleting '%s': %s", argv[i], strerror(errno));
		return 1;
	    }
	}
    }
    return 0;
}
