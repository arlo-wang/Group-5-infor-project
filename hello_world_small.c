#include "system.h"
#include "io.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_stdio.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CHARLIM 256
#define OFFSET -32
#define QUITLETTER '~'
#define PWM_PERIOD 16
#define SCAN_THRESHOLD 100

alt_u8 pwm = 0;
alt_u8 led = 0;
int level = 0;
volatile int pulse_active = 0;  // Shared variable for pulse status
alt_up_accelerometer_spi_dev *acc_dev; // Global accelerometer device pointer

// Function to write to PULSE PIO
void set_pulse(int state) {
    IOWR(PULSE_BASE, 0, state);
}

void led_write(alt_u8 led_pattern) {
    IOWR(LED_BASE, 0, led_pattern);
}

void convert_read(alt_32 acc_read, int *level, alt_u8 *led) {
    acc_read += OFFSET;
    alt_u8 val = (acc_read >> 6) & 0x07;
    *led = (8 >> val) | (8 << (8 - val));
    *level = (acc_read >> 1) & 0x1F;
}

void sys_timer_isr(void *context, alt_u32 id) {
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);  // Clear timer interrupt

    // Read accelerometer value
    alt_32 x_read;
    alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read);

    // Convert accelerometer data
    convert_read(x_read, &level, &led);

    // Check if acceleration exceeds threshold for pulse activation
    if (x_read < -SCAN_THRESHOLD || x_read > SCAN_THRESHOLD) {
        pulse_active = 1;  // Activate PULSE_OUT
    } else {
        pulse_active = 0;  // Deactivate PULSE_OUT
    }

    // Apply PWM logic for LEDs
    if (pwm < abs(level)) {
        if (level < 0) {
            led_write(led << 1);
        } else {
            led_write(led >> 1);
        }
    } else {
        led_write(led);
    }

    // Update PWM counter
    pwm++;
    if (pwm >= PWM_PERIOD) {
        pwm = 0;
    }

    // Set PULSE_OUT
    IOWR(PULSE_BASE, 0, pulse_active);
}


void timer_init(void *isr) {
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0003);
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, 0x0900);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, 0x0000);

    alt_irq_register(TIMER_IRQ, NULL, isr);

    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0007);
}


void print_text(char *text)
{
	char *printMsg;
	asprintf(&printMsg, "Enter number: \n", 0x4);
	alt_putstr(printMsg);
	free(printMsg);
	memset(text, 0, 2 *CHARLIM);
}

void generate_text(char *text, int *running) {
    int idx = 0;
    char newCurr = alt_getchar(); // Get first character

    while (newCurr != EOF && newCurr != '\n') {
        if (newCurr == QUITLETTER) {
            *running = 0; // Stop execution if QUITLETTER is encountered
            break;
        }
        if (idx < CHARLIM - 1) { // Prevent buffer overflow
            text[idx++] = newCurr;
        }
        newCurr = alt_getchar(); // Get the next character
    }
    text[idx] = '\0'; // Null-terminate the string
}

void activate() {
    //alt_up_accelerometer_spi_dev *acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");

    alt_32 x_read;
    int ready = 0;

    for (int i = 0; i < 10; i++) {
        alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read);
        convert_read(x_read, &level, &led);

        if (x_read < -SCAN_THRESHOLD || x_read > SCAN_THRESHOLD) {
            ready = 1;
            break; // Stop the loop early if threshold is exceeded
        }
        usleep(10000); // Small delay to stabilize readings
    }

    if (ready) {
        IOWR(PULSE_BASE, 0, 1); // Activate Pulse
    } else {
        IOWR(PULSE_BASE, 0, 0); // Deactivate Pulse
    }
}

int main() {
	char text[2 * CHARLIM];
	int running = 1;

    print_text(text);
    acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
    timer_init(sys_timer_isr);
    while (running) {
    	generate_text(text, &running);
    	int number = strtol(text, NULL, 10);
        IOWR_32DIRECT(UART_DATA_BUFFER_BASE, 0, number);

        print_text(text);
    }
    return 0;
}


