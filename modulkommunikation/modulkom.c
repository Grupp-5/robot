/*
 * modulkom.c
 *
 * Created: 11/19/2014 2:34:12 PM
 *  Author: emiva760, carth567
 */ 

#include "i2c.h"
#include "modulkom.h"



/*void set_as_slave(Data(*prepare_data)(), void(*interpret_data)(Data), Device_id id) {
	slave_init(prepare_data, interpret_data, id);
}*/
void set_as_slave(Bus_data(*prepare_data)(), void(*interpret_data)(Bus_data), Device_id id) {
	slave_init((Bus_data_union(*)(void))prepare_data, (void(*)(Bus_data_union))interpret_data, id);
}


void set_as_master(uint32_t f_cpu) {
	master_init(f_cpu, SCL_CLOCK);
}

void send_data(Device_id id, Bus_data bus_data) {
	master_transmit(id, ((Bus_data_union)bus_data).data);
}

void fetch_data(Device_id id, Bus_data *bus_data) {
	master_receive(id, &((Bus_data_union*)bus_data)->data);
}