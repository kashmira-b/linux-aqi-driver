#ifndef _AQI_SENSOR_IOCTL_H
#define _AQI_SENSOR_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define AQI_SENSOR_MAGIC 'A'

/* Valid Fields Bitmask Flags */
#define AQI_FIELD_ECO2     (1 << 0)
#define AQI_FIELD_TVOC     (1 << 1)
#define AQI_FIELD_GAS_RES  (1 << 2)
#define AQI_FIELD_TEMP     (1 << 3)
#define AQI_FIELD_HUMIDITY (1 << 4)

/* The unified reading structure returned by the driver */
struct aqi_reading {
    __u64 timestamp_ms;
    __u32 valid_fields; // Bitmask of valid readings
    __u32 eco2_ppm;
    __u32 tvoc_ppb;
    __u32 gas_res_ohms;
    __s32 temperature_mc; // milliCelsius
    __u32 humidity_pc;    // percentage * 1000
    __u32 sensor_status;  // 0 = OK, non-zero = error code
};

/* IOCTL Commands */
// Reset the sensor hardware
#define AQI_IOC_RESET _IO(AQI_SENSOR_MAGIC, 1)

// Force an immediate measurement (if not in continuous polling mode)
#define AQI_IOC_MEASURE _IO(AQI_SENSOR_MAGIC, 2)

#endif /* _AQI_SENSOR_IOCTL_H */
