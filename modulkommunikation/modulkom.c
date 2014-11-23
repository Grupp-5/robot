/*
 * modulkom.c
 *
 * Created: 11/19/2014 2:34:12 PM
 *  Author: emiva760, carth567
 */ 

#include "i2c.h"
#include "modulkom.h"

void set_as_slave(Bus_data(*prepare_data)(), void(*interpret_data)(Bus_data), data_id id) {
	slave_init(
		// Gör om `Bus_data prepare_data()` pekaren till en funktion
		// som returnerar Data
		(Data (*)()) prepare_data,
		// Gör om `void interpret_data(Bus_data)` pekaren till en
		// funktion som tar Data som inparameter
		(void (*)(Data)) interpret_data,
		id
   );
}


void set_as_master(uint32_t f_cpu) {
	master_init(f_cpu, SCL_CLOCK);
}

typedef union {
	Bus_data bus_data;
	Data data;
} Bus_data_union;

void send_data(device_id id, Bus_data bus_data) {
	master_transmit(id, ((Bus_data_union)bus_data).data);
}

void fetch_data(device_id id, Bus_data *bus_data) {
	master_receive(id, &((Bus_data_union*)bus_data)->data);
}