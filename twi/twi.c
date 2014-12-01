﻿/*
	twi.c - TWI/I2C library for Wiring & Arduino
	Copyright (c) 2006 Nicholas Zambetti.	All right reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA	02110-1301	USA

	Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
	Modified 2012 by Wyatt Olson (wyatt@digitalcave.ca) for improvements as a standalone
	library (instead of part of TwoWire Arduino library)
*/

#include <avr/interrupt.h>
#include <compat/twi.h>
#include <stdbool.h>

#include "twi.h"

static volatile uint8_t twi_state;
static volatile uint8_t twi_slarw;
static volatile uint8_t twi_send_stop;			// should the transaction end with a stop
static volatile uint8_t twi_inRepStart;			// in the middle of a repeated start

#ifndef TWI_DISABLE_MASTER
	#ifdef TWI_MASTER_RX_READER
		static void (*twi_master_rx_reader)(uint8_t, uint16_t);
	#endif
	#ifdef TWI_MASTER_TX_WRITER
		static uint8_t (*twi_master_tx_writer)(uint16_t);
	#endif

	#ifdef TWI_CUSTOM_BUFFERS
		static uint8_t* twi_masterBuffer;
	#else
		static uint8_t twi_masterBuffer[TWI_BUFFER_LENGTH];
	#endif
	
	
	static volatile uint16_t twi_masterBufferIndex;
	static volatile uint16_t twi_masterBufferLength;
#endif

#ifndef TWI_DISABLE_SLAVE
	#ifndef TWI_DISABLE_SLAVE_TX
		#ifdef TWI_SLAVE_TX_WRITER
			//Callback to assemble the message one byte at a time.
			static uint8_t (*twi_slave_tx_writer)(uint16_t);
		#else
			//Callback to assemble the buffer, and send it via twi_transmit().
			static void (*twi_slave_tx_callback)(void);
		#endif

		#ifdef TWI_CUSTOM_BUFFERS
			static uint8_t* twi_txBuffer;
		#else
			static uint8_t twi_txBuffer[TWI_BUFFER_LENGTH];
		#endif
		static volatile uint16_t twi_txBufferIndex;
		static volatile uint16_t twi_txBufferLength;
	#endif
	
	#ifndef TWI_DISABLE_SLAVE_RX
		#ifdef TWI_SLAVE_RX_READER
			static void (*twi_slave_rx_reader)(uint8_t, uint16_t);
		#else
			static void (*twi_slave_rx_callback)(uint8_t*, uint16_t);
		#endif
		
		#ifdef TWI_CUSTOM_BUFFERS
			static uint8_t* twi_rxBuffer;
		#else
			static uint8_t twi_rxBuffer[TWI_BUFFER_LENGTH];
		#endif

		static volatile uint16_t twi_rxBufferIndex;
	#endif
#endif

static volatile uint8_t twi_error;

#if !defined(TWI_DISABLE_MASTER) && defined(TWI_CUSTOM_BUFFERS)
	void twi_set_master_buffer(uint8_t* buffer){
		twi_masterBuffer = buffer;
	}
#endif
#if !defined(TWI_DISABLE_SLAVE) && !defined(TWI_DISABLE_SLAVE_RX) && defined(TWI_CUSTOM_BUFFERS)
	void twi_set_rx_buffer(uint8_t* buffer){
		twi_rxBuffer = buffer;
	}
#endif
#if !defined(TWI_DISABLE_SLAVE) && !defined(TWI_DISABLE_SLAVE_TX) && defined(TWI_CUSTOM_BUFFERS)
	void twi_set_tx_buffer(uint8_t* buffer){
		twi_txBuffer = buffer;
	}
#endif





#if !defined(TWI_DISABLE_SLAVE) && !defined(TWI_DISABLE_SLAVE_RX) && defined(TWI_SLAVE_RX_READER)
	void twi_attach_slave_rx_reader( void (*function)(uint8_t, uint16_t) ){
		twi_slave_rx_reader = function;
	}
#endif

#if !defined(TWI_DISABLE_SLAVE) && !defined(TWI_DISABLE_SLAVE_RX) && defined(TWI_SLAVE_TX_WRITER)
	void twi_attach_slave_tx_writer( uint8_t (*function)(uint16_t) ){
		twi_slave_tx_writer = function;
	}
#endif

#if !defined(TWI_DISABLE_MASTER) && defined(TWI_MASTER_RX_READER)
	void twi_attach_master_rx_reader( void (*function)(uint8_t, uint16_t) ){
		twi_master_rx_reader = function;
	}
#endif
#if !defined(TWI_DISABLE_MASTER) && defined(TWI_MASTER_TX_WRITER)
	void twi_attach_master_tx_writer( uint8_t (*function)(uint16_t) ){
		twi_master_tx_writer = function;
	}
#endif

void twi_init(uint32_t F_CPU){
	// initialize state
	twi_state = TWI_READY;
	twi_send_stop = true;		// default value
	twi_inRepStart = false;
	
	// initialize twi prescaler and bit rate
	TWSR &= ~_BV(TWPS0);
	TWSR &= ~_BV(TWPS1);
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

	//See TWI bit rate formula from atmega128 manual pg 204
	// SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
	// note: TWBR should be 10 or higher for master mode
	// It is 72 for a 16mhz Wiring board with 100kHz TWI 

	// enable twi module, acks, and twi interrupt
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
	
	//Enable interrupts if the NO_INTERRUPT_ENABLE define is not set.	If it is, you need to call sei() elsewhere.
#ifndef NO_INTERRUPT_ENABLE
	sei();
#endif
}

#if !defined(TWI_DISABLE_MASTER)
	uint8_t twi_read_from(uint8_t address, uint8_t* data, uint16_t length, uint8_t send_stop){
		uint16_t i;
	
		// ensure data will fit into buffer
		if(TWI_BUFFER_LENGTH < length){
			return 0;
		}
	
		// block until twi is ready, become master receiver
		while(TWI_READY != twi_state){
			continue;
		}
		twi_state = TWI_MRX;
		twi_send_stop = send_stop;
		// reset error state (0xFF.. no error occured)
		twi_error = 0xFF;
	
		// initialize buffer iteration vars
		twi_masterBufferIndex = 0;
		twi_masterBufferLength = length-1;	// This is not intuitive, read on...
		// On receive, the previously configured ACK/NACK setting is transmitted in
		// response to the received byte before the interrupt is signalled. 
		// Therefor we must actually set NACK when the _next_ to last byte is
		// received, causing that NACK to be sent in response to receiving the last
		// expected byte of data.
	
		// build sla+w, slave device address + w bit
		twi_slarw = TW_READ;
		twi_slarw |= address << 1;
	
		if (true == twi_inRepStart) {
			// if we're in the repeated start state, then we've already sent the start,
			// (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
			// We need to remove ourselves from the repeated start state before we enable interrupts,
			// since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
			// up. Also, don't enable the START interrupt. There may be one pending from the 
			// repeated start that we sent outselves, and that would really confuse things.
			twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
			TWDR = twi_slarw;
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);	// enable INTs, but not START
		}
		else
			// send start condition
			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);
	
		// wait for read operation to complete
		while(TWI_MRX == twi_state){
			continue;
		}
	
		if (twi_masterBufferIndex < length)
			length = twi_masterBufferIndex;
	
		// copy twi buffer to data
		for(i = 0; i < length; ++i){
			data[i] = twi_masterBuffer[i];
		}
		
		return length;
	}
#endif

#if !defined(TWI_DISABLE_MASTER)
	uint8_t twi_write_to(uint8_t address, uint8_t* data, uint16_t length, uint8_t block, uint8_t send_stop){
		// ensure data will fit into buffer
		if(TWI_BUFFER_LENGTH < length){
			return 1;
		}
	
		// wait until twi is ready, become master transmitter
		while(TWI_READY != twi_state){
			continue;
		}
		twi_state = TWI_MTX;
		twi_send_stop = send_stop;
		// reset error state (0xFF.. no error occured)
		twi_error = 0xFF;
	
		// initialize buffer iteration vars
		twi_masterBufferIndex = 0;
		twi_masterBufferLength = length;
		
		//If we have custom buffers we don't need to copy the data.
		#if !defined(TWI_CUSTOM_BUFFERS)
			// copy data to twi buffer
			for(uint16_t i = 0; i < length; ++i){
				twi_masterBuffer[i] = data[i];
			}
		#endif
		
		// build sla+w, slave device address + w bit
		twi_slarw = TW_WRITE;
		twi_slarw |= address << 1;
		
		// if we're in a repeated start, then we've already sent the START
		// in the ISR. Don't do it again.
		//
		if (true == twi_inRepStart) {
			// if we're in the repeated start state, then we've already sent the start,
			// (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
			// We need to remove ourselves from the repeated start state before we enable interrupts,
			// since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
			// up. Also, don't enable the START interrupt. There may be one pending from the 
			// repeated start that we sent outselves, and that would really confuse things.
			twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
			TWDR = twi_slarw;				
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);	// enable INTs, but not START
		}
		else
			// send start condition
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);	// enable INTs
	
		// wait for write operation to complete
		while(block && (TWI_MTX == twi_state)){
			continue;
		}
		
		if (twi_error == 0xFF)
			return 0;	// success
		else if (twi_error == TW_MT_SLA_NACK)
			return 2;	// error: address send, nack received
		else if (twi_error == TW_MT_DATA_NACK)
			return 3;	// error: data send, nack received
		else
			return 4;	// other twi error
	}
#endif





#if !defined(TWI_DISABLE_SLAVE)
	void twi_set_slave_address(uint8_t address){
		// set twi slave address (skip over TWGCE bit)
		TWAR = address << 1;
	}
#endif


#if !defined(TWI_DISABLE_SLAVE) && !defined(TWI_DISABLE_SLAVE_TX)
	uint8_t twi_transmit(const uint8_t* data, uint16_t length){
		uint16_t i;
	
		// ensure data will fit into buffer
		if(TWI_BUFFER_LENGTH < length){
			return 1;
		}
		
		// ensure we are currently a slave transmitter
		if(TWI_STX != twi_state){
			return 2;
		}
		
		// set length and copy data into tx buffer
		twi_txBufferLength = length;
		for(i = 0; i < length; ++i){
			twi_txBuffer[i] = data[i];
		}
		
		return 0;
	}
#endif


#if !defined(TWI_DISABLE_SLAVE) && !defined(TWI_DISABLE_SLAVE_TX) && !defined(TWI_SLAVE_TX_WRITER)
	void twi_attach_slave_tx_callback( void (*function)(void) ){
		twi_slave_tx_callback = function;
	}
#endif

#if !defined(TWI_DISABLE_SLAVE) && !defined(TWI_DISABLE_SLAVE_RX) && !defined(TWI_SLAVE_RX_READER)
	void twi_attach_slave_rx_callback( void (*function)(uint8_t*, uint16_t) ){
		twi_slave_rx_callback = function;
	}
#endif

static void twi_reply(uint8_t ack){
	// transmit master read ready signal, with or without ack
	if(ack){
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
	}else{
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
	}
}

static void twi_stop(void){
	// send stop condition
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);

	// wait for stop condition to be exectued on bus
	// TWINT is not set after a stop condition!
	while(TWCR & _BV(TWSTO)){
		continue;
	}

	// update twi state
	twi_state = TWI_READY;
}

static void twi_release_bus(void){
	// release bus
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);

	// update twi state
	twi_state = TWI_READY;
}

ISR(TWI_vect){
	switch(TW_STATUS){
		// All Master
		case TW_START:		 // sent start condition
		case TW_REP_START: // sent repeated start condition
			// copy device address and r/w bit to output register and ack
			TWDR = twi_slarw;
			twi_reply(1);
			break;

	#ifndef TWI_DISABLE_MASTER
		// Master Transmitter
		case TW_MT_SLA_ACK:	// slave receiver acked address
		case TW_MT_DATA_ACK: // slave receiver acked data
			// if there is data to send, send it, otherwise stop 
			if(twi_masterBufferIndex < twi_masterBufferLength){
#ifdef TWI_MASTER_TX_WRITER
				// call the master tx writer function to get the byte to send
				TWDR = twi_master_tx_writer(twi_masterBufferIndex++);
#else
				// copy data from buffer to output register
				TWDR = twi_masterBuffer[twi_masterBufferIndex++];
#endif
				twi_reply(1);
			} 
			else{
				if (twi_send_stop) {
					twi_stop();
				}
				else {
					twi_inRepStart = true;	// we're gonna send the START
					// don't enable the interrupt. We'll generate the start, but we 
					// avoid handling the interrupt until we're in the next transaction,
					// at the point where we would normally issue the start.
					TWCR = _BV(TWINT) | _BV(TWSTA)| _BV(TWEN) ;
					twi_state = TWI_READY;
				}
			}
			break;
		case TW_MT_SLA_NACK:	// address sent, nack received
			twi_error = TW_MT_SLA_NACK;
			twi_stop();
			break;
		case TW_MT_DATA_NACK: // data sent, nack received
			twi_error = TW_MT_DATA_NACK;
			twi_stop();
			break;
		case TW_MT_ARB_LOST: // lost bus arbitration
			twi_error = TW_MT_ARB_LOST;
			twi_release_bus();
			break;

		// Master Receiver
		case TW_MR_DATA_ACK: // data received, ack sent
#ifdef TWI_MASTER_RX_READER
			// pass data to master rx reader function, to handle as appropriate.
			twi_slave_rx_reader(TWDR, twi_masterBufferIndex++);
#else
			// put byte into buffer
			twi_masterBuffer[twi_masterBufferIndex++] = TWDR;
#endif
		case TW_MR_SLA_ACK:	// address sent, ack received
			// ack if more bytes are expected, otherwise nack
			if(twi_masterBufferIndex < twi_masterBufferLength){
				twi_reply(1);
			}else{
				twi_reply(0);
			}
			break;
		case TW_MR_DATA_NACK: // data received, nack sent
			// put final byte into buffer
			twi_masterBuffer[twi_masterBufferIndex++] = TWDR;
			if (twi_send_stop) {
				twi_stop();
			}
			else {
				twi_inRepStart = true;	// we're gonna send the START
				// don't enable the interrupt. We'll generate the start, but we 
				// avoid handling the interrupt until we're in the next transaction,
				// at the point where we would normally issue the start.
				TWCR = _BV(TWINT) | _BV(TWSTA)| _BV(TWEN) ;
				twi_state = TWI_READY;
			}		
			break;
		case TW_MR_SLA_NACK: // address sent, nack received
			twi_stop();
			break;
		// TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case
	#endif

	#ifndef TWI_DISABLE_SLAVE
	#ifndef TWI_DISABLE_SLAVE_RX
		// Slave Receiver
		case TW_SR_SLA_ACK:	 // addressed, returned ack
		case TW_SR_GCALL_ACK: // addressed generally, returned ack
		case TW_SR_ARB_LOST_SLA_ACK:	 // lost arbitration, returned ack
		case TW_SR_ARB_LOST_GCALL_ACK: // lost arbitration, returned ack
			// enter slave receiver mode
			twi_state = TWI_SRX;
			// indicate that rx buffer can be overwritten and ack
			twi_rxBufferIndex = 0;
			twi_reply(1);
			break;
		case TW_SR_DATA_ACK:			 // data received, returned ack
		case TW_SR_GCALL_DATA_ACK: // data received generally, returned ack
			// if there is still room in the rx buffer
			if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
	#ifdef TWI_SLAVE_RX_READER
				//Call custom callback function to handle the byte, and ack
				twi_slave_rx_reader(TWDR, twi_rxBufferIndex++);
	#else
				// put byte in buffer
				twi_rxBuffer[twi_rxBufferIndex++] = TWDR;
	#endif
			twi_reply(1);
			}else{
				// otherwise nack
				twi_reply(0);
			}
			break;
		case TW_SR_STOP: // stop or repeated start condition received
			// put a null char after data if there's room
			if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
				twi_rxBuffer[twi_rxBufferIndex] = '\0';
			}
			// sends ack and stops interface for clock stretching
			twi_stop();
		#ifndef TWI_SLAVE_RX_READER
			// callback to user defined callback
			twi_slave_rx_callback(twi_rxBuffer, twi_rxBufferIndex);
		#endif
			// since we submit rx buffer to "wire" library, we can reset it
			twi_rxBufferIndex = 0;
			// ack future responses and leave slave receiver state
			twi_release_bus();
			break;
		case TW_SR_DATA_NACK:			 // data received, returned nack
		case TW_SR_GCALL_DATA_NACK: // data received generally, returned nack
			// nack back at master
			twi_reply(0);
			break;
	#endif
		
	#ifndef TWI_DISABLE_SLAVE_TX
		// Slave Transmitter
		case TW_ST_SLA_ACK:					// addressed, returned ack
		case TW_ST_ARB_LOST_SLA_ACK: // arbitration lost, returned ack
			// enter slave transmitter mode
			twi_state = TWI_STX;
			// ready the tx buffer index for iteration
			twi_txBufferIndex = 0;
			// set tx buffer length to be zero, to verify if user changes it
			twi_txBufferLength = 0;
		#ifndef TWI_SLAVE_TX_WRITER
			// request for txBuffer to be filled and length to be set
			// note: user must call twi_transmit(bytes, length) to do this
			twi_slave_tx_callback();
		#endif
			// if they didn't change buffer & length, initialize it
			if(0 == twi_txBufferLength){
				twi_txBufferLength = 1;
				twi_txBuffer[0] = 0x00;
			}
			// transmit first byte from buffer, fall
		case TW_ST_DATA_ACK: // byte sent, ack returned
	#ifdef TWI_SLAVE_TX_WRITER
			uint8_t reply = 0x00;
			TWDR = twi_slave_tx_writer(&reply, twi_txBufferIndex++);
			twi_reply(reply);
	#else
			// copy data to output register
			TWDR = twi_txBuffer[twi_txBufferIndex++];
			// if there is more to send, ack, otherwise nack
			if(twi_txBufferIndex < twi_txBufferLength){
				twi_reply(1);
			}else{
				twi_reply(0);
			}
	#endif
			break;
		case TW_ST_DATA_NACK: // received nack, we are done 
		case TW_ST_LAST_DATA: // received ack, but we are done already!
			// ack future responses
			twi_reply(1);
			// leave slave receiver state
			twi_state = TWI_READY;
			break;
	#endif
	#endif
	
		// All
		case TW_NO_INFO:	 // no state information
			break;
		case TW_BUS_ERROR: // bus error, illegal stop/start
			twi_error = TW_BUS_ERROR;
			twi_stop();
			break;
	}
}

