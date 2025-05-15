#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <gpiod.h>
#include <sys/mman.h>

#define GPIO_BASE_OFFSET  0x00200000  // GPIO base relative to peripheral base
#define PERIPHERAL_BASE   0xFE000000  // Peripheral base for Raspberry Pi 4
#define BLOCK_SIZE        (4 * 1024)  // Memory block size for GPIO

// GPIO Registers offsets
#define GPFSEL_OFFSET  (0x00 / 4)  // GPIO Function Select
#define GPSET_OFFSET   (0x1C / 4)  // GPIO Pin Output Set
#define GPCLR_OFFSET   (0x28 / 4)  // GPIO Pin Output Clear
#define GPLEV_OFFSET   (0x34 / 4)  // GPIO Pin Level

static volatile uint32_t *gpio_map;

// Function to initialize memory-mapped GPIO
int init_gpio_memory_map() {
    int mem_fd;

    // Open /dev/mem for direct memory access
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Map GPIO memory to user space
    gpio_map = (uint32_t *)mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, PERIPHERAL_BASE + GPIO_BASE_OFFSET);
    if (gpio_map == MAP_FAILED) {
        perror("Failed to mmap GPIO memory");
        close(mem_fd);
        return -1;
    }

    close(mem_fd);
    return 0;
}

// Function to configure a GPIO pin as output
void set_gpio_output(int pin) {
    int reg_index = pin / 10; // Each GPFSEL register configures 10 pins
    int bit_offset = (pin % 10) * 3; // Each pin takes 3 bits

    gpio_map[GPFSEL_OFFSET + reg_index] &= ~(0b111 << bit_offset); // Clear bits
    gpio_map[GPFSEL_OFFSET + reg_index] |= (0b001 << bit_offset); // Set to output
}

// Function to set a GPIO pin high
void set_gpio_high(int pin) {
    gpio_map[GPSET_OFFSET] = (1 << pin);
}

// Function to set a GPIO pin low
void set_gpio_low(int pin) {
    gpio_map[GPCLR_OFFSET] = (1 << pin);
}

// Function to read the state of a GPIO pin
int read_gpio(int pin) {
    return (gpio_map[GPLEV_OFFSET] & (1 << pin)) != 0;
}

// Function to control GPIO using gpiod API
void control_gpio_gpiod(const char *chip_name, int line_number, int value) {
    struct gpiod_chip *chip;
    struct gpiod_line *line;

    chip = gpiod_chip_open_by_name(chip_name);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return;
    }

    line = gpiod_chip_get_line(chip, line_number);
    if (!line) {
        perror("Failed to get GPIO line");
        gpiod_chip_close(chip);
        return;
    }

    if (gpiod_line_request_output(line, "example", 0) < 0) {
        perror("Failed to request line as output");
        gpiod_chip_close(chip);
        return;
    }

    if (gpiod_line_set_value(line, value) < 0) {
        perror("Failed to set line value");
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
}

int main() {
    if (init_gpio_memory_map() < 0) {
        return -1;
    }

    // Use direct memory access to toggle GPIO 17
    set_gpio_output(17);
    printf("Toggling GPIO 17 using memory-mapped I/O...\n");
    set_gpio_high(17);
    sleep(1);
    set_gpio_low(17);

    // Use gpiod API to control GPIO 27
    printf("Setting GPIO 27 high using gpiod API...\n");
    control_gpio_gpiod("gpiochip0", 27, 1);
    sleep(1);
    printf("Setting GPIO 27 low using gpiod API...\n");
    control_gpio_gpiod("gpiochip0", 27, 0);

    // Unmap GPIO memory
    munmap((void *)gpio_map, BLOCK_SIZE);

    return 0;
}