/*
 * modulkom.h
 *
 * Created: 11/19/2014 2:15:51 PM
 *  Author: emiva760, carth567
 */ 

#ifndef MODULKOM_H_
#define MODULKOM_H_

#include <common.h>

/************************************************************************/
/* Bussfunktioner                                                       */
/************************************************************************/

/* Initieringsfunktioner */
void set_as_slave(Bus_data(*prepare_data)(), void(*interpret_data)(Bus_data), Device_id id);
void set_as_master(uint32_t f_cpu);

/* Kommunikationsfunktioner för master */
void send_data(Device_id id, Bus_data bus_data);
void fetch_data(Device_id id, Bus_data *bus_data);

#endif /* MODULKOM_H_ */