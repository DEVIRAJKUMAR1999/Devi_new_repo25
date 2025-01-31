#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>

#define DRIVER_NAME "/dev/vicharak"
#define POP_DATA _IOR('a', 'c', struct data*)

struct data {
    int length;
    char *data;
};

int main(void) {
    int fd = open(DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return fd;
    }
    struct data d;
    d.data = malloc(100); // Allocate sufficient space
    int ret = ioctl(fd, POP_DATA, &d);
    if (ret < 0) {
        perror("Failed to pop data");
    } else {
        printf("Data popped: %.*s\n", d.length, d.data);
    }
    free(d.data);
    close(fd);
    return ret;
}
