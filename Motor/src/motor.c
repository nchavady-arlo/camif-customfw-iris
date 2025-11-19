/*=====================================*/
/* Kernel lib */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
/*=====================================*/
/* Kenrel timer lib*/
#include <linux/timer.h>
#include <linux/jiffies.h>
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


extern struct timer_list timer;
extern u8 m_isTimeout;

struct motor_control{
	u8	motor;							//pan:0,rising:1 
	u8  mode;							//Step:0,Degree:1		// delete
	bool en;							//disable:0,enable:1
	u8	dir;							//forward:0,reverse:1
	u32	step;							//step					// delete
	u32 degree;							//degree				// delete
	u32 delay;							//step delay
	u32 duration;						//duration for timer
	u8 speed;							//speed	
};
	
struct motor_chip{
	struct device			*dev;
	struct motor_control	mot;	
	
	struct work_struct		work_motor;
	struct mutex			lock;
};


static void motor_timer_callback(struct timer_list *t)
{
    LOG("Motor Stop!!\n");
    m_isTimeout = 1;
}

static ssize_t motor_version(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "%s", AW8646_DRIVER_VERSION);
};

static ssize_t motor_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->mot.motor)
		val="Motor: Rising\n";
	else
		val="Motor: Pan\n";
	
	return sprintf(buf, "%s", val);
};

static ssize_t motor_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	
	mutex_lock(&chip->lock);
	ret = kstrtouint(buf, 10, &val);
	
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	if(val)
		chip->mot.motor = AW_RISING_MOTOR;
	else
		chip->mot.motor = AW_PAN_MOTOR;
		
	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t mode_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->mot.mode==AW_STEP_MODE)
		val="Mode: Step\n";
	else if(chip->mot.mode==AW_DEGREE_MODE)
		val="Mode: Degree\n";
	else if(chip->mot.mode==AW_TIMER_MODE)
		val="Mode: Timer\n";
	else 
		val="Error\n";
	
	return sprintf(buf, "%s", val);
};

static ssize_t dir_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->mot.dir)
		val="Dir: Forward\n";
	else
		val="Dir: Reverse\n";
	
	return sprintf(buf, "%s", val);
};

static ssize_t dir_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);

	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	if(val)
		chip->mot.dir =AW_FORWARD;
	else
		chip->mot.dir =AW_REVERSE;

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t en_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	if(chip->mot.en)
		val="Motor: Enable\n";
	else
		val="Motor: Disable\n";
	
	return sprintf(buf, "%s", val);
};


static ssize_t degree_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;

	len= sprintf(buf, "degree : %u\n", chip->mot.degree);
	
	return len;
};

static ssize_t degree_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	if(val<65535)	//Max : 0xFFFF
		chip->mot.degree =(u32)val;
	else
		chip->mot.degree =0;

	mutex_unlock(&chip->lock);
	return size;  
};


static ssize_t step_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;

	len= sprintf(buf, "Step count: %u\n", chip->mot.step);
	
	return len;
};

static ssize_t step_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	if(val<65535)	//Max : 0xFFFF
		chip->mot.step =(u32)val;
	else
		chip->mot.step =0;

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t delay_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;

	len= sprintf(buf, "Delay: %u us\n", chip->mot.delay);
	
	return len;
};


static ssize_t duration_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;

	len= sprintf(buf, "duration : %u ms\n", chip->mot.duration);
	
	return len;
};

static ssize_t duration_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	if(val<65535) //max: 0xffff
		chip->mot.duration =(u32)val;
	else
		chip->mot.duration =0;

	mutex_unlock(&chip->lock);
	return size;  
};
static ssize_t speed_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;

	len= sprintf(buf, "Speed: %u\n", chip->mot.speed);
	
	return len;
};

static ssize_t speed_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val=0;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	if(val<10)
		chip->mot.speed =(u8)val;
	else
		chip->mot.speed =0;
	chip->mot.delay =awd8833c_set_motor_speed(chip->mot.speed);
	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t motor_info(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;
	char *motor;
	char *mode;
	char *en;
	char *dir;

	mutex_lock(&chip->lock);
	
	if(chip->mot.motor)
		motor="Rising";
	else
		motor="Pan";
	
	if(chip->mot.mode==AW_STEP_MODE)
		mode="Step";
	else if(chip->mot.mode==AW_DEGREE_MODE)
		mode="Degree";	
	else if(chip->mot.mode==AW_TIMER_MODE)
		mode="Timer";	
	
	if(chip->mot.en)
		en="Enable";
	else
		en="Disable";
	
	if(chip->mot.dir)
		dir="Forward";
	else
		dir="Reverse";
	
	len= sprintf(buf,
			"==============================\n" 
			"Motor Attr:\nMotor: %s\nMode:%s\n"
			"Enable: %s\nDirection: %s\nStep count: %u\n"
			"Degree: %u\nDuration: %u ms\n"
			"Delay: %u us\nSpeed: %u\n"
			"==============================\n" 
			,motor,mode,en,dir,chip->mot.step,chip->mot.degree,chip->mot.duration,chip->mot.delay,chip->mot.speed);
	
	mutex_unlock(&chip->lock);
	return len;
};

static void motorctrl_work(struct work_struct *work)
{
	struct motor_chip *chip = container_of(work, struct motor_chip, work_motor);	
	
	LOG("In MotorCtrl work\n");
	
	switch(chip->mot.mode)
	{
		case AW_STEP_MODE:
			LOG("Step Moden\n");
			awd8833c_motor_launch_by_step(chip->mot.motor,chip->mot.dir,chip->mot.speed,chip->mot.step);
			chip->mot.en=AW_FALSE;
			
			break;
		case AW_DEGREE_MODE:
			LOG("Degree Moden\n");
			awd8833c_motor_launch_by_degree(chip->mot.motor,chip->mot.dir,chip->mot.speed,chip->mot.degree);
			chip->mot.en=AW_FALSE;
			break;
		
		case AW_TIMER_MODE:
			LOG("Timer Moden\n");
			awd8833c_motor_launch_by_time(chip->mot.motor,chip->mot.dir,chip->mot.speed,chip->mot.duration);
			chip->mot.en=AW_FALSE;

			break;
				
		default:
			
			break;
	}
	
	//disable all motor
	awd8833c_gpio_init();
}

static ssize_t start_by_step_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	chip->mot.en =		AW_TRUE;
	chip->mot.mode =	AW_STEP_MODE;
	
	LOG("Motor Attr: \n"
		"==============================\n" 
		"Motor: %u\nMode: %u\n"
		"Direction: %u\nStep count: %u\n"
		"Speed: %u\n"
		"==============================\n" 
		,chip->mot.motor,chip->mot.mode,chip->mot.dir,chip->mot.step,chip->mot.speed);

	schedule_work(&chip->work_motor);
	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t start_by_degree_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	chip->mot.en =		AW_TRUE;
	chip->mot.mode =	AW_DEGREE_MODE;
	
	LOG("Motor Attr: \n"
		"==============================\n" 
		"Motor: %u\nMode: %u\n"
		"Direction: %u\nDegree: %u\n"
		"Speed: %u\n"
		"==============================\n" 
		,chip->mot.motor,chip->mot.mode,chip->mot.dir,chip->mot.degree,chip->mot.speed);

	schedule_work(&chip->work_motor);
	mutex_unlock(&chip->lock);
	return size;  
};
static ssize_t start_by_timer_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret;
	unsigned int val;
	struct motor_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	
	ret = kstrtouint(buf, 10, &val);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	chip->mot.en =		AW_TRUE;
	chip->mot.mode = 	AW_TIMER_MODE;

	LOG("Motor Attr: \n"
		"==============================\n" 
		"Motor: %u\nMode: %u\n"
		"Direction: %u\nDuration : %u ms\nSpeed: %u\n"
		"==============================\n" 
		,chip->mot.motor,chip->mot.mode,chip->mot.dir,chip->mot.duration,chip->mot.speed);
	
	schedule_work(&chip->work_motor);
	//schedule_work(&chip->work_motor);
	mutex_unlock(&chip->lock);
	return size;  
};

static DEVICE_ATTR(version,0664,motor_version,NULL);
static DEVICE_ATTR(info,0664,motor_info,NULL);
static DEVICE_ATTR(motor,0664,motor_show,motor_store);
static DEVICE_ATTR(mode,0664,mode_show,NULL);
static DEVICE_ATTR(enable,0664,en_show,NULL);
static DEVICE_ATTR(dir,0664,dir_show,dir_store);
static DEVICE_ATTR(step,0664,step_show,step_store);
static DEVICE_ATTR(degree,0664,degree_show,degree_store);
static DEVICE_ATTR(delay,0664,delay_show,NULL);
static DEVICE_ATTR(duration,0664,duration_show,duration_store);
static DEVICE_ATTR(speed,0664,speed_show,speed_store);
static DEVICE_ATTR(start_by_step,0664,NULL,start_by_step_store);
static DEVICE_ATTR(start_by_degree,0664,NULL,start_by_degree_store);
static DEVICE_ATTR(start_by_timer,0664,NULL,start_by_timer_store);
//static DEVICE_ATTR(stop,0664,NULL,all_stop_store);


static struct attribute *motor_attrs[] = {
	&dev_attr_version.attr,
	&dev_attr_info.attr,
	&dev_attr_motor.attr,
	&dev_attr_mode.attr,
	&dev_attr_enable.attr,
	&dev_attr_dir.attr,
	&dev_attr_step.attr,
	&dev_attr_degree.attr,
	&dev_attr_duration.attr,
	&dev_attr_speed.attr,
	&dev_attr_delay.attr,
	//&dev_attr_test.attr,
	&dev_attr_start_by_step.attr,
	&dev_attr_start_by_degree.attr,
	&dev_attr_start_by_timer.attr,
    NULL,
};

static const struct attribute_group motor_attr_group = {
    .attrs = motor_attrs,
};


static int motor_probe(struct platform_device *pdev)
{
	struct motor_chip *chip;
	int ret;
	
	/* init GPIO */
	if(Motor_gpio_init(pdev))
	{
		dev_err(&pdev->dev, "GPIO Init Error\n");
		return 1;
	}

	/* init timer */
    timer_setup(&timer, motor_timer_callback, 0);

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		dev_err(&pdev->dev, "%s: failed to allocate driver data.\n", __func__);
		return -ENOMEM;
	}

	chip->dev = &pdev->dev;	
	/* init mutex */
	mutex_init(&chip->lock);
	/* init attr */
	chip->mot.motor =	AW_PAN_MOTOR;
	chip->mot.en =		AW_FALSE;
	chip->mot.mode =	AW_STEP_MODE;
	chip->mot.dir =		AW_FORWARD;
	chip->mot.step =	0;
	chip->mot.speed =	0;
	chip->mot.delay =	awd8833c_set_motor_speed(chip->mot.speed);

	INIT_WORK(&chip->work_motor, motorctrl_work);

	platform_set_drvdata(pdev,chip);
	
	ret = sysfs_create_group(&pdev->dev.kobj, &motor_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "Failed to create sysfs group\n");
		return ret;
	}
	
	LOG("Finished Motor probe\n");
    return 0;
}

static int motor_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &motor_attr_group);
    del_timer_sync(&timer);			//Remove the timer
	
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
