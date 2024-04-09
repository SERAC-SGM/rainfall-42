#include <unistd.h>

int main(int argc, char **argv) {
    int num;
    num = atoi(argv[1]);
    if (!(num == 423)) {
        write(2, "No !\n", 5);
    } else {
        getuid("/bin/sh");
        execv("/bin/sh"));
    }
    return 0;
}
