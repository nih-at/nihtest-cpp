#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *out = stdout;
    int start = 1;
    if (argc > 1 && strcmp(argv[1], "-e") == 0) {
	start = 2;
	out = stderr;
    }
    for (int i=start; i<argc; i++) {
	fprintf(out, "%s%s", (i == start ? "" : " "), argv[i]);
    }
    fprintf(out, "\n");
    return 0;
}
