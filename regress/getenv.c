#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
	char *ret;
	ret = getenv(argv[i]);
	printf("%s=%s\n", argv[i], ret ? ret : "<NULL>");
    }
    return 0;
}
