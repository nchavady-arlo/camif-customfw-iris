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
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/kobject.h> 
/*=====================================*/
#include "drv_motor_gpio.h"
#include "drv_motor_awd8833.h"
/*=====================================*/
#define  DEBUG_EN

#ifdef DEBUG_EN
	#define	PR_DEBUG(fmt,args...)	printk(KERN_ERR "[%s]::"  fmt,__func__,## args);
#else 
	#define PR_DEBUG(fmt, args...) 
#endif


/* Add Gear --> Degree */
/* Rising : Step x 14 / 10 */
/* Pan    : Step x 51 / 9 */
struct motor_property{
	s32	step;							//step  base : 0  ( forward +1 ; reverse -1 )
	s32 degree;							//degree  base : 0 ( pan : step /34 *3; rising :step*(14/10)*15)
	s32 speed;
};

struct timer_stamp{
	bool en;							//disable:0	;	enable:1
	u32 count;							//timer count
	ifs_timer_handle	handle;			//motor callback
};

struct motor_chip{
	struct device			*dev;
	u8						motor;		//	pan/rising
	u8						dir;		//	reverse/forward
	bool					status;		//	idle/busy
	struct motor_property	pan;
	struct motor_property	rising;
	struct timer_stamp		time;
	struct mutex			lock;
};

void drv_motor_isr(void *pdata)		//pdata = chip
{
	struct motor_chip *chip = pdata;
	ktime_t timestamp;
	timestamp = ktime_get();
	printk(KERN_INFO"Current ktime: %lld us\n", ktime_to_us(timestamp));
	if(chip->time.count)		//chip->timer.expire_count
	{	
		switch (chip->dir)
		{
			case AW_FORWARD:
				if(chip->motor==AW_PAN_MOTOR)
				{
					drv_motor_halfstep_forward();
					chip->pan.step++;
				}else{
					drv_motor_fullstep_forward();
					chip->rising.step++;
				}
				break;
			
			case AW_REVERSE:
				if(chip->motor==AW_PAN_MOTOR)
				{
					drv_motor_halfstep_reverse();
					chip->pan.step--;
				}else{
					drv_motor_fullstep_reverse();
					chip->rising.step--;
				}
				break;

			default:
				break;
		}
		chip->time.count--;
	}else{
		chip->time.en=AW_IDLE;					//  Clear the status
		drv_motor_io_init();					//  Off all Control IO
		ifs_timer_stop(chip->time.handle);		//  will leave directly
	}
	chip->pan.degree = (chip->pan.step*3/34)%360;
	chip->rising.degree = (chip->rising.step*15)%360;
	return;
}

static ssize_t info_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char  *type,*status,*dir;
	
	if(chip->motor)
			type="Rising";
		else
			type="Pan";

	if(chip->time.en)
		status="Busy";
	else
		status="Idle";
	
	
	if(chip->dir)
		dir="Forward";
	else
		dir="Reverse";
	
	return sprintf(buf, "================================================\n"
						"[module] List all motor property.\n"
						"#.Motor		: %s\n"
						"#.Status	: %s\n"
						"#.Direction 	: %s\n"
						"================================================\n"
						"#.Pan motor current step	: %d\n"
						"#.Rising motor current step	: %d\n"
						"#.Pan motor current angle	: %d\n"
						"#.Rising motor current angle	: %d\n"
						"================================================\n"
						,type,status,dir,chip->pan.step,chip->rising.step,chip->pan.degree,chip->rising.degree
					);
};

static ssize_t motor_version(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "[module] %s", AW8646_DRIVER_VERSION);
};

static ssize_t motor_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->motor==AW_PAN_MOTOR)
		val="PAN MOTOR";
	else
		val="RISING MOTOR";
	
	return sprintf(buf, "%s", val);
};

static ssize_t motor_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	// Check motor status
	if(chip->time.en)
	{
		PR_DEBUG("Motor is running,unable to be modified.\n");
		return size;  
	}

	mutex_lock(&chip->lock);
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return -EINVAL;  
	}

	if(val==AW_RISING_MOTOR)
	{
		chip->motor = AW_RISING_MOTOR;
		PR_DEBUG("[module] \tmotor: rising motor\n");
	}else{
		/* Default motor : PAN */
		chip->motor = AW_PAN_MOTOR;
		PR_DEBUG("[module] \tmotor: pan motor\n");
	}

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t status_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->time.en)
		val="Busy";
	else
		val="Idle";
	
	return sprintf(buf, "%s", val);
};

static ssize_t status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);

	mutex_lock(&chip->lock);
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	if(val)// Start
	{
		PR_DEBUG("[module] Enable the timer,Motor start to run.\n");
		drv_motor_restore_IOstatus(chip->motor);
		drv_motor_sel(AW_BUSY, chip->motor);					//Enable the target motor
		chip->time.en = AW_BUSY;
		switch(chip->motor)
		{
			case AW_PAN_MOTOR:
				ifs_timer_start(chip->time.handle, chip->pan.speed);
				break;
			case AW_RISING_MOTOR:
				ifs_timer_start(chip->time.handle, chip->rising.speed);	
				break;
		}
	}else{	//	Pause
		PR_DEBUG("[module] Stop the Timer and motor.\n");
		ifs_timer_stop(chip->time.handle);
		drv_motor_io_init();
		chip->time.en = AW_IDLE;
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
		val="Forward";
	else if(chip->dir==AW_REVERSE)
		val="Reverse";

	return sprintf(buf, "%s", val);
};

static ssize_t dir_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);

	// Check motor status
	if(chip->time.en)
	{
		PR_DEBUG("Motor is running,unable to be modified.\n");
		return size;  
	}

	mutex_lock(&chip->lock);
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	if(val==AW_FORWARD)	
	{
		PR_DEBUG("[module] direction : Forward.\n");
		chip->dir = AW_FORWARD;
	}else if(val==AW_REVERSE){
		PR_DEBUG("[module] direction : Reverse.\n");
		chip->dir = AW_REVERSE;
	}else{
		PR_DEBUG("[module] Unkown parameter.\n");
	}
	mutex_unlock(&chip->lock);
	return size;  
};

/* For Debug */
static ssize_t speed_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);

	return sprintf(buf, "Speed Levl\r\nPan: %d\tRising: %d",chip->pan.speed,chip->rising.speed);
};

static ssize_t speed_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret=0;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);

	// Check motor status
	if(chip->time.en)
	{
		PR_DEBUG("Motor is running,unable to be modified.\n");
		return size;  
	}

	mutex_lock(&chip->lock);
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	switch(val)	
	{
		case 1:
			if(chip->motor==AW_PAN_MOTOR)
			{
				chip->pan.speed = PAN_SPEED_1;
				PR_DEBUG("[module] PAN Speed Level: 1.\n");
			}
			else
			{
				chip->rising.speed = RISING_SPEED_1;
				PR_DEBUG("[module] Rising Speed Level: 1.\n");
			}
			break;
		case 2:
			if(chip->motor==AW_PAN_MOTOR)
			{
				chip->pan.speed = PAN_SPEED_2;
				PR_DEBUG("[module] PAN Speed Level: 2.\n");
			}
			else
			{
				chip->rising.speed = RISING_SPEED_2;
				PR_DEBUG("[module] Rising Speed Level: 2.\n");
			}
			break;
		case 3:
			if(chip->motor==AW_PAN_MOTOR)
			{
				chip->pan.speed = PAN_SPEED_3;
				PR_DEBUG("[module] PAN Speed Level: 3.\n");
			}
			else
			{
				chip->rising.speed = RISING_SPEED_3;
				PR_DEBUG("[module] Rising Speed Level: 3.\n");
			}
			break;
		case 4:
			if(chip->motor==AW_PAN_MOTOR)
			{
				chip->pan.speed = PAN_SPEED_4;
				PR_DEBUG("[module] PAN Speed Level: 4.\n");
			}
			else
			{
				chip->rising.speed = RISING_SPEED_4;
				PR_DEBUG("[module] Rising Speed Level: 4.\n");
			}
			break;		
		default:
			chip->pan.speed = PAN_SPEED_1;
			PR_DEBUG("[module] PAN Speed Level: 1.\n");
			chip->rising.speed = RISING_SPEED_1;
			PR_DEBUG("[module] Rising Speed Level: 1.\n");
			break;
	}
	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t step_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d;%d",chip->pan.step,chip->rising.step);
};

static ssize_t angle_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d;%d",chip->pan.degree,chip->rising.degree);
}

static ssize_t count_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d", chip->time.count);
};

static ssize_t count_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	// Check motor status
	if(chip->time.en)
	{
		PR_DEBUG("Motor is running,unable to be modified.\n");
		return size;  
	}

	mutex_lock(&chip->lock);
	if(kstrtouint(buf, 10, &val)){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	if(val>0)	
	{
		PR_DEBUG("[module] timer count : %d.\n",val);
		chip->time.count = val;
	}else
	{
		PR_DEBUG("[module] timer count : %d.\n",0);
		chip->time.count = 0;
	}
	mutex_unlock(&chip->lock);
	return size;  
};

static DEVICE_ATTR(info,0664,info_show,NULL);
static DEVICE_ATTR(version,0664,motor_version,NULL);
static DEVICE_ATTR(motor,0664,motor_show,motor_store);
static DEVICE_ATTR(status,0664,status_show,status_store);
static DEVICE_ATTR(direction,0664,dir_show,dir_store);
static DEVICE_ATTR(step,0664,step_show,NULL);
static DEVICE_ATTR(angle,0664,angle_show,NULL);
static DEVICE_ATTR(count,0664,count_show,count_store);
static DEVICE_ATTR(speed,0664,speed_show,speed_store);

static struct attribute *motor_attrs[] = {
	&dev_attr_info.attr,
	&dev_attr_version.attr,
	&dev_attr_motor.attr,
	&dev_attr_status.attr,
	&dev_attr_direction.attr,
	&dev_attr_step.attr,
	&dev_attr_angle.attr,
	&dev_attr_speed.attr,
	&dev_attr_count.attr,
    NULL,
};
static const struct attribute_group motor_attr_group = {
    .attrs = motor_attrs,
};

static int drv_motor_probe(struct platform_device *pdev)
{
	int ret;
	struct motor_chip *chip;
    
	/* config gpio */
	ret = drv_motor_config_io(pdev); 
	if (ret) {
		dev_err(&pdev->dev, "GPIO Init Error\n");
		return 1;
	}

	/* init chip */  
	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		dev_err(&pdev->dev, "%s: failed to allocate driver data.\n", __func__);
		return -ENOMEM;
	}
	
	/*	init gpio	*/
	drv_motor_io_init();
	
	/* init attr */
	chip->dev = &pdev->dev;	
	mutex_init(&chip->lock);
	chip->motor =	AW_PAN_MOTOR;
	chip->time.en =	AW_IDLE;
	chip->time.count =	0;
	chip->time.handle	= ifs_timer_register(MOTOR_TIMER, IFS_TIMER_MODE_RUNLOOP, drv_motor_isr, chip);
	chip->dir =	AW_FORWARD;
	chip->pan.step =	0;
	chip->pan.degree =	0;
	chip->pan.speed = PAN_SPEED_1;
	chip->rising.step =	0;
	chip->rising.degree = 0;
	chip->rising.speed = RISING_SPEED_1;

	// register the driver data -> chip 
	platform_set_drvdata(pdev,chip);
	// register sysfs groups
	ret = sysfs_create_group(&pdev->dev.kobj, &motor_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "Failed to create sysfs group\n");
		ifs_timer_unregister(chip->time.handle);
		platform_set_drvdata(pdev, NULL);
		return ret;
	}

	PR_DEBUG("Finished Motor probe\n");
    return 0;
}

static int drv_motor_remove(struct platform_device *pdev)
{
    struct motor_chip *chip = platform_get_drvdata(pdev);
    if (!chip){
		return 0; 
	}
	sysfs_remove_group(&pdev->dev.kobj, &motor_attr_group);
	if (chip->time.handle) {
        ifs_timer_unregister(chip->time.handle);
        chip->time.handle = NULL; 
    }
	platform_set_drvdata(pdev, NULL);
	
	return 0;
}

static const struct of_device_id motor_of_match[] = {
   { .compatible = "awinic,motor", },
    { },
};

MODULE_DEVICE_TABLE(of, motor_of_match);

static struct platform_driver motor_driver = {
    .probe = drv_motor_probe,
    .remove = drv_motor_remove,
    .driver = {
        .name = "motor_driver",
        .of_match_table = motor_of_match,
    },
};


module_platform_driver(motor_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Willis");
MODULE_DESCRIPTION("Motor Control Driver");
