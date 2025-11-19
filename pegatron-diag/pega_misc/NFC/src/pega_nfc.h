#ifndef PEGA_NFC_H
#define PEGA_NFC_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <semaphore.h>

// External semaphore used to suspend/resume NFC thread
extern sem_t rfal_sem;

/**
 * @brief Initialize the NFC device (GPIO, I2C, RFAL, etc.)
 * 
 * @return int 0 on success, non-zero on failure
 */
int Pega_NFC_Device_init(void);

/**
 * @brief Start the NFC task handler in a new thread
 */
void Pega_NFC_TaskHandler_Start(void);

/**
 * @brief Adds two timespec values and stores the result in res. 
 *        Handles nanosecond overflow.
 *
 * @param ts    Base time (struct timespec)
 * @param delay Delay time (struct timespec)
 * @param res   Result time (struct timespec)
 * 
 * @return true if nanosecond overflow occurred (i.e. ns >= 1s), false otherwise
 */
bool add_time(struct timespec* ts, struct timespec* delay, struct timespec* res);

/**
 * @brief Returns the recommended sleep duration (in microseconds)
 *        before the next call to rfalNfcWorker.
 *
 * @return uint32_t Suggested sleep duration in microseconds
 */
uint32_t demoGetUsSleepDuration(void);

#endif // PEGA_NFC_H
