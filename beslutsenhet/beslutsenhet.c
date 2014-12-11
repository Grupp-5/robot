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

#define ERROR_COUNT 16

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
	TIMSK3 |= (1<<OCIE3A); // Enable interrupts on CTC mode
	TIMSK1 |= (1<<OCIE1A); // Enable interrupts on CTC mode
}

void disableTimers()
{
	TIMSK1 &= ~(1<<OCIE1A); // Disable timer compare interrupt for Timer3
	TIMSK3 &= ~(1<<OCIE3A); // Disable timer compare interrupt for Timer3
}

#define WIDTH_MARGIN 70.0
#define SIDE_MARGIN 80.0 + WIDTH_MARGIN // Höftat
#define STABLE_COUNT 6
typedef enum {
	FAR_LEFT,
	FAR_RIGHT,
	BIT_LEFT,
	BIT_RIGHT,
	SHORT_FORWARD
} Side;
uint8_t sides[5] = {0};

void reset_far() {
	sides[FAR_LEFT] = 0;
	sides[FAR_RIGHT] = 0;
	sides[BIT_LEFT] = 0;
	sides[BIT_RIGHT] = 0;
	sides[SHORT_FORWARD] = 0;
}

void celebrate() {
	send_stop = true;
	pdFlag = false;
	autoMode = false;
	disableTimers();
}

double TURN_FORWARD_SPEED = 0.3;

void makeDecision(void) {
	volatile Sensor_data sensor_data = getSensorData();

	if(sensor_data.f < 30) sides[SHORT_FORWARD]++;
	else sides[SHORT_FORWARD] = 0;

	if (sensor_data.fl > SIDE_MARGIN) sides[FAR_LEFT]++;
	else sides[FAR_LEFT] = 0;

	if (sensor_data.fr > SIDE_MARGIN) sides[FAR_RIGHT]++;
	else sides[FAR_RIGHT] = 0;

	if (sensor_data.fr > WIDTH_MARGIN && sensor_data.fr < SIDE_MARGIN) sides[BIT_RIGHT]++;
	else sides[BIT_RIGHT] = 0;

	if (sensor_data.fl > WIDTH_MARGIN && sensor_data.fl < SIDE_MARGIN) sides[BIT_LEFT]++;
	else sides[BIT_LEFT] = 0;

	if (sides[FAR_LEFT] > STABLE_COUNT/2.0 && sides[FAR_RIGHT] > STABLE_COUNT/2.0) {
		celebrate();
		reset_far();
		return;
	}

	if (sides[BIT_LEFT] > STABLE_COUNT || sides[BIT_RIGHT] > STABLE_COUNT) {
		cleanOldErrors();
	}

	if (sides[FAR_LEFT] > STABLE_COUNT) {
		turnTo(-90.0, TURN_FORWARD_SPEED);
		reset_far();
		send_move_data(0.6, 0, 0);
		waitForCorrectValues();
		cleanOldErrors();
	}

	if (sides[FAR_RIGHT] > STABLE_COUNT) {
		turnTo(90.0, TURN_FORWARD_SPEED);
		reset_far();
		send_move_data(0.6, 0, 0);
		waitForCorrectValues();
		cleanOldErrors();
	}

	/*
	// För att gå fram och tillbaka i en korridor
	if (sides[SHORT_FORWARD] > STABLE_COUNT)
	{
		turnTo(175.0, 0.0);
		reset_far();
		cleanOldErrors();
	}
	*/

}

ISR(TIMER1_COMPA_vect) {
	makeDecisionFlag = true;
}

ISR(TIMER3_COMPA_vect) {
	pdFlag = true;
}

#define PD_PRESCALE 64
#define DEC_PRESCALE 64
double dec_delta_t = 0.1;
// Initialize one timer interrupt to happen approximately once per
// second and one every delta_t
void initTimer(void) {

	// Use clock/64 prescaler and immediate compare mode
	TCCR1B = (1<<CS11) | (1<<CS10) | (1<<WGM12);
	// Compare match every dec_delta_t seconds
	// OCRn = (clock_speed / Prescaler_value) * Desired_time_in_Seconds - 1
	OCR1A = (F_CPU/DEC_PRESCALE) * dec_delta_t - 1;

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
			reset_far();
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

			if (sides[BIT_LEFT] > STABLE_COUNT || sides[BIT_RIGHT] > STABLE_COUNT) {
				/*pdAlgoritm(sensor_data.br, sensor_data.fr);
			} else if () {
				pdAlgoritm(sensor_data.bl, sensor_data.fl);*/
			} else {
				pdAlgoritm(sensor_data.br, sensor_data.bl);
			}

			pdFlag = false;
		}

		if(send_stop) {
			send_move_data(0, 0, 0);
			send_stop = false;
		}

		_delay_ms(20);
    }
}
