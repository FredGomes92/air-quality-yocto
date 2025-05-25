
#include "pms5003.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqtt/async_client.h>
#include <iostream>
#include <cmath>    // for round()
#include <vector>

const std::string SERVER_ADDRESS = "tcp://localhost:1883";
const std::string CLIENT_ID = "pms5003-publisher";
const std::string PMS_TOPIC = "sensors/pms";


void print_data(const PMS5003_DATA *data) {
    printf("Parsed PMS5003 Data:\n");
    printf("Frame length: %u\n", (data->d.f_length));
    printf("PM1.0 CF=1:  %u µg/m³\n", data->d.pm1cf);
    printf("PM2.5 CF=1:  %u µg/m³\n", data->d.pm2_5cf);
    printf("PM10  CF=1:  %u µg/m³\n", data->d.pm10cf);
    printf("PM1.0 ATM:   %u µg/m³\n", data->d.pm1at);
    printf("PM2.5 ATM:   %u µg/m³\n", data->d.pm2_5at);
    printf("PM10  ATM:   %u µg/m³\n", data->d.pm10at);
    printf(">0.3µm:      %u\n", data->d.gt0_3);
    printf(">0.5µm:      %u\n", data->d.gt0_5);
    printf(">1.0µm:      %u\n", data->d.gt1);
    printf(">2.5µm:      %u\n", data->d.gt2_5);
    printf(">5.0µm:      %u\n", data->d.gt5);
    printf(">10µm:       %u\n", data->d.gt10);
    printf("Checksum:    0x%04X\n", data->d.cksum);

    printf("\nRaw Data:\n");
    for (int i = 0; i < PMS5003_EXPECTED_BYTES; i++) {
        printf("%02X ", data->raw_data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}


struct Breakpoint {
    double BpL, BpH; // PM2.5 concentration range (μg/m³)
    int Il, Ih;      // AQI range
};

// Generic AQI calculator
int calculate_aqi(double concentration, const std::vector<Breakpoint>& breakpoints) {
    for (const auto& bp : breakpoints) {
        if (concentration >= bp.BpL && concentration <= bp.BpH) {
            double aqi = ((bp.Ih - bp.Il) / (bp.BpH - bp.BpL)) * (concentration - bp.BpL) + bp.Il;
            return static_cast<int>(round(aqi));
        }
    }
    return -1; // Out of range
}

// PM2.5 breakpoints (EPA)
const std::vector<Breakpoint> PM25_BREAKPOINTS = {
    {0.0, 12.0, 0, 50},
    {12.1, 35.4, 51, 100},
    {35.5, 55.4, 101, 150},
    {55.5, 150.4, 151, 200},
    {150.5, 250.4, 201, 300},
    {250.5, 500.4, 301, 500}
};

// PM10 breakpoints (EPA)
const std::vector<Breakpoint> PM10_BREAKPOINTS = {
    {0, 54, 0, 50},
    {55, 154, 51, 100},
    {155, 254, 101, 150},
    {255, 354, 151, 200},
    {355, 424, 201, 300},
    {425, 504, 301, 400},
    {505, 604, 401, 500}
};

// PM1.0: No official AQI standard, so you can reuse PM2.5 breakpoints as an approximation
const std::vector<Breakpoint> PM1_BREAKPOINTS = PM25_BREAKPOINTS;

// Wrapper to select the right breakpoints
int get_aqi(double concentration, double pm_type) {
    if (pm_type == 1.0) {
        return calculate_aqi(concentration, PM1_BREAKPOINTS);
    } else if (pm_type == 2.5) {
        return calculate_aqi(concentration, PM25_BREAKPOINTS);
    } else if (pm_type == 10.0) {
        return calculate_aqi(concentration, PM10_BREAKPOINTS);
    } else {
        return -1; // Invalid PM type
    }
}

void print_data_le(const PMS5003_DATA_LE *data) {
    printf("Parsed PMS5003 Data:\n");
    printf("Frame length: %u\n", (data->f_length));
    printf("PM1.0 CF=1:  %u µg/m³\n", data->pm1cf);
    printf("PM2.5 CF=1:  %u µg/m³\n", data->pm2_5cf);
    printf("PM10  CF=1:  %u µg/m³\n", data->pm10cf);
    printf("PM1.0 ATM:   %u µg/m³\n", data->pm1at);
    printf("PM2.5 ATM:   %u µg/m³\n", data->pm2_5at);
    printf("PM10  ATM:   %u µg/m³\n", data->pm10at);
    printf(">0.3µm:      %u\n", data->gt0_3);
    printf(">0.5µm:      %u\n", data->gt0_5);
    printf(">1.0µm:      %u\n", data->gt1);
    printf(">2.5µm:      %u\n", data->gt2_5);
    printf(">5.0µm:      %u\n", data->gt5);
    printf(">10µm:       %u\n", data->gt10);
    printf("Checksum:    0x%04X\n", data->cksum);

    printf("\n");
}

std::string create_payload(const PMS5003_DATA_LE *data) {
    char payload[256];

    // Calculate AQI for PM2.5
    int aqi_pm25 = get_aqi(data->pm2_5at, 2.5);
    int aqi_pm10 = get_aqi(data->pm10at, 10.0);
    snprintf(payload, sizeof(payload),
             "{ \"pms\": { \"pm1.0_atm\": %u, \"pm2.5_atm\": %u, \"pm10_atm\": %u, \"aqi_pm25\": %d, \"aqi_pm10\": %d }}",
             data->pm1at, data->pm2_5at, data->pm10at, aqi_pm25, aqi_pm10);
    return std::string(payload);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/ttyUSB0\n", argv[0]);
        return 1;
    }

    if (pms_init(argv[1], 9600) != 0) {
        fprintf(stderr, "Failed to initialize PMS5003\n");
        return 1;
    }
    PMS5003_DATA data;
    PMS5003_DATA_LE data_le;

    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    mqtt::connect_options connOpts;

    bool valid_frame = false;
    int max_retries = 5;

    try {
        client.connect(connOpts)->wait();

        std::string payload_pms;
        mqtt::message_ptr pubmsg_pms;
        flush_uart();

        while (!valid_frame && max_retries-->0) {

            if (read_pms_frame(&data, 0.5) == 0) {
                convert_pms_data_to_le(&data, &data_le);
                // print_data(&data);
                print_data_le(&data_le);

                printf("Valid frame received!\n");

                payload_pms = create_payload(&data_le);
                pubmsg_pms = mqtt::make_message(PMS_TOPIC, payload_pms);
                client.publish(pubmsg_pms)->wait();
                std::cout << "Published: PMS =" << payload_pms << "\n";

                valid_frame = true;
            } else {
                fprintf(stderr, "Failed to read data from PMS5003, retrying... (attempt = %d)\n", 5 - max_retries);
                usleep(2000000); // wait 1s
                reset_uart(argv[1], 9600);
                flush_uart();
                usleep(1000000); // wait 1s

            }
        }
        client.disconnect()->wait();
    } catch (const mqtt::exception& e) {
        std::cerr << "MQTT Error: " << e.what() << std::endl;
        return 1;
    }

    return valid_frame ? 0 : 1;
}