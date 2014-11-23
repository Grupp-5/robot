/*
 * modulkom.h
 *
 * Created: 11/19/2014 2:15:51 PM
 *  Author: emiva760, carth567
 */ 

#ifndef MODULKOM_H_
#define MODULKOM_H_

// Om MAX_DATA ändras måste även I2C_MAX_DATA ändras i i2c.h
#define MAX_DATA 20 // maximalt antal data-bytes
#define SCL_CLOCK  100000L // "bitrate"

typedef uint8_t data_id ;
typedef uint8_t device_id;

typedef	struct {	// Data som ska skickas på bussen
	uint8_t count;
	data_id id;
	uint8_t data[MAX_DATA - 1];
} Bus_data;

/************************************************************************/
/* Bussfunktioner                                                       */
/************************************************************************/

/* Initieringsfunktioner */
void set_as_slave(Bus_data(*prepare_data)(), void(*interpret_data)(Bus_data), data_id id);
void set_as_master(uint32_t f_cpu);

/* Kommunikationsfunktioner för master */
void send_data(device_id id, Bus_data bus_data);
void fetch_data(device_id id, Bus_data *bus_data);

#endif /* MODULKOM_H_ */