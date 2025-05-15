#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/tasklet_dev"
#define IOCTL_TRIGGER_TASKLET _IO('T', 1)

int main()
{
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    printf("Triggering tasklet...\n");
    if (ioctl(fd, IOCTL_TRIGGER_TASKLET, 0) < 0) {
        perror("Failed to trigger tasklet");
        close(fd);
        return -1;
    }

    printf("Tasklet triggered successfully\n");
    close(fd);
    return 0;
}