/*
 * i2c.h
 *
 *  Created on: 2015Äê4ÔÂ24ÈÕ
 *      Author: ideapad
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdbool.h>
#include <stdint.h>

void InitI2C1(void);
uint8_t readI2C1(uint16_t device_address,
                 uint16_t device_register);
int writeI2C1(uint8_t slave_addr,
              uint8_t reg_addr,
              uint8_t data);
int8_t IICreadBytes(uint8_t slave_addr,
                 uint8_t reg_addr,
                 uint8_t length,
                 uint8_t *data);



#endif /* I2C_H_ */
