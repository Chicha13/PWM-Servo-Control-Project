#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define DRIVER_NAME "pwm_click"
#define PWM_CLICK_ADDR 0x40  // PCA9685 default I2C address
#define I2C_BUS_AVAILABLE (1) //The bus used

// PCA9685 Register Addresses
#define MODE1_REG 0x00
#define PRE_SCALE_REG 0xFE 

#define LED_ON_L_BASE 0x06  
#define LED_ON_H_BASE 0x07  
#define LED_OFF_L_BASE 0x08 
#define LED_OFF_H_BASE 0x09 

#define LED_ON_L_BASE1 0x0A  
#define LED_ON_H_BASE1 0x0B  
#define LED_OFF_L_BASE1 0x0C 
#define LED_OFF_H_BASE1 0x0D 

//Defining IOCTL commands
#define PWM_SET_FREQUENCY_CHANNEL _IOW('p', 1, unsigned int)
#define PWM_SET_DUTY_CYCLE_CHANNEL1 _IOW('p', 2, unsigned int)
#define PWM_SET_DUTY_CYCLE_CHANNEL2 _IOW('p', 3, unsigned int)

static struct cdev *cdev = NULL;
static struct class *pwm_click_class = NULL;
static dev_t pwm_click_dev_number;

static struct i2c_client *pwm_click_client=NULL;
static struct i2c_adapter *pwm_click_adapter=NULL;

// Function to write a byte of data to a specific register address

static int write_byte_to_register(struct i2c_client *client, u8 reg, u8 data) {
    struct i2c_msg msg;

    // Build the I2C message
    msg.addr = client->addr;
    msg.flags = 0;  // Write operation
    msg.len = 2;    // Two bytes: register address + data
    msg.buf = kmalloc(2, GFP_KERNEL);

    if (!msg.buf) {
        pr_err("Failed to allocate memory for message buffer\n");
        return -ENOMEM;
    }

    msg.buf[0] = reg;
    msg.buf[1] = data;

    // Perform the I2C transaction
    int ret = i2c_transfer(client->adapter, &msg, 1);

    if (ret < 0) {
        pr_err("I2C transfer error: %d\n", ret);

        // Print more details about the message
        pr_err("Message: addr=0x%x, flags=0x%x, len=%d\n", msg.addr, msg.flags, msg.len);
    }

    kfree(msg.buf);
    return ret;
}
   


// Function to set PWM frequency

static void set_pwm_frequency_channel(struct i2c_client *client, int frequency) {
    // Calculate prescaler value based on frequency
    int prescaler = 25000000 / (4096 * frequency) - 1;

    // Set sleep mode for writing to registers
    write_byte_to_register(client, MODE1_REG, 0x10);

    // Write prescaler value
  write_byte_to_register(client, PRE_SCALE_REG, prescaler);

    // Clear sleep mode to start PWM
    write_byte_to_register(client, MODE1_REG, 0x00);
}


// Function for setting PWM duty cycle
static void set_pwm_duty_cycle_channel(struct i2c_client *client, int channel, int duty_cycle) {
    // Calculate ON and OFF times based on duty cycle
    int on_time = 0;
    int off_time = (4096 * duty_cycle) / 100;
    // Write ON time
    write_byte_to_register(client, LED_ON_L_BASE + 4 * channel, on_time & 0xFF);  
    write_byte_to_register(client, LED_ON_H_BASE + 4 * channel, (on_time >> 8) & 0x0F);
    // Write OFF time
    write_byte_to_register(client, LED_OFF_L_BASE + 4 * channel, off_time & 0xFF); 
    write_byte_to_register(client, LED_OFF_H_BASE + 4 * channel, (off_time >> 8) & 0x0F); 
}


static ssize_t pwm_click_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

    return 0;
}

static ssize_t pwm_click_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
   
    return count;
}



static long pwm_click_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {

    switch (cmd) {
        case PWM_SET_FREQUENCY_CHANNEL:
            if (!pwm_click_client) {
                pr_err("pwm_click_client is NULL\n");
                return -EINVAL;
            }
            set_pwm_frequency_channel(pwm_click_client, arg);  // set frequency for both channels
            break;
        case PWM_SET_DUTY_CYCLE_CHANNEL1:
            if (!pwm_click_client) {
                pr_err("pwm_click_client is NULL\n");
                return -EINVAL;
            }
            set_pwm_duty_cycle_channel(pwm_click_client, 0, arg);  // set Duty cycle for channel 1
            break;
 
        case PWM_SET_DUTY_CYCLE_CHANNEL2:
            if (!pwm_click_client) {
                pr_err("pwm_click_client is NULL\n");
                return -EINVAL;
            }
            set_pwm_duty_cycle_channel(pwm_click_client, 1, arg);  // set Duty cycle for channel 2
            break;
     
        default:
            return -ENOTTY;
    }

    return 0;
}



// Implementing the file operations structure
static const struct file_operations pwm_click_fops = {
    .owner = THIS_MODULE,
    .read = pwm_click_read,
    .write = pwm_click_write,
    .unlocked_ioctl = pwm_click_ioctl,
};

static int pwm_click_probe(struct i2c_client *client, const struct i2c_device_id *id) {

    pr_info("PWM Click Probed\n");
    return 0;
}

static void pwm_click_remove(struct i2c_client *client) {
    pr_info("PWM Click Removed\n");

}

static const struct i2c_device_id pwm_click_id[] = {
    { DRIVER_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pwm_click_id);

static struct i2c_driver pwm_click_driver = {
    .driver = {
        .name = DRIVER_NAME,
    },
    .probe = pwm_click_probe,
    .remove = pwm_click_remove,
    .id_table = pwm_click_id,
};

/*
** I2C Board Info strucutre
*/
static struct i2c_board_info pwm_click_board_info = {
        I2C_BOARD_INFO(DRIVER_NAME, PWM_CLICK_ADDR)
    };
	
static int __init pwm_click_init(void) {
    int ret;
	
	pwm_click_adapter=i2c_get_adapter(I2C_BUS_AVAILABLE);
	
	 if( pwm_click_adapter != NULL )
    {
        pwm_click_client = i2c_new_client_device(pwm_click_adapter, &pwm_click_board_info);
        
        if( pwm_click_client != NULL )
        {
           i2c_add_driver(&pwm_click_driver);
            ret = 0;
        }
        
        i2c_put_adapter(pwm_click_adapter);
    }
	struct device *dev;
    // Allocate the device number dynamically
    ret = alloc_chrdev_region(&pwm_click_dev_number, 0, 1, DRIVER_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device number\n");
        return ret;
    }

    // Create a character device
    cdev = cdev_alloc();
    if (!cdev) {
        pr_err("Failed to allocate cdev\n");
        unregister_chrdev_region(pwm_click_dev_number, 1);
        return -ENOMEM;
    }

    // Set up file operations
    cdev_init(cdev, &pwm_click_fops);
    ret = cdev_add(cdev, pwm_click_dev_number, 1);
    if (ret < 0) {
        pr_err("Failed to add char device\n");
        kobject_put(&cdev->kobj);
        unregister_chrdev_region(pwm_click_dev_number, 1);
        return ret;
    }

    // Create a device class
    pwm_click_class = class_create(THIS_MODULE, "pwm_click_class");
    if (IS_ERR(pwm_click_class)) {
        pr_err("Failed to create class\n");
        cdev_del(cdev);
        unregister_chrdev_region(pwm_click_dev_number, 1);
        return PTR_ERR(pwm_click_class);
    }

	  dev = device_create(pwm_click_class, NULL, pwm_click_dev_number, NULL, "pwm_click_device");
    if (IS_ERR(dev)) {
        pr_err("Failed to create device\n");
        return PTR_ERR(dev);
    }
    
    pr_info("PWM Click Driver Loaded\n");
    return 0;
}

static void __exit pwm_click_exit(void) {
    device_destroy(pwm_click_class, pwm_click_dev_number);
    class_destroy(pwm_click_class);
    cdev_del(cdev);
    i2c_del_driver(&pwm_click_driver);
    unregister_chrdev_region(pwm_click_dev_number, 1);
}

module_init(pwm_click_init);
module_exit(pwm_click_exit);

MODULE_AUTHOR("PWM Servo Control Project");
MODULE_DESCRIPTION("PWM Click I2C Driver");
MODULE_LICENSE("GPL");
