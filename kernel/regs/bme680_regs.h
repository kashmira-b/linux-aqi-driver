#ifndef _BME680_REGS_H
#define _BME680_REGS_H

/* Standard BME680 I2C Address */
#define BME680_I2C_ADDR_PRIMARY   0x76
#define BME680_I2C_ADDR_SECONDARY 0x77

/* Mock Register Map for i2c-stub testing */
#define BME680_REG_STATUS      0x1D
#define BME680_REG_TEMP_MSB    0x22
#define BME680_REG_TEMP_LSB    0x23
#define BME680_REG_HUM_MSB     0x25
#define BME680_REG_HUM_LSB     0x26
#define BME680_REG_GAS_RES_MSB 0x2A
#define BME680_REG_GAS_RES_LSB 0x2B

#endif /* _BME680_REGS_H */
