#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include "../kernel/aqi_sensor_ioctl.h"

#define DEVICE_NODE "/dev/aqi_sensor"

// Helper to construct JSON
int main(int argc, char *argv[]) {
    int fd;
    struct aqi_reading reading;

    // In a real-world scenario, the Python wrapper would invoke this endlessly or read continuously.
    // For this design, we will do a single read and print JSON (the Python script will run this program in a loop).

    fd = open(DEVICE_NODE, O_RDONLY);
    if (fd < 0) {
        // We print JSON error so Python receiver can parse it gracefully
        printf("{\"status\": \"ERROR\", \"error\": \"Failed to open device: %s\"}\n", strerror(errno));
        return 1;
    }

    ssize_t bytes_read = read(fd, &reading, sizeof(struct aqi_reading));
    if (bytes_read < 0) {
        printf("{\"status\": \"ERROR\", \"error\": \"Failed to read from device: %s\"}\n", strerror(errno));
        close(fd);
        return 1;
    }

    if (bytes_read != sizeof(struct aqi_reading)) {
         printf("{\"status\": \"ERROR\", \"error\": \"Incomplete struct read\"}\n");
         close(fd);
         return 1;
    }

    close(fd);

    // Format output as JSON representing the `SensorWebSocketPayload` 'metrics' sub-object
    // The Python process wraps this into the final payload scheme
    int first = 1;
    printf("{");
    if (reading.valid_fields & AQI_FIELD_ECO2) {
        printf("%s\"eco2\": %u", first ? "" : ", ", reading.eco2_ppm);
        first = 0;
    }
    if (reading.valid_fields & AQI_FIELD_TVOC) {
        printf("%s\"tvoc\": %u", first ? "" : ", ", reading.tvoc_ppb);
        first = 0;
    }
    if (reading.valid_fields & AQI_FIELD_GAS_RES) {
        printf("%s\"gasResistance\": %u", first ? "" : ", ", reading.gas_res_ohms);
        first = 0;
    }
    if (reading.valid_fields & AQI_FIELD_TEMP) {
        printf("%s\"temperature\": %.2f", first ? "" : ", ", reading.temperature_mc / 1000.0);
        first = 0;
    }
    if (reading.valid_fields & AQI_FIELD_HUMIDITY) {
        printf("%s\"humidity\": %u", first ? "" : ", ", reading.humidity_pc / 1000);
        first = 0;
    }

    // Always include status. If sensor_status != 0, it means underlying hardware error.
    if (reading.sensor_status != 0) {
        printf("%s\"hardware_status\": \"ERROR\"}\n", first ? "" : ", ");
    } else {
        printf("%s\"hardware_status\": \"OK\"}\n", first ? "" : ", ");
    }

    return 0;
}
