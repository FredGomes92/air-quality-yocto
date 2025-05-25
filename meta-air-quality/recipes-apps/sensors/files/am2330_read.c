#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <string.h>

#define I2C_BUS "/dev/i2c-1"
#define AM2320_ADDR 0x5C

int main() {
    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C bus");
        return 1;
    }

    // Wake up the sensor with a dummy write
    struct i2c_rdwr_ioctl_data wake;
    struct i2c_msg wake_msg;
    uint8_t dummy = 0x00;

    wake_msg.addr = AM2320_ADDR;
    wake_msg.flags = 0;
    wake_msg.len = 0;  // zero-length to just "ping"
    wake_msg.buf = &dummy;

    wake.msgs = &wake_msg;
    wake.nmsgs = 1;
    ioctl(fd, I2C_RDWR, &wake);  // ignore result; may NACK

    usleep(1000);  // wait 1ms

    // Send command: 0x03 (read), start=0x00, len=0x04
    uint8_t cmd_buf[] = {0x03, 0x00, 0x04};
    struct i2c_rdwr_ioctl_data cmd;
    struct i2c_msg cmd_msg;

    cmd_msg.addr = AM2320_ADDR;
    cmd_msg.flags = 0;
    cmd_msg.len = sizeof(cmd_buf);
    cmd_msg.buf = cmd_buf;

    cmd.msgs = &cmd_msg;
    cmd.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &cmd) < 0) {
        perror("Failed to send read command");
        close(fd);
        return 1;
    }

    usleep(2000);  // wait 2ms for sensor to respond

    // Read 8 bytes
    uint8_t buf[8];
    struct i2c_rdwr_ioctl_data read_data;
    struct i2c_msg read_msg;

    read_msg.addr = AM2320_ADDR;
    read_msg.flags = I2C_M_RD;
    read_msg.len = 8;
    read_msg.buf = buf;

    read_data.msgs = &read_msg;
    read_data.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &read_data) < 0) {
        perror("Failed to read data");
        close(fd);
        return 1;
    }

    close(fd);

    if (buf[0] != 0x03 || buf[1] != 0x04) {
        fprintf(stderr, "Unexpected response: %02x %02x\n", buf[0], buf[1]);
        return 1;
    }

    int humidity = (buf[2] << 8) | buf[3];
    int temperature = (buf[4] << 8) | buf[5];

    printf("Humidity: %.1f %%\n", humidity / 10.0);
    printf("Temperature: %.1f °C\n", temperature / 10.0);

    return 0;
}
