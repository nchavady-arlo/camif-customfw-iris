/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include "demo.h"
#include "rfal_nfc.h"
#include <stdio.h>
#include <stdlib.h>
/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

#define NS_IN_ONE_SECOND        1000000000  /*!< Number of nanoseconds in a second */

#define US_TO_NS                      1000  /*!< Convert microseconds to nanoseconds */
#define US_TO_SEC                  1000000  /*!< Convert microseconds to seconds */
#define MS_TO_US                      1000  /*!< Convert milliseconds to microseconds */

#define DEMO_NFC_DEFAULT_SLEEP        1000  /*!< Default value, used e.g. while polling and performing data exchange */

#define DEMO_NFC_WAKEUP_SLEEP       100000  /*!< safety delay, in microseconds */


/*
 ******************************************************************************
 * MAIN FUNCTION
 ******************************************************************************
 */

/* Semaphore used to suspend main thread until:
   - either an interrupt occurs (on the IRQ_MCU GPIO)
   - or a timer expires */
sem_t rfal_sem;

/*!
 *****************************************************************************
 * \brief demoGetUsSleepDuration
 *
 *  This function returns suggested sleep durations until the next call of 
 *  rfalNfcWorker(). It tries to balance reactivity vs MCU processing.
 *  This is the time allowed to sleep before calling the rfalNfcWorker() again.
 *
 *  It allows to reduce the calls to the rfalNfcWorker(), reduces processing, 
 *  and thus reduces the CPU load.
 *
 *  \return the estimated duration while there is no event to be processed by the 
 *          RFAL worker, expressed in microseconds.
 *
 *****************************************************************************
 */
uint32_t demoGetUsSleepDuration(void)
{
    rfalNfcState state = rfalNfcGetState();
    uint32_t     sleep = DEMO_NFC_DEFAULT_SLEEP; /* Value used for communication to be able to advance the state machine */

    uint16_t techs2Find    = demoGetDiscoverTechs2Find();
    uint16_t totalDuration = demoGetDiscoverTotalDuration();

    if( state == RFAL_NFC_STATE_WAKEUP_MODE )
    {
        /* Strictly NFC chip will be here fully autonomous. Giving a time for pure robustness */
        sleep = DEMO_NFC_WAKEUP_SLEEP;
    }
    else if( ((state == RFAL_NFC_STATE_LISTEN_TECHDETECT) && ((techs2Find & 0xF000) == 0U)) /* Tag detection mode */
          ||  (state == RFAL_NFC_STATE_LISTEN_COLAVOIDANCE) /* Listen mode */ )
    {
        /* in listen mode states */

        /* Additional safety timeout. RFAL should anyhow proceed due to discTmr expiration. Giving a value is for robustness. */
        sleep = (totalDuration + 100) * MS_TO_US;
    }

    return sleep;
}

bool add_time(struct timespec* ts, struct timespec* delay, struct timespec* res)
{
    bool overflow = false;

    assert(ts    != NULL);
    assert(delay != NULL);
    assert(res   != NULL);
    assert(ts->tv_nsec    < NS_IN_ONE_SECOND);
    assert(delay->tv_nsec < NS_IN_ONE_SECOND);

    res->tv_sec   = ts->tv_sec;
    res->tv_nsec  = ts->tv_nsec;
    res->tv_sec  += delay->tv_sec;
    res->tv_nsec += delay->tv_nsec;

    if (res->tv_nsec >= NS_IN_ONE_SECOND)  /* nanoseconds must be in range [0 .. 999999999] */
    {
        res->tv_sec  += 1U;
        res->tv_nsec -= NS_IN_ONE_SECOND;
        overflow = true;
    }
    return overflow;
}

int main(void)
{
#if DEMO_CARD_EMULATION_ONLY
    platformLog("Welcome to the ST25R NFC Card Emulation Demo on Linux.\r\n");
#else
    platformLog("Welcome to the ST25R NFC Demo on Linux.\r\n");
#endif
    platformLog("Scanning for NFC technologies...\r\n");

    int ret = 0;
    struct timespec sem_time;
    struct timespec ts_delay;
    struct timespec new_time;

    setlinebuf(stdout);

    /* Initialize the platform */

	/* Create gpiochip0 */
	ret = system("/pega_bin/init_gpiochip0.sh");
    if (ret == -1) {
        perror("system() fail\n");
        return 1;
    }

    /* Initialize GPIO */
    ret = gpio_init();
    if (ret != ERR_NONE)
        goto error;
#ifdef RFAL_USE_I2C
    /* Initialize I2C */
    ret = i2c_init();
    if (ret != ERR_NONE)
        goto error;
#else
    /* Initialize SPI */
    ret = spi_init();
    if (ret != ERR_NONE)
        goto error;
#endif
    /* Initialize interrupt mechanism */
    ret = interrupt_init();
    if (ret != ERR_NONE)
        goto error;

    /* Initialize the semaphore */
    ret = sem_init(&rfal_sem, 0, 0);
    if (ret != 0) {
        goto error;
    }

    /* Initialize RFAL and run the demo */
    bool ok = demoIni();
		
    while(1)
    {
		demoCycle();

        uint32_t delay = demoGetUsSleepDuration();

        ts_delay.tv_sec  =  delay / US_TO_SEC;
        ts_delay.tv_nsec = (delay % US_TO_SEC) * US_TO_NS;
		     
		/* Lock Semaphore to suspend this thread */
        clock_gettime(CLOCK_REALTIME, &sem_time);

        add_time(&sem_time, &ts_delay, &new_time);

        int err = sem_timedwait(&rfal_sem, &new_time);
        if ( (err != 0) && (errno != ETIMEDOUT) ) {
            platformLog("sem_timedwait error %d errno %d\r\n", err, errno);
        }
    }
	
    sem_destroy(&rfal_sem);

error:
    return ret;
}
