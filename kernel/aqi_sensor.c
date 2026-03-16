#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include "aqi_sensor_ioctl.h"
#include "regs/ccs811_regs.h"

#define DRIVER_NAME "aqi_sensor"
#define CLASS_NAME "aqi"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kashmira Pusalkar");
MODULE_DESCRIPTION("Air Quality Sensor Mock Driver via I2C");
MODULE_VERSION("0.1");

static int major_number;
static struct class *aqi_class = NULL;
static struct device *aqi_device = NULL;
static struct cdev aqi_cdev;
static struct i2c_client *aqi_client = NULL; // The bound I2C sensor

// Dynamically reads from I2C for Phase 5 (US3)
static void get_sensor_reading(struct aqi_reading *reading)
{
    struct timespec64 ts;
    ktime_get_real_ts64(&ts);
    reading->timestamp_ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    
    // T020: Graceful Disconnect / Error surfacing
    if (!aqi_client) {
        reading->valid_fields = 0;
        reading->sensor_status = 1; // Error state
        return;
    }

    // T018: Read raw I2C SMbus data
    // Assuming CCS811 algorithm result register (0x02) which outputs 2 bytes eCO2 then 2 bytes TVOC
    // Using simple byte reads for mock purposes with i2c-stub
    int eco2_high = i2c_smbus_read_byte_data(aqi_client, CCS811_REG_ALG_RESULT_DATA);
    int eco2_low  = i2c_smbus_read_byte_data(aqi_client, CCS811_REG_ALG_RESULT_DATA + 1);
    int tvoc_high = i2c_smbus_read_byte_data(aqi_client, CCS811_REG_ALG_RESULT_DATA + 2);
    int tvoc_low  = i2c_smbus_read_byte_data(aqi_client, CCS811_REG_ALG_RESULT_DATA + 3);

    if (eco2_high < 0 || eco2_low < 0) {
        reading->valid_fields = 0;
        reading->sensor_status = 2; // I2C Comms Error
        return;
    }

    reading->valid_fields = AQI_FIELD_ECO2 | AQI_FIELD_TVOC;
    reading->eco2_ppm = (eco2_high << 8) | eco2_low;
    reading->tvoc_ppb = (tvoc_high << 8) | tvoc_low;
    reading->sensor_status = 0;
}

static int aqi_open(struct inode *inodep, struct file *filep)
{
    pr_info("%s: Device opened\n", DRIVER_NAME);
    return 0;
}

static ssize_t aqi_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    struct aqi_reading reading;
    int error_count = 0;

    // T015 [US2] Add buffer size validation returning -EINVAL
    if (len < sizeof(struct aqi_reading)) {
        pr_err("%s: Read buffer too small (expected %zu, got %zu)\n", 
               DRIVER_NAME, sizeof(struct aqi_reading), len);
        return -EINVAL;
    }

    get_sensor_reading(&reading);

    error_count = copy_to_user(buffer, &reading, sizeof(struct aqi_reading));

    if (error_count == 0) {
        return sizeof(struct aqi_reading);
    } else {
        pr_err("%s: Failed to send %d chars to user\n", DRIVER_NAME, error_count);
        return -EFAULT;
    }
}

static long aqi_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case AQI_IOC_RESET:
            pr_info("%s: IOCTL Reset triggered\n", DRIVER_NAME);
            break;
        case AQI_IOC_MEASURE:
            pr_info("%s: IOCTL Force Measure triggered\n", DRIVER_NAME);
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static int aqi_release(struct inode *inodep, struct file *filep)
{
    pr_info("%s: Device successfully closed\n", DRIVER_NAME);
    return 0;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = aqi_open,
    .read = aqi_read,
    .unlocked_ioctl = aqi_ioctl,
    .release = aqi_release,
};

static int __init aqi_init(void)
{
    int ret;
    dev_t dev_num;

    pr_info("%s: Initializing kernel module\n", DRIVER_NAME);

    ret = alloc_chrdev_region(&dev_num, 0, 1, DRIVER_NAME);
    major_number = MAJOR(dev_num);
    if (ret < 0) {
        pr_err("%s: Failed to allocate major number\n", DRIVER_NAME);
        return ret;
    }

    cdev_init(&aqi_cdev, &fops);
    aqi_cdev.owner = THIS_MODULE;
    ret = cdev_add(&aqi_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        pr_err("%s: Failed to add cdev\n", DRIVER_NAME);
        return ret;
    }

    aqi_class = class_create(CLASS_NAME);
    if (IS_ERR(aqi_class)) {
        cdev_del(&aqi_cdev);
        unregister_chrdev_region(dev_num, 1);
        pr_err("%s: Failed to register device class\n", DRIVER_NAME);
        return PTR_ERR(aqi_class);
    }

    aqi_device = device_create(aqi_class, NULL, dev_num, NULL, DRIVER_NAME);
    if (IS_ERR(aqi_device)) {
        class_destroy(aqi_class);
        cdev_del(&aqi_cdev);
        unregister_chrdev_region(dev_num, 1);
        pr_err("%s: Failed to create device\n", DRIVER_NAME);
        return PTR_ERR(aqi_device);
    }

    pr_info("%s: Node /dev/%s created successfully with major %d\n", 
            DRIVER_NAME, DRIVER_NAME, major_number);
    return 0;
}

static void __exit aqi_exit(void)
{
    dev_t dev_num = MKDEV(major_number, 0);
    device_destroy(aqi_class, dev_num);
    class_destroy(aqi_class);
    cdev_del(&aqi_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("%s: Module successfully unloaded\n", DRIVER_NAME);
}

// T017: I2C Driver Probe and Remove
static int aqi_i2c_probe(struct i2c_client *client)
{
    pr_info("%s: I2C device found at address 0x%x\n", DRIVER_NAME, client->addr);
    aqi_client = client;
    return 0;
}

static void aqi_i2c_remove(struct i2c_client *client)
{
    pr_info("%s: I2C device removed\n", DRIVER_NAME);
    aqi_client = NULL;
}

static const struct i2c_device_id aqi_id[] = {
    { "ccs811", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, aqi_id);

static struct i2c_driver aqi_i2c_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
    .probe = aqi_i2c_probe,
    .remove = aqi_i2c_remove,
    .id_table = aqi_id,
};

// Override the basic module init to also register the I2C driver
static int __init aqi_full_init(void)
{
    int ret = aqi_init();
    if (ret) return ret;
    return i2c_add_driver(&aqi_i2c_driver);
}

static void __exit aqi_full_exit(void)
{
    i2c_del_driver(&aqi_i2c_driver);
    aqi_exit();
}

module_init(aqi_full_init);
module_exit(aqi_full_exit);
