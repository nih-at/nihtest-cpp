#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
	FILE *in;
	int ret;
	char buf[1024];
	if (strcmp(argv[i], "-") == 0) {
	    in = stdin;
	} else {
	    in = fopen(argv[i], "r");
	}
	if (in == NULL) {
	    fprintf(stderr, "cannot open '%s': %s", argv[i], strerror(errno));
	    continue;
	}
	while ((ret=fread(buf, 1, sizeof(buf), in)) > 0) {
	    printf("%.*s", ret, buf);
	}
	if (ret == -1) {
	    fprintf(stderr, "read error from '%s': %s", argv[i], strerror(errno));
	}
	if (in != stdin) {
	    fclose(in);
	}
    }
    return 0;
}