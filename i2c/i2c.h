/*
 * i2c.h
 *
 * Created: 11/11/2014 11:37:06 AM
 *  Author: emiva760, geoza435, carth567
 */


#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <common.h>

#define REQUEST_TO_READ TW_SR_SLA_ACK
#define REQUEST_TO_WRITE TW_ST_SLA_ACK

// Förebereder sensorenhetens data för överföring.
Data prepare_data(void);
// Tolkar mottagen data från bussen.
void interpret_data(Data data);

void master_init(uint32_t f_cpu, uint32_t bitrate);
void slave_init(Bus_data_union(*prepare_data)(), void(*interpret_data)(Bus_data_union), uint8_t slave_address);

/* Hämta från slav */
void master_receive(uint8_t slave_address, Data *data);

/* Skicka till slav */
void master_transmit(uint8_t slave_address, Data data);

/* Skicka till master */
void slave_transmit(Data data);

/* Hämta från master */
Data slave_receive();

#endif /* I2C_H_ */
