#ifdef _MSC_VER
// We're okay with using the incredibly insecure function getenv().
// (Yes, it's not thread save, and we're not multi-threaded.)
#define _CRT_SECURE_NO_WARNINGS
#endif

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
