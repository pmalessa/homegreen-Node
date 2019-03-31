/*
 * I2C_Master.h
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#define SDA_PORT PORTA
#define SDA_PIN PORTA6

#define SCL_PORT PORTA
#define SCL_PIN PORTA4

#define I2C_TIMEOUT 100

#define I2C_SLOWMODE 1

#ifndef I2C_CPUFREQ
#define I2C_CPUFREQ F_CPU
#endif

#ifndef I2C_FASTMODE
#define I2C_FASTMODE 0
#endif

#ifndef I2C_SLOWMODE
#define I2C_SLOWMODE 0
#endif

#ifndef I2C_NOINTERRUPT
#define I2C_NOINTERRUPT 0
#endif

#ifndef I2C_TIMEOUT
#define I2C_TIMEOUT 0
#else
#if I2C_TIMEOUT > 10000
#error I2C_TIMEOUT is too large
#endif
#endif

#define I2C_TIMEOUT_DELAY_LOOPS (I2C_CPUFREQ / 1000UL) * I2C_TIMEOUT / 4000UL
#if I2C_TIMEOUT_DELAY_LOOPS < 1
#define I2C_MAX_STRETCH 1
#else
#if I2C_TIMEOUT_DELAY_LOOPS > 60000UL
#define I2C_MAX_STRETCH 60000UL
#else
#define I2C_MAX_STRETCH I2C_TIMEOUT_DELAY_LOOPS
#endif
#endif

#if I2C_FASTMODE
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ / 350000L) / 2 - 18) / 3)
#else
#if I2C_SLOWMODE
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ / 23500L) / 2 - 18) / 3)
#else
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ / 90000L) / 2 - 18) / 3)
#endif
#endif

#define I2C_READ    1
#define I2C_WRITE   0

#define SDA_DDR  (_SFR_IO_ADDR(SDA_PORT) - 1)
#define SCL_DDR  (_SFR_IO_ADDR(SCL_PORT) - 1)
#define SDA_OUT   _SFR_IO_ADDR(SDA_PORT)
#define SCL_OUT   _SFR_IO_ADDR(SCL_PORT)
#define SDA_IN   (_SFR_IO_ADDR(SDA_PORT) - 2)
#define SCL_IN   (_SFR_IO_ADDR(SCL_PORT) - 2)

#ifndef __tmp_reg__
#define __tmp_reg__ 0
#endif

bool __attribute__ ((noinline)) i2c_init() __attribute__ ((used));

bool __attribute__ ((noinline)) i2c_start(uint8_t addr) __attribute__ ((used));

void  __attribute__ ((noinline)) i2c_start_wait(uint8_t addr) __attribute__ ((used));

bool __attribute__ ((noinline)) i2c_rep_start(uint8_t addr) __attribute__ ((used));

void __attribute__ ((noinline)) i2c_stop() asm("ass_i2c_stop") __attribute__ ((used));

bool __attribute__ ((noinline)) i2c_write(uint8_t value) asm("ass_i2c_write") __attribute__ ((used));

uint8_t __attribute__ ((noinline)) i2c_read(bool last) __attribute__ ((used));

void __attribute__ ((noinline)) i2c_delay_half() asm("ass_i2c_delay_half")  __attribute__ ((used));

void __attribute__ ((noinline)) i2c_wait_scl_high() asm("ass_i2c_wait_scl_high")  __attribute__ ((used));

#endif /* I2C_MASTER_H_ */
