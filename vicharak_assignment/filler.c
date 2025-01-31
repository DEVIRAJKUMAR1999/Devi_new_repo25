#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DRIVER_NAME "/dev/vicharak"
#define PUSH_DATA _IOW('a', 'b', struct data*)

struct data {
    int32_t length;
    char *data;
};

int main(void) {
    int fd = open(DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return fd;
    }
    struct data d;
    d.length = 3;
    d.data = malloc(d.length);
    memcpy(d.data, "xyz", d.length);
    int ret = ioctl(fd, PUSH_DATA, &d);
    if (ret < 0) {
        perror("Failed to push data");
    } else {
        printf("Data pushed: %.*s\n", d.length, d.data);
    }
    free(d.data);
    close(fd);
    return ret;
}
