#include <iostream>
#include <chrono>
#include <thread>
#include <mqtt/async_client.h>
#include <sstream>


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

const std::string SERVER_ADDRESS = "tcp://localhost:1883";
const std::string CLIENT_ID = "am2330-publisher";
const std::string TEMP_TOPIC = "sensors/temperature";
const std::string HUMIDITY_TOPIC = "sensors/humidity";

float read_temperature_sensor() {
    // Mock function - replace with real sensor reading logic
    return 20.0 + static_cast<float>(rand() % 1000) / 100.0;
}

bool read_sensor_i2c(float &temperature, float &humidity) {
    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C bus");
        return false;
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
        return false;
    }

    usleep(2000);  // wait 2ms for sensor to respond

    // Read 8 bytes
    uint8_t buf[8];
    struct i2c_rdwr_ioctl_data read_data;
    struct i2c_msg read_msg;

    read_msg.addr = AM2320_ADDR;
    read_msg.flags = I2C_M_RD; // Read flag
    read_msg.len = sizeof(buf);
    read_msg.buf = buf;

    read_data.msgs = &read_msg;
    read_data.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &read_data) < 0) {
        perror("Failed to read data");
        close(fd);
        return false;
    }

    close(fd);

    if (buf[0] != 0x03 || buf[1] != 0x04) {
        fprintf(stderr, "Unexpected response: %02x %02x\n", buf[0], buf[1]);
        return 1;
    }

    humidity = (buf[2] << 8) | buf[3];
    temperature = (buf[4] << 8) | buf[5];

    humidity = humidity / 10.0f;
    temperature = temperature / 10.0f;



    printf("Humidity: %.1f %%\n", humidity);
    printf("Temperature: %.1f °C\n", temperature);

    return true;
}
bool read_sensor(float& temperature, float& humidity) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen("am2320_read", "r");  // Adjust path as needed
    if (!pipe) return false;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    std::istringstream iss(result);
    std::string label;

    // Parse humidity
    if (!(iss >> label >> humidity)) return false;
    if (label != "Humidity:") return false;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '%'); // Skip '%'

    // Parse temperature
    if (!(iss >> label >> temperature)) return false;
    if (label != "Temperature:") return false;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '°'); // Skip '°C'

    return true;
}

int main() {
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    mqtt::connect_options connOpts;

    try {
        client.connect(connOpts)->wait();

        std::string payload_temp;
        mqtt::message_ptr pubmsg_temp;

        std::string payload_humidity;
        mqtt::message_ptr pubmsg_humidity;


        // while (true) {
        // float temperature = read_temperature_sensor();

        float temperature = 0.0f, humidity = 0.0f;

        if (read_sensor_i2c(temperature, humidity)) {

            payload_temp = "{\"value\": " + std::to_string(temperature) + "}";
            pubmsg_temp = mqtt::make_message(TEMP_TOPIC, payload_temp);

            payload_humidity = "{\"value\": " + std::to_string(humidity) + "}";
            pubmsg_humidity = mqtt::make_message(HUMIDITY_TOPIC, payload_humidity);


            client.publish(pubmsg_temp)->wait();
            client.publish(pubmsg_humidity)->wait();
            // std::this_thread::sleep_for(std::chrono::seconds(5));



            std::cout << "Published: Temp=" << temperature << "°C, Humidity=" << humidity << "%" << std::endl;
        } else {
            std::cerr << "Failed to read sensor" << std::endl;
        }

        // std::string payload = std::to_string(temperature);
        // mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC, payload);
        // client.publish(pubmsg)->wait();

        // std::cout << "Published: " << payload << "°C" << std::endl;
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // }
        client.disconnect()->wait();
    } catch (const mqtt::exception& e) {
        std::cerr << "MQTT Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
