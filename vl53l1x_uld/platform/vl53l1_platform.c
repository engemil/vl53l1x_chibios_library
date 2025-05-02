/**
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "vl53l1_platform.h"
#include <string.h>
#include <time.h>
#include <math.h>

// Static variable to hold the I2C driver pointer
static I2CDriver *VL53L1X_I2C_DRIVER = NULL;

void VL53L1_SetI2CDriver(I2CDriver *driver){
    VL53L1X_I2C_DRIVER = driver;
}

// Static variable to hold the I2C driver pointer
static SerialDriver *VL53L1X_SERIAL_DRIVER = NULL;

void VL53L1_SetSerialDriver(SerialDriver *driver){
    VL53L1X_SERIAL_DRIVER = driver;
}


int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    msg_t status;
    uint8_t tx_buf[2U + count];
    
    // Prepare index (2 bytes, MSB first)
    tx_buf[0U] = (uint8_t)(index >> 8U);
    tx_buf[1U] = (uint8_t)(index & 0xFF);

    // Copy data to transmit buffer
    memcpy(&tx_buf[2U], pdata, count);

    // Acquire I2C bus
    i2cAcquireBus(VL53L1X_I2C_DRIVER);

    // Send index and data
    status = i2cMasterTransmitTimeout(VL53L1X_I2C_DRIVER, dev, tx_buf, 2U + count, NULL, 0U, TIME_MS2I(100U));
    
    i2cReleaseBus(VL53L1X_I2C_DRIVER);
    
    return (status == MSG_OK) ? 0 : -1;
}

int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count){
    
    msg_t status;
    uint8_t index_buf[2U] = {(uint8_t)(index >> 8U), (uint8_t)(index & 0xFF)};

    // Acquire I2C bus
    i2cAcquireBus(VL53L1X_I2C_DRIVER);

    // Send register address (2 bytes)
    status = i2cMasterTransmitTimeout(VL53L1X_I2C_DRIVER, dev, index_buf, 2, NULL, 0, TIME_MS2I(100U));
    if (status != MSG_OK) {
        i2cReleaseBus(VL53L1X_I2C_DRIVER);
        return -1; // Error
    }

    // Read data
    status = i2cMasterReceiveTimeout(VL53L1X_I2C_DRIVER, dev, pdata, count, TIME_MS2I(100U));
    i2cReleaseBus(VL53L1X_I2C_DRIVER);

    return (status == MSG_OK) ? 0 : -1;
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data) {
	return VL53L1_WriteMulti(dev, index, &data, 1U);
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data) {
    uint8_t buf[2U] = {(uint8_t)(data >> 8U), (uint8_t)(data & 0xFF)}; // MSB first
    return VL53L1_WriteMulti(dev, index, buf, 2U);
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data) {
    uint8_t buf[4U] = {
        (uint8_t)(data >> 24U),
        (uint8_t)((data >> 16U) & 0xFF),
        (uint8_t)((data >> 8U) & 0xFF),
        (uint8_t)(data & 0xFF)
    }; // MSB first
    return VL53L1_WriteMulti(dev, index, buf, 4U);
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data) {
	return VL53L1_ReadMulti(dev, index, data, 1U);
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data) {
    uint8_t buf[2U];
    int8_t status = VL53L1_ReadMulti(dev, index, buf, 2U);
    if (status == 0) {
        *data = ((uint16_t)buf[0U] << 8) | buf[1U]; // Combine MSB and LSB
    }
    return status;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data) {
    uint8_t buf[4U];
    int8_t status = VL53L1_ReadMulti(dev, index, buf, 4U);
    if (status == 0) {
        *data = ((uint32_t)buf[0U] << 24U) |
                ((uint32_t)buf[1U] << 16U) |
                ((uint32_t)buf[2U] << 8U)  |
                (uint32_t)buf[3U]; // Combine bytes, MSB first
    }
    return status;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms){
    (void)dev; // dev parameter not used
    if (wait_ms < 0) {
        return -1; // Invalid wait time
    }
    chThdSleepMilliseconds(wait_ms);
	return 0; // to be implemented
}

