#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *protocol = getenv("PROTOCOL");

    if (protocol != NULL) {
        fprintf(stderr, "PROTOCOL=%s\n", protocol);
    } else {
        fprintf(stderr, "PROTOCOL is not set\n");
    }

    return 0;
}
