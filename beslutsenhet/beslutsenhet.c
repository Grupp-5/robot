/*
 * beslutsenhet.c
 *
 * Created: 2014-11-13 14:10:27
 *  Author: erima694, eribo740, geoza435
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <common.h>
#include <modulkom.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

//Set CPU clock
#define F_CPU 8000000UL
#include <util/delay.h>

volatile uint8_t autoMode;
volatile uint8_t makeDecisionFlag;
volatile uint8_t pdFlag;
volatile uint8_t turn;
volatile uint8_t send_stop = false;

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};
Bus_data master_data_to_send = {0};
Bus_data master_data_to_receive = {0};

void send_move_data(double forward, double side, double turn) {
	Move_data move_data;
	move_data.count = command_lengths[MOVE]+2;
	move_data.id = MOVE;
	move_data.forward_speed = forward;
	move_data.side_speed = side;
	move_data.turn_speed = turn;
	send_data(which_device[MOVE], move_data.bus_data);
}

Sensor_data getSensorData() {
	volatile Sensor_data sensor_data;
	master_data_to_receive.count = command_lengths[SENSOR_DATA]+2;
	fetch_data(SENSOR, &master_data_to_receive);
	sensor_data = (Sensor_data)master_data_to_receive;
	return sensor_data;
}

void waitForCorrectValues()
{
	volatile Sensor_data sensor_data;
	do
	{
		sensor_data = getSensorData();
		_delay_ms(20);
		
	} while (sensor_data.bl + sensor_data.br > 75 && !send_stop);
}

double MAX_ADJUSTMENT = 0.5; //Constant to stop the robot from turning like crazy
double P = 0.005; // Constant for the proportional part
double D = 0.02;  // Constant for the derivative part

#define ERROR_COUNT 10

double delta_t = 0.05;

double errors[ERROR_COUNT] = {0};
uint8_t current_error = 0; // Nuvarande error i errors-arrayen
double prevError = 0; //The previous error

double error_mean() {
	double sum = 0;
	for (int i = 0; i < ERROR_COUNT; i++) {
		sum += errors[i];
	}
	return sum/ERROR_COUNT;
}

void nextError() {
	current_error = (current_error + 1) % ERROR_COUNT;
}

void cleanOldErrors() {
	for (uint8_t i = 0; i < ERROR_COUNT; i++) {
		errors[i] = 0;
	}
}

void pdAlgoritm(double distanceRight, double distanceLeft) {
	errors[current_error] = distanceRight - distanceLeft;

	double error = error_mean();

	double d_adjustment, p_adjustment, adjustment = 0;

	d_adjustment = D*(error - prevError)/delta_t;
	p_adjustment = P*errors[current_error];
	nextError();

	adjustment = p_adjustment + d_adjustment;
	adjustment = fmax(fmin(adjustment, MAX_ADJUSTMENT), -MAX_ADJUSTMENT);

	prevError = error;

	PD_Data pd_data = {
		.id = PD_DATA,
		.count = command_lengths[PD_DATA] + 2,
		.error = error,
		.p = p_adjustment,
		.d = d_adjustment,
		.adjustment = adjustment
	};

	send_data(COMMUNICATION, pd_data.bus_data);

	send_move_data(0.6, adjustment*1.2, adjustment);
}

// Blockerar main och snurrar DEG grader samtidigt som den går framåt
// P-reglerad, D behövdes tydligen inte här.
void turnTo(double deg, double forward_speed) {
	double recordedDeg = getSensorData().gyro;
	double goalDeg = recordedDeg - deg;
	double deg_margin = 5.0;

	double turn_p = 0.02;

	double error = recordedDeg - goalDeg;
	double adjustment;

	while (fabs(error) > deg_margin && !send_stop)
	{
		adjustment = turn_p*error;
		adjustment = fmax(fmin(adjustment, MAX_ADJUSTMENT), - MAX_ADJUSTMENT);
		send_move_data(forward_speed, 0.0, adjustment);
		error = getSensorData().gyro - goalDeg;
		_delay_ms(1);
	}
}

void enableTimers()
{
	TIMSK1 |= (1<<TOIE1);  // Enable timer overflow interrupt for Timer1
	TIMSK3 |= (1<<OCIE3A); // Enable interrupts on CTC mode
}

void disableTimers()
{
	TIMSK1 &= ~(1<<TOIE1);  // Disable timer overflow interrupt for Timer1
	TIMSK3 &= ~(1<<OCIE3A); // Disable timer compare interrupt for Timer3
}

#define SIDE_MARGIN 80.0 + 70.0 // Höftat
#define STABLE_COUNT 4
typedef enum {
	LEFT,
	RIGHT
} Side;
uint8_t far[2] = {0};

void reset_far() {
	far[LEFT] = 0;
	far[RIGHT] = 0;
}

void celebrate() {
	send_stop = true;
	pdFlag = false;
	autoMode = false;
	disableTimers();
}

void makeDecision(void) {
	volatile Sensor_data sensor_data = getSensorData();

	if (sensor_data.fl > SIDE_MARGIN) far[LEFT]++;
	else far[LEFT] = 0;

	if (sensor_data.fr > SIDE_MARGIN) far[RIGHT]++;
	else far[RIGHT] = 0;

	if (far[LEFT] > STABLE_COUNT/2.0 && far[RIGHT] > STABLE_COUNT/2.0) {
		celebrate();
		reset_far();
		return;
	}

	if (far[LEFT] > STABLE_COUNT) {
		turnTo(-90.0, 0.6);
		reset_far();
		send_move_data(0.6, 0, 0);
		waitForCorrectValues();
		cleanOldErrors();
	}

	if (far[RIGHT] > STABLE_COUNT) {
		turnTo(90.0, 0.6);
		reset_far();
		send_move_data(0.6, 0, 0);
		waitForCorrectValues();
		cleanOldErrors();
	}
}

ISR(TIMER1_OVF_vect) {
	makeDecisionFlag = 1;
	TCNT1H = 0xB0; //Reset Timer1 high register
	TCNT1L = 0x00; //Reset Timer1 low register
}

ISR(TIMER3_COMPA_vect) {
	pdFlag = true;
}

#define PD_PRESCALE 64
// Initialize one timer interrupt to happen approximately once per
// second and one every delta_t
void initTimer(void) {
	//TIMSK1 = (1<<TOIE1);//Enable timer overflow interrupt for Timer1
	TCNT1H = 0x80; //Initialize Timer1 high register
	TCNT1L = 0x00; //Initialize Timer1 low register
	TCCR1B = (1<<CS11)|(1<<CS10);//Use clock/64 prescaler

	// Use clock/64 prescaler and immediate compare mode
	TCCR3B = (1<<CS31) | (1<<CS30) | (1<<WGM32);
	// Compare match every delta_t seconds
	// OCRn = (clock_speed / Prescaler_value) * Desired_time_in_Seconds - 1
	OCR3A = (F_CPU/PD_PRESCALE) * delta_t - 1;
}

Bus_data prepare_data() {
	return data_to_send;
}

void interpret_data(Bus_data data){
	data_to_receive = data;
	
	if(data_to_receive.id == SET_P) {
		
		Constant_data constant_data = (Constant_data)data_to_receive;
		P = constant_data.constant;
		
	}else if(data_to_receive.id == SET_D) {
		
		Constant_data constant_data = (Constant_data)data_to_receive;
		D = constant_data.constant;
		
	}else if(data_to_receive.id == CHANGEMODE) {
		
		autoMode = data_to_receive.data[0];
		if(autoMode) {
			enableTimers();
			cleanOldErrors();

		}else {
			disableTimers();
			send_stop = true;
			pdFlag = false;
		}
		
	}
}

int main(void) {
	set_as_slave(F_CPU, prepare_data, interpret_data, DECISION);
	set_as_master(F_CPU);

	makeDecisionFlag = 0;
	pdFlag = false;
	autoMode = 0;
	turn = 1;

	initTimer();
	sei();
    while(1) {

        if(makeDecisionFlag) {
			makeDecision();
			makeDecisionFlag = 0;
		}
		
		_delay_ms(20);

		if(pdFlag) {
			volatile Sensor_data sensor_data = getSensorData();
			pdAlgoritm(sensor_data.br, sensor_data.bl);
			pdFlag = false;
		}

		if(send_stop) {
			send_move_data(0, 0, 0);
			send_stop = false;
		}

		_delay_ms(20);
    }
}
