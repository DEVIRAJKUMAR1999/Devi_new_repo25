#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define DRIVER_NAME "/dev/vicharak"
#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int32_t*)

int main(void) {
    int fd = open(DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return fd;
    }
    int32_t size = 10;
    int ret = ioctl(fd, SET_SIZE_OF_QUEUE, &size);
    if (ret < 0) {
        perror("Failed to set queue size");
    } else {
        printf("Queue size set to %d\n", size);
    }
    close(fd);
    return ret;
}
