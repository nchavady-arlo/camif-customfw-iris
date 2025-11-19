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

/*! \file pltf_i2c.h
 *
 *  \author
 *
 *  \brief Implementation for I2C communication.
 *
 */

#ifndef PLATFORM_I2C_H
#define PLATFORM_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <stdbool.h>
#include <stdint.h>
#include "st_errno.h"

/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */

/*!
 *****************************************************************************
 * \brief  Initialize I2C Interface
 *
 * This methods initialize I2C interface so that Linux host can use I2C interface
 * to communicate with ST25R.
 *
 * \return ERR_IO   : I2C interface not initialized successfuly
 * \return ERR_NONE : No error
 *****************************************************************************
 */
stError i2c_init();

/*!
 *****************************************************************************
 * \brief  I2C Interface
 *
 * This methods initialize I2C interface so that Linux host can use I2C interface
 * to communicate with ST25R.
 *
 * \return ERR_IO   : I2C interface not initialized successfuly
 * \return ERR_NONE : No error
 *****************************************************************************
 */
/* function for full duplex I2C communication */
void i2c_start(void);
stError i2c_tx(const uint8_t* txBuf, uint16_t txLen, bool last, bool txOnly);
stError i2c_rx(const uint8_t* rxBuf, uint16_t rxLen);

/*!
 *****************************************************************************
 * \brief  To protect I2C communication
 *
 * This method acquire a mutex and shall be used before communication takes
 * place.
 *
 *****************************************************************************
 */
void pltf_protect_com(void);

/*!
 *****************************************************************************
 * \brief  To unprotect I2C communication
 *
 * This method release the mutex that was acquired with pltf_protect_com.
 *
 *****************************************************************************
 */
void pltf_unprotect_com(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_I2C */
