#ifndef _CCS811_REGS_H
#define _CCS811_REGS_H

/* Standard CCS811 I2C Address */
#define CCS811_I2C_ADDR_PRIMARY 0x5A

/* Mock Register Map for i2c-stub testing */
#define CCS811_REG_STATUS    0x00
#define CCS811_REG_MEAS_MODE 0x01
#define CCS811_REG_ALG_RESULT_DATA 0x02

#endif /* _CCS811_REGS_H */
