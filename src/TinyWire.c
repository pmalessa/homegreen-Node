/*
 * TinyWire.c
 *
 *  Created on: 13.02.2019
 *      Author: pmale
 */

#include "PLATFORM.h"
#include "TinyWire.h"
#include "I2C_Master.h"

uint8_t rxBuffer[BUFFER_LENGTH];
uint8_t rxBufferIndex;
uint8_t rxBufferLength;
uint8_t transmitting;
uint8_t error;

void TinyWire_begin() {
    rxBufferIndex = 0;
    rxBufferLength = 0;
    error = 0;
    transmitting = false;
    i2c_init();
}

void TinyWire_end() {}

void TinyWire_beginTransmission(uint8_t address) {
    if (transmitting) error = (i2c_rep_start((address << 1) | I2C_WRITE) ? 0 : 2);
    else {
        i2c_start_wait((address << 1) | I2C_WRITE);
        error = 0;
    }
    transmitting = 1;
}

uint8_t TinyWire_endTransmission(uint8_t sendStop) {
    uint8_t transError = error;
    if (sendStop) {
        i2c_stop();
        transmitting = 0;
    }
    error = 0;
    return transError;
}

uint8_t TinyWire_writeOne(uint8_t data) {
    if (i2c_write(data)) return 1;
    else {
        if (error == 0) error = 3;
        return 0;
    }
}

uint8_t TinyWire_write(const uint8_t *data, uint8_t quantity) {
	uint8_t trans = 0;
    for (uint8_t i = 0; i<quantity; i++) trans += TinyWire_writeOne(data[i]);
    return trans;
}

uint8_t TinyWire_requestFrom(uint8_t address,uint8_t quantity,uint32_t iaddress,uint8_t isize,uint8_t sendStop) {
    uint8_t localerror = 0;
    if (isize > 0) {
    	TinyWire_beginTransmission(address);
        if (isize > 3) isize = 3;
        while (isize-- > 0) TinyWire_writeOne((uint8_t)(iaddress >> (isize * 8)));
        TinyWire_endTransmission(false);
    }
    if (quantity > BUFFER_LENGTH) quantity = BUFFER_LENGTH;
    localerror = !i2c_rep_start((address<<1) | I2C_READ);
    if (error == 0 && localerror) error = 2;
    for (uint8_t cnt=0; cnt < quantity; cnt++) rxBuffer[cnt] = i2c_read(cnt == quantity-1);
    rxBufferIndex = 0;
    rxBufferLength = quantity;
    if (sendStop) {
        transmitting = 0;
        i2c_stop();
    }
    return quantity;
}

int TinyWire_available() {
    return (rxBufferLength - rxBufferIndex);
}

int TinyWire_read() {
int value = -1;
    if(rxBufferIndex < rxBufferLength) {
        value = rxBuffer[rxBufferIndex];
        ++rxBufferIndex;
    }
    return value;
}

int TinyWire_peek() {
    int value = -1;
    if (rxBufferIndex < rxBufferLength) value = rxBuffer[rxBufferIndex];
    return value;
}
