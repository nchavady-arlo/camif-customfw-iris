#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/bitops.h>
#include <linux/random.h>
//==============================================================================	
#define ktd2058_NAME "leds-ktd2058"
//==============================================================================
#define KTD2058_DRIVER_VER	    "0.01"
//==============================================================================
//#define KTD_DEBUG
#ifdef KTD_DEBUG
#define LOG_DBG(fmt, args...)   printk(KERN_ERR "[%s]:: " fmt, __func__, ## args);
#else 
#define LOG_DBG(fmt, args...) 
#endif
//==============================================================================
#define DBG_INFO(fmt, args...)   printk(KERN_ERR "[%s]:: " fmt, __func__, ## args);
//==============================================================================
#define REG_ID						0x00
#define REG_MONITOR					0x01
#define REG_CONTROL					0x02
#define REG_IRED0					0x03
#define REG_IGRN0					0x04
#define REG_IBLU0					0x05
#define REG_IRED1					0x06
#define REG_IGRN1					0x07
#define REG_IBLU1					0x08
#define REG_ISELA12					0x09
#define REG_ISELA34					0x0A
#define REG_ISELB12					0x0B
#define REG_ISELB34					0x0C
#define REG_ISELC12					0x0D
#define REG_ISELC34					0x0E

enum ktd2058_mode{
	GLOBAL_OFF = 0x00,
	NIGHT_MODE = 0x01,
	NORMAL_MODE = 0x02,
	DEFAULT = 0x03,
};

enum ktd2058_br_level{
	BR_OFF = 0x00,
	BR_NORMAL,
	BR_2COLORS, 
	BR_LOGSCALE,
	BR_DEMO,
	BR_FLOWERS,
	BR_LOGSCALE_DEMO,
	BR_White,
	STOP
};

enum ktd2058_chase_level{
	CHASE_OFF =0x00,
	CHASEUP_1, 
	CHASEDOWN_1,
	CHASEUP_2,
	CHASEDOWN_2,
	CHASEUP_3,
	CHASEDOWN_3,
	CHASEUP_4,
	CHASEDOWN_4,
	CHASE_2COLORS_UP,
	CHASE_2COLORS_DOWN,
	CHASE_DEMO,
	CHASE_AMZ,
};

struct ktd2058_platform_data {
	unsigned int brightextend;		//Brightness Extend
	unsigned int ce_temp;			//Chip Enable Temperature
	unsigned int fade_rate;			
	unsigned int en_mode; 
};

struct ktd2058_breath_ops {
	enum ktd2058_br_level	level;
	u8						fade_on;
	u8						fade_off;
	int						time_on;
	int						time_off;	
	int						en;
	int						cnt;
	int						max_cnt;		
};

struct ktd2058_chase_ops{
	enum ktd2058_chase_level level;
	u8						fade;
	int						delay;
	int						cnt;
	int						max_cnt;
};

struct ktd2058_color{	
	u8						enPattern;
	u8						u8Brightness;
};

struct ktd2058_chip {
	struct device					*dev;
	struct i2c_client				*client;

	struct led_classdev				cdev_flash;

	struct work_struct				work_chase;
	struct work_struct				work_breathing;	
	
	struct ktd2058_platform_data	*pdata;
	struct mutex					lock;

	struct ktd2058_breath_ops		br_ops;
	struct ktd2058_chase_ops		chase_ops;
	struct ktd2058_color			color;
};
//==============================================================================
static void ktd2058_color_pattern_set(struct ktd2058_chip *chip);
//==============================================================================
static int ktd2058_write_reg(struct i2c_client *client, int reg, u8 value)
{
    int ret;
		
    ret = i2c_smbus_write_byte_data(client, reg, value);

    if(ret < 0)
        dev_err(&client->dev, "%s: err %d\n", __func__, ret);
    return ret;
}

static int ktd2058_read_reg(struct i2c_client *client, int reg, u8 *val)
{
    int ret;
	ret = i2c_smbus_read_byte_data(client, reg);

	if(ret < 0){
        dev_err(&client->dev, "%s: err %d\n", __func__, ret);
        return ret;
    }else{
		*val = ret;
    }

    LOG_DBG("Reading 0x%02x=0x%02x\n", reg, *val);

	return ret;
}

static int ktd2058_masked_write(struct i2c_client *client, int reg, u8 mask, u8 val)
{
    int rc;
    u8 temp;

    rc = ktd2058_read_reg(client, reg, &temp);
    if(rc < 0){
		dev_err(&client->dev, "failed to read reg=0x%x, rc=%d\n", reg, rc);
    }	
	else{
        temp &= ~mask;
        temp |= val & mask;
        rc = ktd2058_write_reg(client, reg, temp);
        if(rc<0){
            dev_err(&client->dev,
                           "failed to write masked data. reg=%03x, rc=%d\n", reg, rc);
        }
    }
 
    ktd2058_read_reg(client, reg, &temp);
	return rc;
}
//==============================================================================
static int ktd_pow(int a, int b)
{
	int i, c;
	c = a;
	
	for (i = 1; i<b; i++){
		c *= a;
		//LOG_DBG("c =%d\n", c);
	}
	return c;
}

static void ktd_parse_dt(struct device *dev, struct ktd2058_chip *chip)
{
	struct device_node *np = dev->of_node;
	struct ktd2058_platform_data *pdata = chip->pdata;
	int rc;
	u32 temp;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL); 
	if(!pdata){
		dev_err(dev,"failed to alloc pdata in parse dt!\n"); 
		return;
	}
	
	//need to read the property from DTSi. (iford.dtsi), modified by Aron.
	rc = of_property_read_u32(np, "brightextend", &temp);
    if(rc){
		pr_err("Invalid brightextend!\n");
    }else{
        pdata->brightextend = temp;
		LOG_DBG("brightextend --<0x%x> \n", pdata->brightextend)
    }

	rc = of_property_read_u32(np, "ce-temp", &temp);
    if(rc){
		pr_err("Invalid ce-temp!\n");
    }else{
        pdata->ce_temp = temp;
		LOG_DBG("ce_temp --<0x%x> \n", pdata->ce_temp)
    }

	rc = of_property_read_u32(np, "fade-rate", &temp);
    if(rc){
		pr_err("Invalid fade-rate!\n");
    }else{
        pdata->fade_rate = temp;
		LOG_DBG("fade_rate --<0x%x> \n", pdata->fade_rate)
    }

	dev->platform_data = pdata;

}

static void ktd2058_set_color0(struct ktd2058_chip *chip, u8 ired, u8 igreen, u8 iblue)
{
	//struct ktd2058_platform_data *pdata = chip->pdata;

	LOG_DBG("ired = 0x%x, igreen = 0x%x, iblue = 0x%x\n", ired, igreen, iblue);
	
	ktd2058_write_reg(chip->client, REG_IRED0, ired);
	ktd2058_write_reg(chip->client, REG_IGRN0, igreen);
	ktd2058_write_reg(chip->client, REG_IBLU0, iblue);
}

static void ktd2058_set_color1(struct ktd2058_chip *chip, u8 ired, u8 igreen, u8 iblue)
{
	//struct ktd2058_platform_data *pdata = chip->pdata;

	LOG_DBG("ired = 0x%x, igreen = 0x%x, iblue = 0x%x\n", ired, igreen, iblue);
	
	ktd2058_write_reg(chip->client, REG_IRED1, ired);
	ktd2058_write_reg(chip->client, REG_IGRN1, igreen);
	ktd2058_write_reg(chip->client, REG_IBLU1, iblue);
}

static void ktd2058_select_all_colors(struct ktd2058_chip *chip, u8 isela12, u8 isela34, u8 iselb12, 
											u8 iselb34, u8 iselc12, u8 iselc34, int delay)
{
    ktd2058_write_reg(chip->client, 0x09, isela12);
    ktd2058_write_reg(chip->client, 0x0A, isela34);
    ktd2058_write_reg(chip->client, 0x0B, iselb12);
    ktd2058_write_reg(chip->client, 0x0C, iselb34);
    ktd2058_write_reg(chip->client, 0x0D, iselc12);
    ktd2058_write_reg(chip->client, 0x0E, iselc34);	
	msleep(delay);
}

static void ktd2058_select_color(struct ktd2058_chip *chip, int color, u8 sel)
{
	u8 isel;
	int i;

    switch(color)
	{
	   case 0:
	          isel = 0x88; //color0
              break;
	   case 1:
	          isel = 0xFF; //color1	
              break;   
       case 3:
	          isel = 0x00; //OFF
              break;
 	   case 4:
	          isel = sel;
              break;
			  
	   default:
	          isel = 0x00; //OFF
			  LOG_DBG("invalid value(%d)!\n",color);
              break;		  
	}
	
	
	for (i = 0x09; i <=0x0F; i++){
		LOG_DBG("color = 0x%x, reg = 0x%x, isel = 0x%x\n", color, i, isel);
		ktd2058_write_reg(chip->client, i, isel);		
	}	
	
}

static void ktd2058_set_random_palette(struct ktd2058_chip *chip, u8 imax)
{
	int i;
	u8 iset, rand;

	for(i = 0x03; i < 0x09; i++){
		get_random_bytes(&rand, sizeof(rand));
		//LOG_DBG("rand = %d\n", rand);
		rand %= 5;
		iset = DIV_ROUND_UP(imax/(2 << rand),1);
		//LOG_DBG("rand = %d, iset = 0x%x\n", rand, iset);
		ktd2058_write_reg(chip->client, i, iset);
	}
}

static void ktd2058_select_one(struct ktd2058_chip *chip, u8 reg, u8 data, int delay)
{
	ktd2058_write_reg(chip->client, reg, data);
	msleep(delay);
}

static void ktd2058_mode_change(struct ktd2058_chip *chip, int mode, u8 fade)
{
	struct ktd2058_platform_data *pdata = chip->pdata;
	u8 value = 0;
	
	value = (pdata->brightextend << 5) | (pdata->ce_temp << 3)| fade;
	pdata->fade_rate = fade;

	switch (mode){
		case GLOBAL_OFF:
			LOG_DBG("global off == 0x%x\n", value);
			ktd2058_write_reg(chip->client, REG_CONTROL, value);	
			break;
		case NIGHT_MODE:
			value = 0x40 | value;
			LOG_DBG("Night mode == 0x%x\n", value);
			ktd2058_write_reg(chip->client, REG_CONTROL, value);
			break;
		case NORMAL_MODE:
			value = 0x80 | value;
			LOG_DBG("Normal Mode == 0x%x\n", value);
			ktd2058_write_reg(chip->client, REG_CONTROL, value);
			break;
		case DEFAULT:
			value = 0x07 | value;
			LOG_DBG("Default == 0x%x\n", value);
			ktd2058_write_reg(chip->client, REG_CONTROL, value);
			pdata->fade_rate = 0x07;			
			break;
		default:
			LOG_DBG("nothing selected!!");
			break;
	}
}

/******************************************************************************************
    # Color Palette Library Function
    # Displays color0 on A1-3, color1 on C2-4, and rest of palette on A4-C1
    # Adjust color 0 and color 1 to see different palettes

*******************************************************************************************/
static void ktd2058_util_color_palette(struct ktd2058_chip *chip, u8 fade, int delay)
{
    ktd2058_mode_change(chip, DEFAULT, 0x00/*Don't care*/);

    ktd2058_set_color0(chip, 0x00, 0x00, 0x00);                // color0 = black, adjust for other palettes
    ktd2058_set_color1(chip, 0x80, 0x80, 0x80);                // color1 = white, adjust for other palettes
    ktd2058_select_all_colors(chip, 0x88, 0x89, 0xAB, 0xCD, 0xEF, 0xFF, 0);
    ktd2058_mode_change(chip, NORMAL_MODE, fade);
    msleep(delay);		
}

/*******************************************************************************************
# Chasing Library Functions
********************************************************************************************/
enum chase_state{
	UP = 0x00,
	DOWN,
};

static void ktd2058_chase_one(struct ktd2058_chip *chip, int state,  u8 fade, int delay)
{
	int i;

	ktd2058_mode_change(chip, NORMAL_MODE, fade);

	LOG_DBG("CHASE_one, state = %d, delay = %d, fade = 0x%x\n", state, delay, fade);
	if(state == UP){
		for (i = 0x09; i <=0x0F; i++){
			ktd2058_select_one(chip, i, 0x80, delay);
			ktd2058_select_one(chip, i, 0xF8, delay);
			ktd2058_select_one(chip, i, 0xFF, 0);
		}
	}else if (state == DOWN){
		for(i = 0x0E; i > 0x08; i--){
			ktd2058_select_one(chip, i, 0x08, delay);
			ktd2058_select_one(chip, i, 0x8F, delay);
			ktd2058_select_one(chip, i, 0xFF, 0);
		}
	}
}

static void ktd2058_chase_two(struct ktd2058_chip *chip, int state, u8 fade, int delay)
{
	int i;

	ktd2058_mode_change(chip, NORMAL_MODE, fade);

	LOG_DBG("CHASE_two, state = %d, delay = %d, fade = 0x%x\n", state, delay, fade);
	if(state == UP){
		for(i = 0x09; i<0x0C; i++){
			ktd2058_select_one(chip, i, 0x80, 0);
            ktd2058_select_one(chip, i+3, 0x80, delay);
       	    ktd2058_select_one(chip, i, 0xF8, 0);
           	ktd2058_select_one(chip, i+3, 0xF8, delay);
           	ktd2058_select_one(chip, i, 0xFF, 0);
           	ktd2058_select_one(chip, i+3, 0xFF, 0);
		}
	}
	else if (state == DOWN){
		for(i = 0x0E; i > 0x0B; i--){
			ktd2058_select_one(chip, i, 0x08, 0);
            ktd2058_select_one(chip, i-3, 0x08, delay);
			ktd2058_select_one(chip, i, 0x8F, 0);
			ktd2058_select_one(chip, i-3, 0x8F, delay);
			ktd2058_select_one(chip, i, 0xFF, 0);
			ktd2058_select_one(chip, i-3, 0xFF, 0);
		}
	}
}

static void ktd2058_chase_three(struct ktd2058_chip *chip, int state, u8 fade, int delay)
{
	int i;

	ktd2058_mode_change(chip, NORMAL_MODE, fade);
	LOG_DBG("CHASE_3, state = %d, delay = %d, fade = 0x%x\n", state, delay, fade);
	if(state == UP){
		for(i = 0x09; i<0x0B; i++){
	        LOG_DBG("chase up 3, delay = %d, fade = 0x%x\n", delay, fade);
	        ktd2058_select_one(chip, i, 0x80, 0);
		    ktd2058_select_one(chip, i+2, 0x80, 0);
			ktd2058_select_one(chip, i+4, 0x80, delay);
            ktd2058_select_one(chip, i, 0xF8, 0);
	        ktd2058_select_one(chip, i+2, 0xF8, 0);
		    ktd2058_select_one(chip, i+4, 0xF8, delay);
			ktd2058_select_one(chip, i, 0xFF, 0);
            ktd2058_select_one(chip, i+2, 0xFF, 0);
	        ktd2058_select_one(chip, i+4, 0xFF, 0);
		}
	}else if (state == DOWN){
		for(i = 0x0E; i > 0x0C; i--){
			ktd2058_select_one(chip, i, 0x08, 0);
			ktd2058_select_one(chip, i-2, 0x08, 0);
			ktd2058_select_one(chip, i-4, 0x08, delay);
			ktd2058_select_one(chip, i, 0x8F, 0);
			ktd2058_select_one(chip, i-2, 0x8F, 0);
			ktd2058_select_one(chip, i-4, 0x8F, delay);
			ktd2058_select_one(chip, i, 0xFF, 0);
			ktd2058_select_one(chip, i-2, 0xFF, 0);
			ktd2058_select_one(chip, i-4, 0xFF, 0);
		}
	}
}

static void ktd2058_chase_four(struct ktd2058_chip *chip, int state, u8 fade, int delay)
{
	ktd2058_mode_change(chip, NORMAL_MODE, fade);
	LOG_DBG("CHASE_4, state = %d, delay = %d, fade = 0x%x\n", state, delay, fade);
	if(state == UP){
		ktd2058_select_all_colors(chip, 0x8F, 0xF8, 0xFF, 0x8F, 0xF8, 0xFF, delay);
	    ktd2058_select_all_colors(chip, 0xF8, 0xFF, 0x8F, 0xF8, 0xFF, 0x8F, delay);
		ktd2058_select_all_colors(chip, 0xFF, 0x8F, 0xF8, 0xFF, 0x8F, 0xF8, delay);
	}else if (state == DOWN){
        ktd2058_select_all_colors(chip, 0xFF, 0x8F, 0xF8, 0xFF, 0x8F, 0xF8, delay);
	    ktd2058_select_all_colors(chip, 0xF8, 0xFF, 0x8F, 0xF8, 0xFF, 0x8F, delay);
		ktd2058_select_all_colors(chip, 0x8F, 0xF8, 0xFF, 0x8F, 0xF8, 0xFF, delay);
	}
}

static void ktd2058_chase_2colors(struct ktd2058_chip *chip, int state, u8 fade, int delay)
{
	 int i;

	ktd2058_mode_change(chip, NORMAL_MODE, fade);
	LOG_DBG("CHASE 2colors, state = %d, delay = %d, fade = 0x%x\n", state, delay, fade);
	if(state == UP){
		for(i = 0x09; i<0x0C; i++){           //# registers 0x09 to 0x0B
			ktd2058_select_one(chip, i, 0x80, 0);
		    ktd2058_select_one(chip, i+3, 0xF0, delay);
			ktd2058_select_one(chip, i, 0x08, 0);
			ktd2058_select_one(chip, i+3, 0x0F, delay);
			ktd2058_select_one(chip, i, 0x00, 0);
			ktd2058_select_one(chip, i+3, 0x00, 0);
		}
        for(i = 0x0C; i < 0x0F; i++){			//# registers 0x0C to 0x0E
		    ktd2058_select_one(chip, i, 0x80, 0);
			ktd2058_select_one(chip, i-3, 0xF0, delay);
			ktd2058_select_one(chip, i, 0x08, 0);
			ktd2058_select_one(chip, i-3, 0x0F, delay);
			ktd2058_select_one(chip, i, 0x00, 0);
			ktd2058_select_one(chip, i-3, 0x00, 0);
		}
	}else if (state == DOWN){
		for(i=0x0E; i>0x0B; i--){
			ktd2058_select_one(chip, i, 0x08, 0);
			ktd2058_select_one(chip, i-3, 0x0F, delay);
			ktd2058_select_one(chip, i, 0x80, 0);
		    ktd2058_select_one(chip, i-3, 0xF0, delay);
			ktd2058_select_one(chip, i, 0x00, 0);
            ktd2058_select_one(chip, i-3, 0x00, 0);
		}
		for(i=0x0B; i>0x08; i--){
            ktd2058_select_one(chip, i, 0x08, 0);
            ktd2058_select_one(chip, i+3, 0x0F, delay);
	        ktd2058_select_one(chip, i, 0x80, 0);
		    ktd2058_select_one(chip, i+3, 0xF0, delay);
			ktd2058_select_one(chip, i, 0x00, 0);
            ktd2058_select_one(chip, i+3, 0x00, 0);
		}

	}
}

static void ktd2058_chasing_demo(struct ktd2058_chip *chip)
{
	int fade; 
	int i = 0;

	ktd2058_select_color(chip, 3, 0); //select_off
	ktd2058_set_color0(chip, 0xC0, 0xC0, 0xC0);
	ktd2058_set_color1(chip, 0x00, 0x00, 0x00);
	for(fade=7; fade>0; fade--){
		LOG_DBG("fade =%d\n", fade);
		ktd2058_chase_one(chip, UP, fade, 83);
	}
	ktd2058_select_color(chip, 3, 0); //select_off
	ktd2058_set_color1(chip, 0x20, 0x00, 0x00);

    ktd2058_chase_one(chip, UP, 1, 83);          //            # cycles=3, fade=1, delay=83ms
    ktd2058_chase_one(chip, UP, 1, 83);          //            # cycles=3, fade=1, delay=83ms
    ktd2058_chase_one(chip, UP, 1, 83);          //            # cycles=3, fade=1, delay=83ms

	ktd2058_chase_one(chip, DOWN, 1, 83);
	ktd2058_chase_one(chip, DOWN, 1, 83);
	ktd2058_chase_one(chip, DOWN, 1, 83);

	ktd2058_chase_two(chip, DOWN, 1, 83);
	ktd2058_chase_two(chip, DOWN, 1, 83);
	ktd2058_chase_two(chip, DOWN, 1, 83);

    ktd2058_set_color0(chip, 0xC0, 0x00, 0x00);        //        # color0 = bright red
    ktd2058_set_color1(chip, 0x00, 0x00, 0x10);        //        # color1 = dim blue
    
	ktd2058_chase_two(chip, DOWN, 1, 83);
	ktd2058_chase_two(chip, DOWN, 1, 83);
	ktd2058_chase_two(chip, DOWN, 1, 83);

    ktd2058_select_color(chip, 3, 0);
    ktd2058_set_color1(chip, 0x00, 0x00, 0xC0);        //  # color1 = bright blue
	while(i < 8){
		ktd2058_chase_2colors(chip, DOWN, 2, 83);           //  # cycles=8, fade=2, delay=83ms
		LOG_DBG("index = %d\n", i);
		i++;
	}
    
	ktd2058_select_color(chip, 3, 0);					// select_off()
    ktd2058_set_color0(chip, 0xC0, 0xC0, 0x00);       //         # color0 = bright yellow
    ktd2058_set_color1(chip, 0x00, 0x00, 0x10);       //         # color1 = dim blue
    
	ktd2058_chase_two(chip, DOWN, 1, 83);              //      # cycles=3, fade=1, delay=83ms
    ktd2058_chase_two(chip, DOWN, 1, 83);              //      # cycles=3, fade=1, delay=83ms
    ktd2058_chase_two(chip, DOWN, 1, 83);              //      # cycles=3, fade=1, delay=83ms

	ktd2058_chase_three(chip, DOWN, 1, 83);           //         # cycles=3, fade=1, delay=83ms
	ktd2058_chase_three(chip, DOWN, 1, 83);           //         # cycles=3, fade=1, delay=83ms
	ktd2058_chase_three(chip, DOWN, 1, 83);           //         # cycles=3, fade=1, delay=83ms
    
	for(fade=0; fade<3; fade++){
        ktd2058_chase_four(chip, DOWN, fade, 83);     //        # cycles=1, fade, delay=83ms
	}

    for(fade=2; fade>=0; fade--){
        ktd2058_chase_four(chip, UP, fade, 83);       //        # cycles=1, fade, delay=83ms
	}

    ktd2058_chase_three(chip, UP, 0, 83);            //          # cycles=3, fade=0, delay=83ms
    ktd2058_chase_three(chip, UP, 0, 83);            //          # cycles=3, fade=0, delay=83ms
    ktd2058_chase_three(chip, UP, 0, 83);            //          # cycles=3, fade=0, delay=83ms

    ktd2058_chase_two(chip, UP, 1, 83);         //          # cycles=3, fade=1, delay=83ms
    ktd2058_chase_two(chip, UP, 1, 83);         //          # cycles=3, fade=1, delay=83ms
    ktd2058_chase_two(chip, UP, 1, 83);         //          # cycles=3, fade=1, delay=83ms

    for(fade=1; fade<8; fade++){
        ktd2058_chase_one(chip, UP, fade, 83);    //        # cycles=1, fade, delay=83ms
	}

   	ktd2058_select_color(chip, 3, 0);					// select_off()
}

static void ktd2058_chase_amz_demo(struct ktd2058_chip *chip)
{

    msleep(1700);//time.sleep(1.7)                             # startup delay (3.7s on some Dots)
    
	//# fade up to a dim blue
    ktd2058_set_color0(chip, 0x04, 0xC0, 0xC0);			//                # color0 = bright cyan
    ktd2058_set_color1(chip, 0x00, 0x00, 0x20);         //       # color1 = dim blue
    ktd2058_select_color(chip, 1, 0);
    ktd2058_mode_change(chip, NORMAL_MODE, 7);//global_on(7)
    msleep(3000);//time.sleep(3)
    
	//# chase cyan over dim blue
    ktd2058_chase_one(chip, UP,  3, 110);				//       # fade=3, delay=0.11s
    ktd2058_mode_change(chip, DEFAULT, 0);//DEFAULT()
   
	// # chase amber over black
    ktd2058_set_color0(chip, 0xC0, 0x48, 0x00);         //       # color0 = amber
	ktd2058_set_color1(chip, 0x00, 0x00, 0x00);         //       # color1 = black (off)
    ktd2058_chase_one(chip,  UP, 3, 110);				//       # fade=3, delay=0.11s
	ktd2058_mode_change(chip, GLOBAL_OFF, 3);
    msleep(30);//time.sleep(5/32*2**3)                       # delay of 5 time-constants
}

static void ktd2058_chase_ctrl_work(struct work_struct *work)
{
	struct ktd2058_chip *chip = container_of(work, struct ktd2058_chip, work_chase);	
	//int i, delay;
	
	LOG_DBG("start!\n");
	
	cancel_work_sync(&chip->work_breathing); //should cancel the other thread before sending command!

	switch(chip->chase_ops.level){
		case CHASEUP_1:
			ktd2058_chase_one(chip, UP, 3, 1500);
			break;

		case CHASEDOWN_1:
			ktd2058_chase_one(chip, DOWN, 3, 1500);
			break;

		case CHASEUP_2:	
			ktd2058_chase_two(chip, UP, 3, 1500);
			break;

		case CHASEDOWN_2:
			ktd2058_chase_two(chip, DOWN, 3, 1500);
			break;

		case CHASEUP_3:
			ktd2058_chase_three(chip, UP, 3, 1500);
			break;

		case CHASEDOWN_3:
			ktd2058_chase_three(chip, DOWN, 3, 1500);
			break;

		case CHASEUP_4:
			ktd2058_chase_four(chip, UP, 3, 1500);
			break;

		case CHASEDOWN_4:
			ktd2058_chase_four(chip, DOWN, 3, 1500);
			break;

		case CHASE_2COLORS_UP:
			ktd2058_chase_2colors(chip, UP, 3, 1500);
			break;

		case CHASE_2COLORS_DOWN:
			ktd2058_chase_2colors(chip, DOWN, 3, 1500);
			break;
		
		case CHASE_DEMO:
			ktd2058_chasing_demo(chip);
			break;
			
		case CHASE_AMZ:
			ktd2058_chase_amz_demo(chip);
			break;
		case CHASE_OFF:
		default:
			break;
	}
	
	if(chip->chase_ops.cnt < chip->chase_ops.max_cnt){
		chip->chase_ops.cnt++;
		schedule_work(&chip->work_chase);
	}else{
		chip->chase_ops.cnt = 0;
		chip->chase_ops.level = CHASE_OFF;
		ktd2058_mode_change(chip, DEFAULT, 0);
	}
	
	LOG_DBG("END!\n");
}

/******************************************************************************************
# Breathing Library Functions
******************************************************************************************/
static void ktd2058_breath(struct ktd2058_chip *chip, u8 fade_on, u8 fade_off, int timeon, int timeoff)
{
	LOG_DBG("Start!, fadeon = 0x%x, fadeoff = 0x%x, timeon = 0x%x, timeoff = 0x%x \n", fade_on, fade_off, timeon, timeoff);
	ktd2058_mode_change(chip, NORMAL_MODE, fade_on);
	msleep(timeon);
	ktd2058_mode_change(chip, GLOBAL_OFF, fade_off);
	msleep(timeoff);
}

static void ktd2058_breath_2colors(struct ktd2058_chip *chip, u8 fade_on, u8 fade_off, int timeon, int timeoff)
{
	LOG_DBG("Start!, fadeon = 0x%x, fadeoff = 0x%x, timeon = 0x%x, timeoff = 0x%x \n", fade_on, fade_off, timeon, timeoff);	
	//ktd2058_mode_change(chip, DEFAULT, 0);	
	ktd2058_select_color(chip, 0, 0);
	ktd2058_mode_change(chip, NORMAL_MODE, fade_on);
	msleep(timeon);
	//ktd2058_mode_change(chip, DEFAULT, 0);
	ktd2058_mode_change(chip, GLOBAL_OFF, fade_off);
	msleep(timeoff);
	ktd2058_select_color(chip, 1, 0);
	ktd2058_mode_change(chip, NORMAL_MODE, fade_on);
	msleep(timeon);
	//ktd2058_mode_change(chip, DEFAULT, 0);
	ktd2058_mode_change(chip, GLOBAL_OFF, fade_off);
	msleep(timeoff);
}

static void ktd2058_breath_demo(struct ktd2058_chip *chip)
{
	int i;
	u8 palette_list[7] = {0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

	LOG_DBG("start!\n")

	chip->br_ops.fade_on = 0x07;
	chip->br_ops.fade_off = 3;
	chip->br_ops.time_on = 1500;
	chip->br_ops.time_off = 1500;

	ktd2058_set_color0(chip, 0x00, 0x00, 0x00);
	ktd2058_set_color1(chip, 0xC0, 0xC0, 0xC0);

	for (i=0; i<sizeof(palette_list); i++){	
		LOG_DBG("index = %d\n", i);
		ktd2058_select_color(chip, 4, palette_list[i]);		
		ktd2058_breath(chip, chip->br_ops.fade_on, chip->br_ops.fade_off, chip->br_ops.time_on, chip->br_ops.time_off);
	}
	/* 
	ktd2058_select_all_colors(chip, 0xB9,0xDC,0xEA,0xB9,0xDC,0xEA,0);	
	ktd2058_breath(chip, chip->br_ops.fade_on, chip->br_ops.fade_off, chip->br_ops.time_on, chip->br_ops.time_off);
	ktd2058_mode_change(chip, DEFAULT, 0);

	ktd2058_set_color0(chip, 0xC0, 0x40, 0x00);
	ktd2058_set_color1(chip, 0x08, 0x00, 0x00);
	ktd2058_breath_2colors(chip, chip->br_ops.fade_on, chip->br_ops.fade_off, chip->br_ops.time_on, chip->br_ops.time_off);
	ktd2058_breath_2colors(chip, chip->br_ops.fade_on, chip->br_ops.fade_off, chip->br_ops.time_on, chip->br_ops.time_off);
	ktd2058_breath_2colors(chip, chip->br_ops.fade_on, chip->br_ops.fade_off, chip->br_ops.time_on, chip->br_ops.time_off);
	*/
}	

static void ktd2058_breath_white_16stage(struct ktd2058_chip *chip)
{
	uint8_t i;
	//uint8_t MAX_BRIGHTNESS = 0xB0;
	uint8_t MIN_BRIGHTNESS = 0x04;
	
	uint8_t brightness;

	
	static const u8 gamma_table[16] = {
		8,  8,  9, 10, 13, 17, 23, 31,
		41, 53, 68, 84, 103, 124, 147, 176
	};
	/*
	static const u8 gamma_table[12] = {
		4,   5,   7,  10,  16,  24,
		36,  52,  72,  97, 124, 176
	};
	*/
	
	ktd2058_mode_change(chip,NORMAL_MODE, 0x01);	
	ktd2058_select_color(chip, 0, 0); //select color 0
	
	for (i = 0; i < 16; i++) 
	{
		brightness = gamma_table[i];
		ktd2058_set_color0(chip, brightness, brightness, brightness);
		msleep(100);
	}
	
	msleep(600);
	ktd2058_mode_change(chip,NORMAL_MODE, 0x04);			
	ktd2058_set_color0(chip, MIN_BRIGHTNESS, MIN_BRIGHTNESS, MIN_BRIGHTNESS);
	msleep(2000);
	
}	

static void ktd2058_breath_white_2stage(struct ktd2058_chip *chip)
{
	
	uint8_t MAX_BRIGHTNESS = 0xB0;
	uint8_t MIN_BRIGHTNESS = 0x04;
	
	ktd2058_mode_change(chip,NORMAL_MODE, 0x07);	
	ktd2058_select_color(chip, 0, 0); //select color 0
	
	ktd2058_set_color0(chip, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
	msleep(2200);

	
	ktd2058_mode_change(chip,NORMAL_MODE, 0x04);			
	ktd2058_set_color0(chip, MIN_BRIGHTNESS, MIN_BRIGHTNESS, MIN_BRIGHTNESS);
	msleep(2000);
	
}	

static void ktd2058_breath_logscale(struct ktd2058_chip *chip, u8 fade, u8 ired, u8 igrn, u8 iblu)
{
	int delay_ms, i, gain;
	LOG_DBG("fade = 0x%x, ired = 0x%x, igrn = 0x%x, iblu = 0x%x\n", fade, ired, igrn, iblu);

	ktd2058_mode_change(chip, DEFAULT, 0);		
	ktd2058_set_color0(chip, 0x00, 0x00, 0x00); //off
	ktd2058_select_color(chip, 0, 0); //select color 0
	
	delay_ms = 900;//ktd_pow(30, fade);
	ktd2058_mode_change(chip,NORMAL_MODE, fade);			
	for (i=0; i<8; i++){
		gain = ktd_pow(2,(7-i));	
		//printf("%d , %d , %d\n",DIV_ROUND_UP(ired, gain), DIV_ROUND_UP(igrn, gain), DIV_ROUND_UP(iblu, gain));
		ktd2058_set_color0(chip, DIV_ROUND_UP(ired, gain), DIV_ROUND_UP(igrn, gain), DIV_ROUND_UP(iblu, gain));
		LOG_DBG("delay_ms = %d, gain = %d\n", delay_ms, gain);
		msleep(delay_ms);
	}
	msleep(delay_ms*3);
	ktd2058_set_color0(chip, 0x00, 0x00, 0x00);
	msleep(900);//msleep(ktd_pow(5.5/32*2, fade));
}

static void ktd2058_br_flower_demo(struct ktd2058_chip *chip)
{
	ktd2058_set_random_palette(chip, 0xFF);
	ktd2058_select_all_colors(chip, 0x80, 0xF0, 0x80, 0xF0, 0x80, 0xF0, 0);
	ktd2058_breath(chip, 0x07, 0x03, 1500, 1000);
	ktd2058_set_random_palette(chip, 0xFF);
	ktd2058_select_all_colors(chip, 0x0F, 0x08, 0x0F, 0x08, 0x0F, 0x08, 0);
	ktd2058_breath(chip, 0x07, 0x03, 1500, 1000);
}


static void ktd2058_breathing_ctrl_work(struct work_struct *work)
{
	struct ktd2058_chip *chip = container_of(work, struct ktd2058_chip, work_breathing);	
	LOG_DBG("start!\n");

	chip->br_ops.cnt++;

	switch(chip->br_ops.level){
		case BR_NORMAL:
			ktd2058_breath(chip, 0x07, 0x03, 1500, 1500);
			break;
		case BR_2COLORS:
			ktd2058_breath_2colors(chip, 0x07, 0x03, 1500, 1500);
			break;
		case BR_LOGSCALE:
			ktd2058_breath_logscale(chip, 0x07, 0xC0, 0xC0, 0x00);

			/*
			delay_ms = ktd_pow(30, chip->br_ops.fade_on) * 10;
			
			for (i=0; i<8; i++){
				gain = ktd_pow(2,(7-i));	
				ktd2058_set_color0(chip, 0xFF/gain, 0xFF/gain, 0xFF/gain);
				msleep(delay_ms);
			}
			msleep(delay_ms*3);
			ktd2058_set_color0(chip, 0x00, 0x00, 0x00);
			msleep(ktd_pow(5.5/32*2, chip->br_ops.fade_on));
			*/
			break;
		case BR_DEMO:
			ktd2058_breath_demo(chip);
			break;
			
		case BR_White:
			chip->br_ops.cnt=6;
			if(chip->br_ops.en==1)
				ktd2058_breath_white_16stage(chip);
			else
				ktd2058_breath_white_2stage(chip);
			break;
			
		case BR_FLOWERS:
			ktd2058_br_flower_demo(chip);
			break;
	
		case BR_LOGSCALE_DEMO:
			ktd2058_breath_logscale(chip, 2, 0xC0, 0xC0, 0x00);     //cycles=1, fade=2, color0 = yellow
			//ktd2058_breath_logscale(chip, 3, 0xC0, 0x00, 0xC0);     //cycles=1, fade=3, color0 = magenta
		    //ktd2058_breath_logscale(chip, 4, 0xC0, 0x18, 0x00);     //cycles=1, fade=4, color0 = orange
			//ktd2058_breath_logscale(chip, 5, 0x00, 0x30, 0xC0);     //cycles=1, fade=5, color0 = azure
		    //ktd2058_breath_logscale(chip, 6, 0xC0, 0x09, 0x30);     //cycles=1, fade=6, color0 = fushia
			//ktd2058_breath_logscale(chip, 7, 0x60, 0xC0, 0x00);     //cycles=1, fade=7, color0 = chartreuse
			
			break;

		case BR_OFF:	
		    ktd2058_breath_logscale(chip, 6, 0xC0, 0x09, 0x30);     //cycles=1, fade=6, color0 = fushia
			break;
			
		case STOP:
			ktd2058_mode_change(chip, GLOBAL_OFF, 0x02);
			break;
			
		default:
			break;
	}
	
	if(chip->br_ops.cnt < chip->br_ops.max_cnt || chip->br_ops.en >= 1 ){
		schedule_work(&chip->work_breathing);
	}else{
		chip->br_ops.cnt = 0;
		chip->br_ops.max_cnt = 0;
		chip->br_ops.en = 0;
		chip->br_ops.level = BR_OFF;
	}
	
	LOG_DBG("END!\n");
}

static void ktd2058_race_two(struct ktd2058_chip *chip, int state, u8 fade, int delay)
{
	int j;

    ktd2058_mode_change(chip, NORMAL_MODE, fade);	//global_on(fade)
	if (state == UP){
	    for(j = 0; j < 3; j++){				// reg in range(0x09, 0x0F):               # registers 0x09 to 0x0E
            ktd2058_select_one(chip, 0x09 + j, 0x80, 0);
            ktd2058_select_one(chip, 0x0E - j, 0x08, delay);
            ktd2058_select_one(chip, 0x09 + j, 0xF8, 0);
            ktd2058_select_one(chip, 0x0E - j, 0x8F, delay);
            ktd2058_select_one(chip, 0x09 + j, 0xFF, 0);
            ktd2058_select_one(chip, 0x0E - j, 0xFF, 0);
		}
	}else if(state == DOWN){
	    for(j=0x0E; j>0x08; j--){				// reg in range(0x0E, 0x08, -1):           # registers 0x0E to 0x09
            ktd2058_select_one(chip, 0x0B - j, 0x08, 0);
            ktd2058_select_one(chip, 0x0C + j, 0x80, delay);
            ktd2058_select_one(chip, 0x0B - j, 0x8F, 0);
            ktd2058_select_one(chip, 0x0C + j, 0xF8, delay);
            ktd2058_select_one(chip, 0x0B - j, 0xFF, 0);
            ktd2058_select_one(chip, 0x0C + j, 0xFF, 0);
		}
	}
}

static void ktd2058_flow_one(struct ktd2058_chip *chip, int state, u8 fade, int delay, u8 color)
{
	u8 sel_data;
	int i;

	LOG_DBG("Start!\n");
	
    if (color == 0)
		sel_data = 0x08;                         //# then select color0
	else if (color == 1)                         //   # if color=1,...
		sel_data = 0x0F;                         //# then select color1

    ktd2058_mode_change(chip, NORMAL_MODE, fade);	//global_on(fade)
	if (state == UP){
	    for(i = 0x09; i < 0x0F; i++){				// reg in range(0x09, 0x0F):               # registers 0x09 to 0x0E
		    ktd2058_select_one(chip, i, sel_data*16, delay);
			ktd2058_select_one(chip, i, sel_data*17, delay);
		}
	}else if(state == DOWN){
	    for(i=0x0E; i>0x08; i--){				// reg in range(0x0E, 0x08, -1):           # registers 0x0E to 0x09
		    ktd2058_select_one(chip, i, sel_data, delay);
			ktd2058_select_one(chip, i, sel_data*17, delay);
		}
	}
}

static void ktd2058_flow_two(struct ktd2058_chip *chip, int state, u8 fade, int delay, u8 color)
{
	u8 sel_data;
	int j;

	LOG_DBG("Start!\n");
	
    if (color == 0)
		sel_data = 0x08;                         //# then select color0
	else if (color == 1)                         //   # if color=1,...
		sel_data = 0x0F;                         //# then select color1

    ktd2058_mode_change(chip, NORMAL_MODE, fade);	//global_on(fade)
	if (state == UP){
	    for(j = 0; j < 3; j++){				// reg in range(0x09, 0x0F):               # registers 0x09 to 0x0E
			ktd2058_select_one(chip, 0x09 + j, sel_data*16, 0);
			ktd2058_select_one(chip, 0x0E - j, sel_data, delay);
			ktd2058_select_one(chip, 0x09 + j, sel_data*17, 0);
			ktd2058_select_one(chip, 0x0E - j, sel_data*17, delay);
		}
	}else if(state == DOWN){
	    for(j=0x0E; j>0x08; j--){				// reg in range(0x0E, 0x08, -1):           # registers 0x0E to 0x09
			ktd2058_select_one(chip, 0x0B - j, sel_data, 0);
			ktd2058_select_one(chip, 0x0C + j, sel_data*16, delay);
			ktd2058_select_one(chip, 0x0B - j, sel_data*17, 0);
			ktd2058_select_one(chip, 0x0C + j, sel_data*17, delay);
		}
	}
}

static ssize_t ktd2058_br_logscale_demo_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	msleep(10);

	ktd2058_mode_change(chip, DEFAULT, 0);	
	msleep(4000);

	if ((value > 0)&&(value < 100)){
		chip->br_ops.level = BR_LOGSCALE_DEMO;	
		chip->br_ops.max_cnt = value;

		schedule_work(&chip->work_breathing);	//ktd2058_set_color0(chip, 0xFF, 0xFF, 0xFF);
	}else{
		LOG_DBG("invalid value!\n");
	}

	LOG_DBG("End!\n");
	return size;
}
static ssize_t ktd2058_br_demo_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	msleep(10);

	ktd2058_mode_change(chip, DEFAULT, 0);	
	msleep(4000);

	if ((value > 0)&&(value < 100)){
		chip->br_ops.level = BR_DEMO;	
		chip->br_ops.max_cnt = value;

		schedule_work(&chip->work_breathing);	//ktd2058_set_color0(chip, 0xFF, 0xFF, 0xFF);
	}else{
		LOG_DBG("invalid value!\n");
	}

	LOG_DBG("End!\n");
	return size;
}

static ssize_t ktd2058_brightextend_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);

	if (value > 0x01) value = 0x01;
	else if (value < 0) value = 0;
	
	chip->pdata->brightextend = value;

	ktd2058_masked_write(chip->client, REG_CONTROL, 0x02, value << 5);
	LOG_DBG("End!\n");
	return size;
}

static ssize_t ktd2058_brightextend_show(struct device *dev,
												struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	uint8_t reg_val= 0;

    #if 0
	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
    #endif
	
	ktd2058_read_reg(chip->client, REG_CONTROL, &reg_val);
	reg_val = (reg_val & 0x20) >> 5;
	if (reg_val != chip->pdata->brightextend){
		chip->pdata->brightextend = reg_val;
		LOG_DBG("brightextend is changed, restored!");
	}
	return sprintf(buf, "0x%x\n", reg_val);
}

static ssize_t ktd2058_fade_rate_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	
	if (value > 0x07) value = 0x07;
	else if (value < 0) value = 0;
	
	chip->pdata->fade_rate = value;

	ktd2058_masked_write(chip->client, REG_CONTROL, 0x07, value);
	LOG_DBG("End!");
	return size;
}

static ssize_t ktd2058_fade_rate_show(struct device *dev,
												struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	uint8_t reg_val= 0;
    
	#if 0
	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	#endif
	
	ktd2058_read_reg(chip->client, REG_CONTROL, &reg_val);
	reg_val = (reg_val & 0x07);
	
	if (reg_val != chip->pdata->fade_rate)
	{
		chip->pdata->fade_rate = reg_val;
		LOG_DBG("fade_rate is changed, restored!");
	}
	
	return sprintf(buf, "0x%x\n", reg_val);
}

static ssize_t ktd2058_ce_temp_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");

	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);	
	
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	if (value > 0x03) value = 0x03;
	else if (value < 0) value = 0;
	
	chip->pdata->ce_temp = value;

	ktd2058_masked_write(chip->client, REG_CONTROL, 0x18, value << 3);
	LOG_DBG("End!");
	return size;
}

static ssize_t ktd2058_ce_temp_show(struct device *dev,
												struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	uint8_t reg_val= 0;

    #if 0
	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
    #endif
	
	ktd2058_read_reg(chip->client, REG_CONTROL, &reg_val);
	reg_val = (reg_val & 0x18) >> 3;
	if (reg_val != chip->pdata->ce_temp){
		chip->pdata->ce_temp = reg_val;
		LOG_DBG("ce_temp is changed, restored!");
	}
	return sprintf(buf, "0x%x\n", reg_val);
}

static ssize_t ktd2058_breathing_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	msleep(10);

	if ((value > 0)&&(value < 100)){
	    ktd2058_set_color0(chip, 0x00, 0x00, 0x00);                // color0 = black, adjust for other palettes
		ktd2058_set_color1(chip, 0x80, 0x80, 0x80);                // color1 = white, adjust for other palettes
	    ktd2058_select_all_colors(chip, 0x88, 0x89, 0xAB, 0xCD, 0xEF, 0xFF, 0);	//ktd2058_set_color0(chip, 0xFF, 0xFF, 0xFF);
		chip->br_ops.cnt = 0; //init cnt;
		chip->br_ops.en = 0;
		chip->br_ops.max_cnt = value;
		chip->br_ops.level = BR_NORMAL;
		//ktd2058_mode_change(chip, DEFAULT, 0);	
		schedule_work(&chip->work_breathing);	
	}else if (value == 100){
		cancel_work_sync(&chip->work_breathing);
		msleep(10);
		ktd2058_util_color_palette(chip, 7, 1000);
	}

	LOG_DBG("End!");
	return size;
}	


static ssize_t ktd2058_br_white_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	
	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	msleep(10);
	
	
	chip->br_ops.cnt = 6; 		//not used
	chip->br_ops.max_cnt = 5;	//not used

	if(value <= 0)
	{
		chip->br_ops.en = 0;
		chip->br_ops.level = STOP;
	}else if(value ==1)
	{
		chip->br_ops.en = 1;
		chip->br_ops.level = BR_White;
	}
	else{
		chip->br_ops.en = 2;
		chip->br_ops.level = BR_White;
	}
	schedule_work(&chip->work_breathing);	

	LOG_DBG("End!");
	return size;
}	


static ssize_t ktd2058_br_white_show(struct device *dev,
												struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	char *led_en;
	
	if(chip->br_ops.en)
		led_en="RingLED Enable\n";
	else 
		led_en="RingLED Disable\n";
	
	
	return sprintf(buf,"RingLED Status:\n%s", led_en);
}

static ssize_t ktd2058_br2colors_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	msleep(10);

	if ((value > 0)&&(value < 100)){
		//ktd2058_set_color0(chip, 0xFF, 0xFF, 0xFF);
		chip->br_ops.cnt = 0; //init cnt;
		chip->br_ops.en = 0;
		chip->br_ops.max_cnt = value;
		chip->br_ops.level = BR_2COLORS;
		ktd2058_mode_change(chip, DEFAULT, 0);	
		msleep(4000);
		
		ktd2058_set_color0(chip, 0xC0, 0x40, 0x00);
		ktd2058_set_color1(chip, 0x08, 0x00, 0x00);
		schedule_work(&chip->work_breathing);	
	}else{
		LOG_DBG("invalid value!\n");
	}

	LOG_DBG("End!");
	return size;
}	

static ssize_t ktd2058_br_logscale_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	cancel_work_sync(&chip->work_chase);
	cancel_work_sync(&chip->work_breathing);
	msleep(10);

	if ((value > 0)&&(value < 8)){
		ktd2058_set_color0(chip, 0xFF, 0xFF, 0xFF);
		chip->br_ops.cnt = 0; //init cnt;
		chip->br_ops.en = 0;
		chip->br_ops.max_cnt = value;
		chip->br_ops.level = BR_LOGSCALE;
		
		ktd2058_mode_change(chip, DEFAULT, 0);	
		msleep(4000);
		//delay_ms = (15 * 2) << value/*fade rate*/;
		ktd2058_mode_change(chip, NORMAL_MODE, chip->br_ops.fade_on);

		schedule_work(&chip->work_breathing);	
	}else{
		LOG_DBG("invalid value!\n");
	}

	LOG_DBG("End!");
	return size;
}	

static ssize_t ktd2058_chase_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}

	cancel_work_sync(&chip->work_breathing);
	cancel_work_sync(&chip->work_chase);
	
	if ((value >= 0) && (value < 20)){
		chip->chase_ops.level = value;
		chip->chase_ops.fade = 0;
		chip->chase_ops.cnt = 0;
		chip->chase_ops.max_cnt = 1;
		chip->chase_ops.delay = 2000;
	
		if ((value == CHASE_2COLORS_UP) || (value == CHASE_2COLORS_DOWN)){
			chip->chase_ops.max_cnt = 5;
			ktd2058_mode_change(chip, DEFAULT, 0);
			msleep(4000);
			ktd2058_select_color(chip, 3, 0);
		}else if ((value == CHASEUP_1) || (value == CHASEDOWN_2)){
			chip->chase_ops.max_cnt = chip->chase_ops.max_cnt*2;
		}else if ((value == CHASEUP_3) || (value == CHASEDOWN_3)){
			chip->chase_ops.max_cnt = chip->chase_ops.max_cnt*3;
		}else if ((value == CHASEUP_4) || (value == CHASEDOWN_4)){
			chip->chase_ops.max_cnt = chip->chase_ops.max_cnt*4;
		}else if(value == CHASE_DEMO){
		}else if(value == CHASE_AMZ){
		}

		ktd2058_mode_change(chip, NORMAL_MODE, chip->chase_ops.fade);

		schedule_work(&chip->work_chase);

	}else{
		LOG_DBG("Invalid value!\n");
	}
	
	return size;
}

static ssize_t ktd2058_modechange_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	if(value == 0)	{
		ktd2058_mode_change(chip, GLOBAL_OFF, 7);
	}else if (value == 1){
		ktd2058_mode_change(chip, NIGHT_MODE, 7);
	}else if (value == 2){
		ktd2058_mode_change(chip, NORMAL_MODE, 7);
	}else if (value == 3){	
		ktd2058_mode_change(chip, DEFAULT, 7);
	}
	return size;
}

static ssize_t ktd2058_br_flower_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	if ((value > 0)&&(value < 100)){
		//ktd2058_set_color0(chip, 0xFF, 0xFF, 0xFF);
		chip->br_ops.cnt = 0; //init cnt;
		chip->br_ops.en = 0;
		chip->br_ops.max_cnt = value;
		chip->br_ops.level = BR_FLOWERS;
		ktd2058_mode_change(chip, DEFAULT, 0);	
		msleep(4000);
		
		schedule_work(&chip->work_breathing);	
	}else{
		LOG_DBG("invalid value!\n");
	}

	LOG_DBG("End!");
	return size;
}

static ssize_t ktd2058_chasing_demo_store(struct device *dev, 
												struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	if ((value > 0)&&(value < 100)){
		ktd2058_mode_change(chip, DEFAULT, 0);	
		msleep(4000);
		
		//ktd2058_chasing_demo(chip);
		chip->chase_ops.level = CHASE_DEMO;

		schedule_work(&chip->work_chase);
	}else{
		LOG_DBG("invalid value!\n");
	}

	LOG_DBG("End!");
	return size;
}

static void ktd2058_racing_demo(struct ktd2058_chip *chip)
{
	int i;

	ktd2058_select_color(chip, 3, 0);			//	    select_off()
    ktd2058_set_color0(chip, 0x00, 0xC0, 0x00);	//                # color0 = bright green
    ktd2058_set_color1(chip, 0x00, 0x00, 0x00); //               # color1 = black (off)

    for(i = 7; i>0; i--){						// fade in range(7, 0, -1):
        ktd2058_race_two(chip, UP, i, 100);     //             # cycles=1, fade, delay=0.1s
	}

    ktd2058_race_two(chip, UP, 1, 100);     //                    # cycles=6, fade=1, delay=0.1s
    ktd2058_race_two(chip, UP, 1, 100);     //                    # cycles=6, fade=1, delay=0.1s
	ktd2058_race_two(chip, UP, 1, 100);     //                    # cycles=6, fade=1, delay=0.1s

    for(i=0; i<3; i++){										//i in range(3):
        ktd2058_race_two(chip, UP, 1, 83);
        ktd2058_race_two(chip, DOWN, 1, 83);
	}
	ktd2058_select_color(chip, 3, 0);			//	    select_off()
    ktd2058_set_color1(chip, 0x08, 0x04, 0x00);      //          # color1 = dim yellow-orange
    for(i=0; i<3; i++){							// i in range(3):
        ktd2058_race_two(chip, UP, 1, 83);
        ktd2058_race_two(chip, DOWN, 1, 83);
	}

    ktd2058_set_color0(chip, 0x00, 0x30, 0xC0);                //# color0 = bright azure blue
    for(i=0; i<3; i++){							// i in range(3):
        ktd2058_race_two(chip, UP, 1, 83);
        ktd2058_race_two(chip, DOWN, 1, 83);
	}
	
	ktd2058_race_two(chip, DOWN, 1, 100);//race_2out(3, 1, 0.1)                        # cycles=3, fade=1, delay=0.1s
	ktd2058_race_two(chip, DOWN, 1, 100);//race_2out(3, 1, 0.1)                        # cycles=3, fade=1, delay=0.1s
    ktd2058_race_two(chip, DOWN, 1, 100);//race_2out(3, 1, 0.1)                        # cycles=3, fade=1, delay=0.1s
    
	for(i=1; i<5; i++){// fade in range(1, 5):
        ktd2058_race_two(chip, DOWN, i, 0.1);                 //# cycles=1, fade, delay=0.1s
	}
	
	ktd2058_select_color(chip, 3, 0);			//	    select_off()
    msleep(2000);//time.sleep(2)
}

static void ktd2058_flowing_demo(struct ktd2058_chip *chip)
{
	int i;

	ktd2058_mode_change(chip, DEFAULT, 0);			//DEFAULT()
	ktd2058_select_color(chip, 3, 0);				//select_off()
    ktd2058_set_color0(chip, 0xC0, 0x00, 0xC0);     //# color0 = magneta
    ktd2058_set_color1(chip, 0x00, 0x00, 0x00);     //# color1 = black (off)
    for(i=0; i<2; i++){								//i in range(2):
        ktd2058_flow_one(chip, UP, 4, 83, 0);		//# fade=4, delay=83ms, color=0
        msleep(100);								//time.sleep(0.1)
        ktd2058_flow_one(chip, DOWN, 2, 100, 1);    //# fade=2, delay=0.1s, color=1
        ktd2058_flow_one(chip, DOWN, 4, 83, 0);     //# fade=4, delay=83ms, color=0
        msleep(100);//time.sleep(0.1)
        ktd2058_flow_one(chip, UP, 2, 100, 1);      //# fade=2, delay=0.1s, color=1
        ktd2058_set_color0(chip, 0xC0, 0x09, 0x30); //# color0 = fushia
	}
    msleep(500);										//time.sleep(0.5)
	ktd2058_select_color(chip, 3, 0);				//select_off()
    ktd2058_set_color0(chip, 0xC0, 0xC0, 0x00);     //# color0 = yellow
    ktd2058_set_color1(chip, 0x00, 0x00, 0xC0);     //# color1 = blue
    for(i=0; i<3; i++){											// i in range(3):
        ktd2058_flow_two(chip, UP, 4, 100, 0);      //# fade=4, delay=0.1s, color=0
        ktd2058_flow_two(chip, DOWN, 2, 0.1, 1);    //# fade=2, delay=0.1s, color=1
		msleep(100);								//time.sleep(0.1)
        ktd2058_flow_two(chip, DOWN, 2, 0.1, 0);    //#fade=2, delay=0.1s, color=0
        ktd2058_flow_two(chip, UP, 2, 0.1, 1);      //# fade=2, delay=0.1s, color=1
        msleep(100);								//time.sleep(0.1)
	}
    ktd2058_mode_change(chip, GLOBAL_OFF, 3);       //global_off(3)
	msleep(2000);								//time.sleep(2)
}

static ssize_t ktd2058_racing_demo_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	if(ret){
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	//if ((value > 0)&&(value < 100)){
	if(value == 1){
		ktd2058_mode_change(chip, DEFAULT, 0);	
		msleep(4000);
		
		ktd2058_racing_demo(chip);
	}
	else if(value == 2){
		ktd2058_mode_change(chip, DEFAULT, 0);	
		msleep(4000);
		
		ktd2058_flowing_demo(chip);
	}else{
		LOG_DBG("invalid value!\n");
	}

	LOG_DBG("End!");
	return size;
}
//For driver debug
static ssize_t ktd2058_debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;

	LOG_DBG("Start!\n");
	ret = kstrtouint(buf, 10, &value);
	
	if (ret)
	{
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	DBG_INFO("[%s]value = %d\n", __func__, value);
	
	#if 0
	//DBG_INFO("Reading 0x%02x=0x%02x\n", reg, *val);
	DBG_INFO("---------------------\n");
	DBG_INFO("chip->br_ops.level     = %d\n", chip->br_ops.level);
	DBG_INFO("chip->br_ops.fade_on   = %d\n", chip->br_ops.fade_on);
	DBG_INFO("chip->br_ops.fade_off  = %d\n", chip->br_ops.fade_off);
	DBG_INFO("chip->br_ops.time_on   = %d\n", chip->br_ops.time_on);
	DBG_INFO("chip->br_ops.time_off  = %d\n", chip->br_ops.time_off);
	DBG_INFO("chip->br_ops.en        = %d\n", chip->br_ops.en);
	DBG_INFO("chip->br_ops.cnt       = %d\n", chip->br_ops.cnt);
	DBG_INFO("chip->br_ops.max_cnt   = %d\n", chip->br_ops.max_cnt);
	DBG_INFO("---------------------\n");
	DBG_INFO("chip->chase_ops.level   = %d\n", chip->chase_ops.level);
	DBG_INFO("chip->chase_ops.fade    = %d\n", chip->chase_ops.fade);
	DBG_INFO("chip->chase_ops.delay   = %d\n", chip->chase_ops.delay);
	DBG_INFO("chip->chase_ops.cnt     = %d\n", chip->chase_ops.cnt);
	DBG_INFO("chip->chase_ops.max_cnt = %d\n", chip->chase_ops.max_cnt);
	DBG_INFO("---------------------\n");
	#endif
	
	return size;
}

//To adjust the color pattern.
static ssize_t ktd2058_device_brightness_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;
	
	ret = kstrtouint(buf, 10, &value);
	
	if (ret)
	{
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	if (value == 201) //Up
	{
		if (chip->color.u8Brightness < 100)
		{
			chip->color.u8Brightness += 10;
			ktd2058_color_pattern_set(chip);
		}		
	}
	else if (value == 202) //Down
	{
		if (chip->color.u8Brightness > 10)
		{
			chip->color.u8Brightness -= 10;
			ktd2058_color_pattern_set(chip);
		}		
	} 
	else if (value > 0 && value <= 100)
	{
		chip->color.u8Brightness = value;	
		ktd2058_color_pattern_set(chip);
	}
	else
	{
		DBG_INFO("Error value!(%d).\n", value);
	}
				
	return size;
}
//Aron.
static ssize_t ktd2058_device_color(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	
    //pr_info("ktd2058_device_info.\n");

	//ktd2058_read_reg(chip->client, REG_ID, &reg_val[0]);
	
	return sprintf(buf, "Color:%d, Brightness:%d\n", chip->color.enPattern, chip->color.u8Brightness);
}
//To adjust the color pattern.
static ssize_t ktd2058_device_color_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	ssize_t ret;
	unsigned int value;
	
	ret = kstrtouint(buf, 10, &value);
	
	if (ret)
	{
		dev_err(chip->dev, "%s: failed to store!\n", __func__);
		return ret;  
	}
	
	//DBG_INFO("ktd2058_device_color_store(%d).\n", value);
	
	chip->color.enPattern = value;
	
	ktd2058_color_pattern_set(chip);
				
	return size;
}

//Aron.
//To show device info.
//#define DEVICE_ATTR(_name, _mode, _show, _store)
/*
_show : read method.  [ex:cat info]
_store: write method. [ex:echo > breathing]
*/
static ssize_t ktd2058_device_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ktd2058_chip *chip = container_of(led_cdev, struct ktd2058_chip, cdev_flash);
	uint8_t i,reg_val[REG_IBLU1+1] = {0};

    //pr_info("ktd2058_device_info.\n");
	
	for (i=REG_ID;i<=REG_IBLU1;i++)
	{
		ktd2058_read_reg(chip->client, i, &reg_val[i]);
	}
	
	//ktd2058_read_reg(chip->client, REG_ID, &reg_val[0]);
	
	return sprintf(buf, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n", reg_val[0],reg_val[1],reg_val[2],reg_val[3],reg_val[4],reg_val[5],reg_val[6],reg_val[7],reg_val[8]);
}

static ssize_t ktd2058_device_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	
    //pr_info("ktd2058_device_info.\n");
	
	return sprintf(buf, "%s\n", KTD2058_DRIVER_VER);
}
//==============================================================================
static void ktd2058_color_pattern_set(struct ktd2058_chip *chip)
{
	   u8 u8Red, u8Green, u8Blue, enColor, u8brightness;
	   
	   cancel_work_sync(&chip->work_breathing);
	   cancel_work_sync(&chip->work_chase);
	
	   ktd2058_mode_change(chip, DEFAULT, 0);			//DEFAULT()
	   ktd2058_select_color(chip, 3, 0);				//select_off()
 
       enColor 		= chip->color.enPattern;
	   u8brightness = chip->color.u8Brightness;
	   
       DBG_INFO("ktd2058_color_pattern_set(%d,%d).\n", enColor, u8brightness);
	   
	   if (u8brightness < 1 || u8brightness > 100) //over range
	   {
		   u8brightness = 100;
	   }
	   
	   u8Red   = (u8)((u16)192 * (u16)u8brightness / (u16)100);
	   u8Green = u8Red;
	   u8Blue  = u8Red;
	   
       switch(enColor)
		{
			case 0: //off
				ktd2058_mode_change(chip, GLOBAL_OFF, 0);
				return;
				
			case 1: //Red
				ktd2058_set_color0(chip, u8Red, 0x00, 0x00);     
				ktd2058_set_color1(chip, u8Red, 0x00, 0x00);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 2: //Green
				ktd2058_set_color0(chip, 0x00, u8Green, 0x00);     
				ktd2058_set_color1(chip, 0x00, u8Green, 0x00);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 3: //Blue
				ktd2058_set_color0(chip, 0x00, 0x00, u8Blue);     
				ktd2058_set_color1(chip, 0x00, 0x00, u8Blue);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 4: //Yellow
				ktd2058_set_color0(chip, u8Red, u8Green, 0x00);     
				ktd2058_set_color1(chip, u8Red, u8Green, 0x00);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 5: //Purple
				ktd2058_set_color0(chip, u8Red, 0x00, u8Blue);     
				ktd2058_set_color1(chip, u8Red, 0x00, u8Blue);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 6: //Cyan
				ktd2058_set_color0(chip, 0x00, u8Green, u8Blue);     
				ktd2058_set_color1(chip, 0x00, u8Green, u8Blue);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 7: //White
				ktd2058_set_color0(chip, u8Red, u8Green, u8Blue);     
				ktd2058_set_color1(chip, u8Red, u8Green, u8Blue);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 8: //Pink			    
				u8Red   = (u8)((u16)255 * (u16)u8brightness / (u16)100);
				u8Green = (u8)((u16)192 * (u16)u8brightness / (u16)100);
				u8Blue  = (u8)((u16)203 * (u16)u8brightness / (u16)100);
								
				ktd2058_set_color0(chip, u8Red, u8Green, u8Blue);     
				ktd2058_set_color1(chip, u8Red, u8Green, u8Blue);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;
			case 9: //Orange
			    u8Red   = (u8)((u16)255 * (u16)u8brightness / (u16)100);
				u8Green = (u8)((u16)165 * (u16)u8brightness / (u16)100);
								
				ktd2058_set_color0(chip, u8Red, u8Green, 0x00);     
				ktd2058_set_color1(chip, u8Red, u8Green, 0x00);     //# color1 = black (off)
				ktd2058_select_color(chip, 0, 0); //color 0
				//ktd2058_select_color(chip, 1, 0); //color 1
				break;	   
			
			default: //off
				DBG_INFO("Error color value!(%d).\n", enColor);
				return; 			   
		}
				
	ktd2058_mode_change(chip, NORMAL_MODE, 0);	
	
	DBG_INFO("u8Red(%d) u8Green(%d) u8Blue(%d)\n", u8Red, u8Green, u8Blue);
}
//==============================================================================
static DEVICE_ATTR(mode, 0664, NULL, ktd2058_modechange_store);
static DEVICE_ATTR(brightextend, 0644, ktd2058_brightextend_show, ktd2058_brightextend_store);
static DEVICE_ATTR(ce_temp, 0644, ktd2058_ce_temp_show, ktd2058_ce_temp_store);
static DEVICE_ATTR(fade_rate, 0644, ktd2058_fade_rate_show, ktd2058_fade_rate_store);
//==============================================================================
static DEVICE_ATTR(breathing, 0664, NULL, ktd2058_breathing_store);
static DEVICE_ATTR(br_white, 0664, ktd2058_br_white_show, ktd2058_br_white_store);
static DEVICE_ATTR(br2colors, 0664, NULL, ktd2058_br2colors_store);
static DEVICE_ATTR(brlogscale, 0664, NULL, ktd2058_br_logscale_store);
static DEVICE_ATTR(br_demo, 0664, NULL, ktd2058_br_demo_store);
static DEVICE_ATTR(br_logscale_demo, 0664, NULL, ktd2058_br_logscale_demo_store);
static DEVICE_ATTR(flower, 0664, NULL, ktd2058_br_flower_store);
//==============================================================================
static DEVICE_ATTR(chase, 0664, NULL, ktd2058_chase_store);
static DEVICE_ATTR(chasing_demo, 0664, NULL, ktd2058_chasing_demo_store);
static DEVICE_ATTR(racing, 0664, NULL, ktd2058_racing_demo_store);
static DEVICE_ATTR(br_color, 0664, NULL, ktd2058_device_brightness_store);
static DEVICE_ATTR(color, 0664, ktd2058_device_color, ktd2058_device_color_store);
//==============================================================================
static DEVICE_ATTR(info, 0664, ktd2058_device_info, NULL);
static DEVICE_ATTR(version, 0664, ktd2058_device_version, NULL);
static DEVICE_ATTR(debug, 0664, NULL, ktd2058_debug_store);
//==============================================================================
static struct attribute *ktd2058_flash_attrs[] = {
	&dev_attr_mode.attr,
	&dev_attr_brightextend.attr,
	&dev_attr_fade_rate.attr,
	&dev_attr_ce_temp.attr,
	&dev_attr_breathing.attr,
	&dev_attr_br2colors.attr,
	&dev_attr_brlogscale.attr,
	&dev_attr_br_demo.attr,
	&dev_attr_br_white.attr,	
	&dev_attr_br_logscale_demo.attr,
	&dev_attr_flower.attr,
	&dev_attr_chase.attr,
	&dev_attr_chasing_demo.attr,
	&dev_attr_racing.attr,
	&dev_attr_br_color.attr,
	&dev_attr_color.attr,
	&dev_attr_info.attr,
	&dev_attr_version.attr,
	&dev_attr_debug.attr,
	NULL,
};
//==============================================================================
ATTRIBUTE_GROUPS(ktd2058_flash);
//==============================================================================
static void ktd2058_chip_init(struct ktd2058_chip *chip)
{
	struct ktd2058_platform_data *pdata = chip->pdata;
	u8 value = 0;

	value = (pdata->brightextend << 5) | (pdata->ce_temp << 3)| (pdata->fade_rate);
	ktd2058_write_reg(chip->client, REG_CONTROL, value);

	chip->br_ops.level = BR_OFF;
	chip->chase_ops.level = CHASE_OFF;
    
	chip->color.enPattern = 0;
	chip->color.u8Brightness = 100;
}

static int ktd2058_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ktd2058_platform_data *pdata = dev_get_drvdata(&client->dev);
	struct ktd2058_chip *chip;
	int err = 0;
	
	LOG_DBG("probe start!\n");
	
	if(!pdata){
		ktd_parse_dt(&client->dev, chip);
		pdata = dev_get_platdata(&client->dev);
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,"%s: check_functionality failed.", __func__);
		err = -ENODEV;
		goto exit0;	
	}

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip){
		dev_err(&client->dev, "%s: failed to allocate driver data.\n", __func__);
		err = -ENOMEM;
		goto exit0;
	}

	chip->client = client;
	chip->pdata = pdata;
	chip->dev = &client->dev;
	mutex_init(&chip->lock);
	i2c_set_clientdata(client, chip);
	ktd2058_chip_init(chip);
	ktd2058_mode_change(chip, GLOBAL_OFF, 0/*fade rate*/);
	INIT_WORK(&chip->work_breathing, ktd2058_breathing_ctrl_work);
	INIT_WORK(&chip->work_chase, ktd2058_chase_ctrl_work);
	chip->cdev_flash.name = "ktd2058";
	//chip->cdev_flash.max_brightness = 127;
	//chip->cdev_flash.brightness_set = ktd2058_strobe_brightness_set;
	chip->cdev_flash.default_trigger = "flash";
	chip->cdev_flash.groups = ktd2058_flash_groups;
	err = led_classdev_register((struct device *)&client->dev, &chip->cdev_flash);
	if(err < 0){
		dev_err(chip->dev, "failed to register flash\n");
		goto exit1;
	}

	LOG_DBG("probe finished!\n");
	
	return 0;

exit1:
	led_classdev_unregister(&chip->cdev_flash);
exit0:
	return err;
}

static int ktd2058_remove(struct i2c_client *client)
{
	struct ktd2058_chip *chip = i2c_get_clientdata(client);

	led_classdev_unregister(&chip->cdev_flash);
	return 0;
}

static const struct i2c_device_id ktd2058_id[] = {
	{ktd2058_NAME, 0},
	{}
};

static struct of_device_id ktd2058_match_table[] = {
	{ .compatible = "ktd,ktd2058",},
	{ },
};

MODULE_DEVICE_TABLE(of, ktd2058_match_table);

static struct i2c_driver ktd2058_i2c_driver = {
	.driver = {
			.name = ktd2058_NAME, 
			.owner = THIS_MODULE,
			.of_match_table = ktd2058_match_table,
			.pm = NULL,
			},
	.probe = ktd2058_probe, 
	.remove = ktd2058_remove, 
	.id_table = ktd2058_id, 
};

static int __init ktd2058_init(void)
{
	int err;
	err = i2c_add_driver(&ktd2058_i2c_driver);
	if(err){
		printk(KERN_WARNING "ktd2058 driver failed"
				"(errno = %d)\n", err);
	}else {
		 printk( "Successfully added driver %s\n",
					ktd2058_i2c_driver.driver.name);
	}
	return err;
}

static void __exit ktd2058_exit(void)
{
	i2c_del_driver(&ktd2058_i2c_driver);
}


module_init(ktd2058_init);
module_exit(ktd2058_exit);

MODULE_DESCRIPTION("KINETIC TECHNOLOGIES FLASH DRIVER FOR ktd2058");
MODULE_AUTHOR("Luke Jang ljang@kinet-ic.com");
MODULE_LICENSE("GPL");
