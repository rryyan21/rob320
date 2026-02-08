#include "util.h"

int ctrl_c_pressed = 0;

int main(int argc, char* argv[]) {
    char* public_ip = get_public_ip();
    if (public_ip == NULL) {
        public_ip = strdup("127.0.0.1");
    }
    printf("%s", public_ip);
    free(public_ip);
}