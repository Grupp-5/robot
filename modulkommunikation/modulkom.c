/*
 * modulkom.c
 *
 * Created: 11/19/2014 2:34:12 PM
 *  Author: emiva760, carth567, geoza435
 */

#include <twi.h>
#include "modulkom.h"

void(*interpret_data_cb)(Bus_data);
Bus_data(*prepare_data_cb)();

void slave_rx_callback(uint8_t *data, uint16_t len) {
	// TODO: Gör nåt med len, kolla om det får plats? Jag vet inte.
	// TODO: Kommer ju aldrig funka utan att spara undan saker nånstans..
	interpret_data_cb((Bus_data)*((Bus_data*) data));
}

void slave_tx_callback() {
	static Bus_data data = {0};
	data = prepare_data_cb();
	// TODO: data.count + 2
	twi_transmit((uint8_t*) &data, data.count);
}

void set_as_slave(uint32_t f_cpu, Bus_data(*prepare_data)(), void(*interpret_data)(Bus_data), device_id id) {
	twi_set_slave_address(id);
	twi_init(f_cpu);

	// Slave Receive
	interpret_data_cb = interpret_data;
	twi_attach_slave_rx_callback(slave_rx_callback);

	//Slave Transmit
	prepare_data_cb = prepare_data;
	twi_attach_slave_tx_callback(slave_tx_callback);
}

/************************************************************************/
/*                        MASTER MODE                                   */
/************************************************************************/
void set_as_master(uint32_t f_cpu) {
	//master_init(f_cpu, SCL_CLOCK);
	twi_init(f_cpu);
}

void send_data(device_id id, Bus_data bus_data) {
	// TODO, antingen kolla i command_lengths efter count,
	// eller lägg till parameter med count.
	// TODO: count + 2
	static Bus_data data = {0};
	data = bus_data;
	twi_write_to(id, (uint8_t*) &data, bus_data.count, TWI_BLOCK, TWI_STOP);
}

void fetch_data(device_id id, Bus_data *bus_data) {
	// TODO, antingen kolla i command_lengths efter count,
	// eller lägg till parameter med count.
	// TODO: count + 2
	twi_read_from(id, (uint8_t*) bus_data, bus_data->count, TWI_STOP);
}
