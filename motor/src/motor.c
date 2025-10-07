#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>

#include "pega_gpio.h"
#include "pega_motor_awd8833.h"

#define  DEBUG_EN

#ifdef DEBUG_EN
	#define	LOG(fmt,args...)	printk(KERN_ERR "[%s]::"  fmt,__func__,## args);
#else 
	#define LOG(fmt, args...) 
#endif

struct motor_control{
	u8	motor;							//pan:0,rising:1 
	u8  mode;							//Step:0,Degree:1
	bool en;							//disable:0,enable:1
	u8	dir;							//forward:0,reverse:1
	u32	cycle;							//cycle
	u32 degree;							//degree
	u32 delay;							//step delay
	u8 sp;								//speed	
};
	
struct motor_chip{
	struct device			*dev;
	struct motor_control	mot;	
	
	struct work_struct		work_motor;
	//struct work_struct		rising_motor;	
	struct mutex			lock;
};


static ssize_t motor_version(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "%s", AW8646_DRIVER_VERSION);
};

static ssize_t motor_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	mutex_lock(&chip->lock);
	if(chip->mot.motor)
		val="Motor: Rising\n";
	else
		val="Motor: Pan\n";
	
	mutex_unlock(&chip->lock);
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
		chip->mot.motor = AW_PAN_MOTOR;
	else
		chip->mot.motor = AW_RISING_MOTOR;
		
	mutex_unlock(&chip->lock);
	return size;  
};


static ssize_t mode_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;

	mutex_lock(&chip->lock);
	if(chip->mot.mode)
		val="Mode: Step\n";
	else
		val="Mode: Degree\n";
	
	mutex_unlock(&chip->lock);
	return sprintf(buf, "%s", val);
};

static ssize_t mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
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
		chip->mot.mode =1;
	else
		chip->mot.mode =0;
		
	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t dir_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;
	mutex_lock(&chip->lock);

	if(chip->mot.dir)
		val="Dir: Forward\n";
	else
		val="Dir: Reverse\n";
	
	mutex_unlock(&chip->lock);
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
		chip->mot.dir =1;
	else
		chip->mot.dir =0;

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t en_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	char *val;
	mutex_lock(&chip->lock);

	if(chip->mot.en)
		val="Motor: Enable\n";
	else
		val="Motor: Disable\n";
	
	mutex_unlock(&chip->lock);
	return sprintf(buf, "%s", val);
};

static ssize_t en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
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
		chip->mot.en =1;
	else
		chip->mot.en =0;

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t cycle_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;
	mutex_lock(&chip->lock);

	len= sprintf(buf, "cycle count: %u\n", chip->mot.cycle);
	
	mutex_unlock(&chip->lock);
	return len;
};

static ssize_t cycle_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
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
	
	if(val<1000)
		chip->mot.cycle =(u32)val;
	else
		chip->mot.cycle =0;

	mutex_unlock(&chip->lock);
	return size;  
};

static ssize_t delay_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;
	mutex_lock(&chip->lock);

	len= sprintf(buf, "Delay: %u\n", chip->mot.delay);
	
	mutex_unlock(&chip->lock);
	return len;
};

static ssize_t sp_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct motor_chip *chip = dev_get_drvdata(dev);
	ssize_t len;
	mutex_lock(&chip->lock);

	len= sprintf(buf, "Speed: %u\n", chip->mot.sp);
	
	mutex_unlock(&chip->lock);
	return len;
};

static ssize_t sp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
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
		chip->mot.sp =(u8)val;
	else
		chip->mot.sp =0;
	chip->mot.delay =awd8833c_set_motor_speed(chip->mot.sp);
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
	
	if(chip->mot.mode)
		mode="Step";
	else
		mode="Degree";	
	if(chip->mot.en)
		en="Enable";
	else
		en="Disable";
	
	if(chip->mot.dir)
		dir="Forward";
	else
		dir="Reverse";
	
	len= sprintf(buf,
			"===================================\n" 
			"Motor Attr:\nMotor: %s\nMode:%s"
			"Enable: %s\nDirection: %s\nCycle count: %u\n"
			"Delay: %u\nSpeed: %u\n"
			"===================================\n" 			
			,motor,mode,en,dir,chip->mot.cycle,chip->mot.delay,chip->mot.sp);
	
	mutex_unlock(&chip->lock);
	return len;
};

static void motorctrl_work(struct work_struct *work)
{
	struct motor_chip *chip = container_of(work, struct motor_chip, work_motor);	
	
	LOG("In MotorCtrl work\n");
	awd8833c_gpio_init();
	awd8833c_set_enable(chip->mot.en, chip->mot.motor);
	awd8833c_set_motor_speed(chip->mot.sp);
	awd8833c_step_function(chip->mot.dir, AW_FULL_STEP, chip->mot.cycle);
	/*
	switch(chip->mot.mode)
	{
		case STEP_MODE:
			awd8833c_gpio_init();
			awd8833c_set_enable(chip->mot.en, chip->mot.motor);
			awd8833c_set_motor_speed(chip->mot.sp);
			awd8833c_step_function(chip->mot.dir, AW_FULL_STEP, chip->mot.cycle);
			
			break;
		case DEG_MODE:
				
			break;
				
		default:
			
			break;
	}
	*/
	//disable all motor
	awd8833c_set_enable(0,0);
	awd8833c_set_enable(0,1);
}

static ssize_t step_on_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
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
	LOG("====================================\n"
		"Motor Attr: \nMotor: %u\nMode: %u\n"
		"Enable: %u\nDirection: %u\nCycle count: %u\n"
		"Speed: %u\n"
		"====================================\n"
		,chip->mot.motor,chip->mot.mode,chip->mot.en,chip->mot.dir,chip->mot.cycle,chip->mot.sp);
	
	schedule_work(&chip->work_motor);
	mutex_unlock(&chip->lock);
	return size;  
};

static DEVICE_ATTR(version,0664,motor_version,NULL);
static DEVICE_ATTR(info,0664,motor_info,NULL);
static DEVICE_ATTR(motor,0664,motor_show,motor_store);
static DEVICE_ATTR(mode,0664,mode_show,mode_store);
static DEVICE_ATTR(enable,0664,en_show,en_store);
static DEVICE_ATTR(dir,0664,dir_show,dir_store);
static DEVICE_ATTR(cycle,0664,cycle_show,cycle_store);
static DEVICE_ATTR(delay,0664,delay_show,NULL);
static DEVICE_ATTR(speed,0664,sp_show,sp_store);
static DEVICE_ATTR(step_work,0664,NULL,step_on_store);
//static DEVICE_ATTR(stop,0664,NULL,all_stop_store);



static struct attribute *motor_attrs[] = {
	&dev_attr_version.attr,
	&dev_attr_info.attr,
	&dev_attr_motor.attr,
	&dev_attr_mode.attr,
	&dev_attr_enable.attr,
	&dev_attr_dir.attr,
	&dev_attr_cycle.attr,
	&dev_attr_delay.attr,
	&dev_attr_speed.attr,
	//&dev_attr_test.attr,
	&dev_attr_step_work.attr,
    NULL,
};

static const struct attribute_group motor_attr_group = {
    .attrs = motor_attrs,
};


static int motor_probe(struct platform_device *pdev)
{
	struct motor_chip *chip;
	int ret;
	
	if(Motor_gpio_init(pdev))
	{
		LOG("error\n");
		return 1;
	}

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		dev_err(&pdev->dev, "%s: failed to allocate driver data.\n", __func__);
		return -ENOMEM;
	}

	chip->dev = &pdev->dev;	
	/* init mutex */
	mutex_init(&chip->lock);
	/* init attr */
	chip->mot.motor=0;						//pan or rising
	chip->mot.en=0;							//enable
	chip->mot.dir=0;						//direction
	chip->mot.cycle=0;						//cycleation
	chip->mot.delay=0;						//step delay
	chip->mot.sp=0;							//speed

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
