/*=====================================*/
/* Kernel lib */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <drv_timer.h>
#include <linux/types.h>

/*=====================================*/
#include "pega_gpio.h"
#include "pega_motor_awd8833.h"
/*=====================================*/
#define  DEBUG_EN

#ifdef DEBUG_EN
	#define	LOG(fmt,args...)	printk(KERN_ERR "[%s]::"  fmt,__func__,## args);
#else 
	#define LOG(fmt, args...) 
#endif

#define PAN_SPEED				2		//(ms)
#define RISING_SPEED			10		//(ms)
#define MOTOR_TIMER				4

struct motor_control{
	s32	step;							//step  base : 0  ( forward +1 ; reverse -1 )
	s32 degree;							//degree  base : 0 ( step*1/85*7.5 )
};

struct timer_stamp{
	bool en;							//disable:0	;	enable:1
	u32 count;							//timer count
	s64 timestamp_ns[256];
	int stamp_num;
	ifs_timer_handle	handle;			//motor callback
};

struct motor_chip{
	struct device			*dev;
	u8	motor;							//pan:0	;	rising:1 
	u8	dir;
	struct motor_control	pan;
	struct motor_control	rising;
	struct timer_stamp		time;
	struct mutex			lock;
};

void motor_irs(void *pdata)		//pdata = chip
{
	struct motor_chip *chip = pdata;
	/* timing recording */
	chip->time.timestamp_ns[chip->time.stamp_num % 256] = ktime_get();
	chip->time.stamp_num++;
	/*--------------------*/
	if(chip->time.count)		//chip->timer.expire_count
	{	
		switch (chip->dir)
		{
			case AW_FORWARD:
				if(chip->motor==AW_PAN_MOTOR)
				{
					awd8833c_half_step_forward_function();
					chip->pan.step++;
				}else{
					awd8833c_full_step_forward_function();
					chip->rising.step++;
				}
				break;
			
			case AW_REVERSE:
				if(chip->motor==AW_PAN_MOTOR)
				{
					awd8833c_half_step_reverse_function();
					chip->pan.step--;
				}else{
					awd8833c_full_step_reverse_function();
					chip->rising.step--;
				}
				break;

			default:
				break;
		}
	/* timing recording */	
		chip->time.count--;
		chip->time.timestamp_ns[chip->time.stamp_num % 256] = ktime_get();
		chip->time.stamp_num++;
	/*--------------------*/
		return;
	}
	chip->time.en=false;
	awd8833c_gpio_init();
	chip->pan.degree = (chip->pan.step*3/34)%360;
	chip->rising.degree = (chip->rising.step*15)%360;
	ifs_timer_stop(chip->time.handle);		//  will leave directly
	return;
}

static ssize_t info_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	char* line = "================================================\n";
	char* info = "[module]\tMotor controll via timer and gpio.\n";
	char* workflow = 
				"#.\tSelect the motor type.\n"
				"#.\tSet the motor direction.\n"	
				"#.\tSet timer count(step).\n"
				"#.\tEnable the timer(motor).\n";		
	return sprintf(buf, "%s%s%s%s",line,info,workflow,line);
};

static ssize_t motor_version(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "[module]\t%s", AW8646_DRIVER_VERSION);
};

static ssize_t motor_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->motor==AW_PAN_MOTOR)
		val="[module]\tmotor: PAN MOTOR\n";
	else
		val="[module]\tmotor: RISING MOTOR\n";
	
	return sprintf(buf, "%s", val);
};

static ssize_t motor_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return -EINVAL;  
	}

	if(val==AW_RISING_MOTOR)
	{
		chip->motor = AW_RISING_MOTOR;
		printk("[module]\tmotor: RISING MOTOR\n");
	}else{
		/* Default motor : PAN */
		chip->motor = AW_PAN_MOTOR;
		printk("[module]\tmotor: PAN MOTOR\n");
	}

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t timer_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->time.en)
		val="[module]\ttimer/motor: Enable\n";
	else
		val="[module]\ttimer/motor: Disnable\n";
	
	return sprintf(buf, "%s", val);
};

static ssize_t timer_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	if(val)	
	{
		printk("[module]Enable the timer,Motor start to run.\n");
		awd8833c_restore_state(chip->motor);
		awd8833c_set_enable(AW_TRUE, chip->motor);					//Enable the target motor
		chip->time.en = true;
		switch(chip->motor)
		{
			case AW_PAN_MOTOR:
				ifs_timer_start(chip->time.handle, PAN_SPEED);
				break;
			case AW_RISING_MOTOR:
				ifs_timer_start(chip->time.handle, RISING_SPEED);	
				break;
		}
	}else{
		printk("[module]Stop the Timer and motor.\n");
		ifs_timer_stop(chip->time.handle);
		awd8833c_gpio_init();
		chip->time.en = false;
		chip->pan.degree = (chip->pan.step*3/34)%360;
		chip->rising.degree = (chip->rising.step*15)%360;
	}

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t dir_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;
	
	if(chip->dir==AW_FORWARD)
		val="[module]\tdirection : forward.\n";
	else if(chip->dir==AW_REVERSE)
		val="[module]\tdirection : reverse.\n";

	return sprintf(buf, "%s", val);
};

static ssize_t dir_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	if(val==AW_FORWARD)	
	{
		printk("[module]\tdirection : forward.\n");
		chip->dir = AW_FORWARD;
	}else if(val==AW_REVERSE){
		printk("[module]\tdirection : reverse.\n");
		chip->dir = AW_REVERSE;
	}else{
		printk("[module]\tunkown parameter.\n");
	}

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t step_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	return sprintf(buf, "[module]\tPan step:%d\n"
						"[module]\tRising step:%d\n",
						chip->pan.step,chip->rising.step);
};

static ssize_t angle_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	return sprintf(buf, "[module]\tPan angle:%d\n"
						"[module]\tRising angle:%d\n",
						chip->pan.degree,chip->rising.degree);
}

static ssize_t count_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	return sprintf(buf, "[module]\tcount left:%d\n", chip->time.count);
};

static ssize_t count_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	if(!chip->time.en){
		if(val>0)	
		{
			printk("[module]\ttimer count : %d.\n",val);
			chip->time.count = val;
		}else
		{
			printk("[module]\ttimer count : %d.\n",0);
			chip->time.count = 0;
		}
	}else{
		printk("timer is running,unable to be modified.\n");
	}
	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t timestamp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t len = 0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	int i;

    for (i = 0; i < 256; i++) {
        len += scnprintf(buf + len, PAGE_SIZE - len,
                         "[module]\ttimestamp_ns[%3d] = %lld\n", i, chip->time.timestamp_ns[i]);

        if (len >= PAGE_SIZE - 1)
            break;
    }

    return len;
}

static DEVICE_ATTR(readme,0664,info_show,NULL);
static DEVICE_ATTR(version,0664,motor_version,NULL);
static DEVICE_ATTR(motor,0664,motor_show,motor_store);
static DEVICE_ATTR(timer,0664,timer_show,timer_store);
static DEVICE_ATTR(direction,0664,dir_show,dir_store);
static DEVICE_ATTR(step,0664,step_show,NULL);
static DEVICE_ATTR(angle,0664,angle_show,NULL);
static DEVICE_ATTR(count,0664,count_show,count_store);
static DEVICE_ATTR(timestamp,0664,timestamp_show,NULL);

static struct attribute *motor_attrs[] = {
	&dev_attr_readme.attr,
	&dev_attr_version.attr,
	&dev_attr_motor.attr,
	&dev_attr_timer.attr,
	&dev_attr_direction.attr,
	&dev_attr_step.attr,
	&dev_attr_angle.attr,
	&dev_attr_count.attr,
	&dev_attr_timestamp.attr,
    NULL,
};
static const struct attribute_group motor_attr_group = {
    .attrs = motor_attrs,
};

static int motor_probe(struct platform_device *pdev)
{
	int ret;
	struct motor_chip *chip;
    
	/* init GPIO */
	if(Motor_gpio_init(pdev))
	{
		dev_err(&pdev->dev, "GPIO Init Error\n");
		return 1;
	}

	/* init chip */
	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		dev_err(&pdev->dev, "%s: failed to allocate driver data.\n", __func__);
		return -ENOMEM;
	}

	/* Init Hardware Timer */
	memset(&chip->pan, 0, sizeof(struct motor_chip));
	memset(&chip->rising, 0, sizeof(struct motor_chip));

	chip->dev = &pdev->dev;	
	/* init mutex */
	mutex_init(&chip->lock);
	/* init attr */
	chip->motor =	AW_PAN_MOTOR;
	chip->time.en =	AW_FALSE;
	chip->time.count =	0;
	chip->time.handle	= ifs_timer_register(MOTOR_TIMER, IFS_TIMER_MODE_RUNLOOP, motor_irs, chip);
	chip->dir =	AW_FORWARD;
	chip->pan.step =	0;
	chip->pan.degree =	0;
	chip->rising.step =	0;
	chip->rising.degree = 0;

	
	platform_set_drvdata(pdev,chip);

	ret = sysfs_create_group(&pdev->dev.kobj, &motor_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "Failed to create sysfs group\n");
		return ret;
	}
	/*	GPIO Init	*/
	awd8833c_gpio_init();
	
	LOG("Finished Motor probe\n");
    return 0;
}

static int motor_remove(struct platform_device *pdev)
{
    struct motor_chip *chip = platform_get_drvdata(pdev);
    if (chip){
        ifs_timer_unregister(chip->time.handle);
	}
	return 0;
}

static const struct of_device_id motor_of_match[] = {
   { .compatible = "awinic,motor", },
    { },
};

MODULE_DEVICE_TABLE(of, motor_of_match);

static struct platform_driver motor_driver = {
    .probe = motor_probe,
    .remove = motor_remove,
    .driver = {
        .name = "motor_driver",
        .of_match_table = motor_of_match,
    },
};


module_platform_driver(motor_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Willis");
MODULE_DESCRIPTION("Motor Control Driver");
