#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_BASE_PHYS 0xFE200000
#define GPIO_LEN       0xB4

#define GPFSEL1_OFFSET 0x04
#define GPSET0_OFFSET  0x1C
#define GPCLR0_OFFSET  0x28

#define GPIO_PIN 18

int main() {
    int fd;
    volatile uint32_t *gpio;

    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    // Map GPIO physical to virtual address
    gpio = mmap(NULL, GPIO_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_PHYS);
    if (gpio == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    // Set GPIO18 as output
    uint32_t sel = gpio[GPFSEL1_OFFSET / 4];
    sel &= ~(0x7 << 24);      // Clear bits 26:24
    sel |= (0x1 << 24);       // Set to output
    gpio[GPFSEL1_OFFSET / 4] = sel;

    // Blink LED
    for (int i = 0; i < 10; i++) {
        // Set GPIO18 high
        gpio[GPSET0_OFFSET / 4] = (1 << GPIO_PIN);
        usleep(500000); // 500 ms

        // Set GPIO18 low
        gpio[GPCLR0_OFFSET / 4] = (1 << GPIO_PIN);
        usleep(500000); // 500 ms
    }

    // Cleanup
    munmap((void *)gpio, GPIO_LEN);
    close(fd);

    return 0;
}
