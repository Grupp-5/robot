typedef unsigned char byte;

#define ON 1
#define OFF 0
#define _ON 0 
#define _OFF 1

// EEPROM
#define AX_MODEL_NUMBER_L 0
#define AX_MODEL_NUMBER_H 1
#define AX_VERSION 2
#define AX_ID 3
#define AX_BAUD_RATE 4
#define AX_RETURN_DELAY_TIME 5
#define AX_CW_ANGLE_LIMIT_L 6
#define AX_CW_ANGLE_LIMIT_H 7
#define AX_CCW_ANGLE_LIMIT_L 8
#define AX_CCW_ANGLE_LIMIT_H 9
#define AX_SYSTEM_DATA2 10
#define AX_LIMIT_TEMPERATURE 11
#define AX_DOWN_LIMIT_VOLTAGE 12
#define AX_UP_LIMIT_VOLTAGE 13
#define AX_MAX_TORQUE_L 14
#define AX_MAX_TORQUE_H 15
#define AX_RETURN_LEVEL 16
#define AX_ALARM_LED 17
#define AX_ALARM_SHUTDOWN 18
#define AX_OPERATING_MODE 19
#define AX_DOWN_CALIBRATION_L 20
#define AX_DOWN_CALIBRATION_H 21
#define AX_UP_CALIBRATION_L 22
#define AX_UP_CALIBRATION_H 23

// RAM
#define AX_TORQUE_ENABLE 24
#define AX_LED 25
#define AX_CW_COMPLIANCE_MARGIN 26
#define AX_CCW_COMPLIANCE_MARGIN 27
#define AX_CW_COMPLIANCE_SLOPE 28
#define AX_CCW_COMPLIANCE_SLOPE 29
#define AX_GOAL_POSITION_L 30
#define AX_GOAL_POSITION_H 31
#define AX_GOAL_SPEED_L 32
#define AX_GOAL_SPEED_H 33
#define AX_TORQUE_LIMIT_L 34
#define AX_TORQUE_LIMIT_H 35
#define AX_PRESENT_POSITION_L 36
#define AX_PRESENT_POSITION_H 37
#define AX_PRESENT_SPEED_L 38
#define AX_PRESENT_SPEED_H 39
#define AX_PRESENT_LOAD_L 40
#define AX_PRESENT_LOAD_H 41
#define AX_PRESENT_VOLTAGE 42
#define AX_PRESENT_TEMPERATURE 43
#define AX_REGISTERED_INSTRUCTION 44
#define AX_PAUSE_TIME 45
#define AX_MOVING 46
#define AX_LOCK 47
#define AX_PUNCH_L 48
#define AX_PUNCH_H 49

//--- Instruction ---
#define INST_PING 0x01
#define INST_READ 0x02
#define INST_WRITE 0x03
#define INST_REG_WRITE 0x04
#define INST_ACTION 0x05
#define INST_RESET 0x06
#define INST_DIGITAL_RESET 0x07 
#define INST_SYSTEM_READ 0x0C
#define INST_SYSTEM_WRITE 0x0D
#define INST_SYNC_WRITE 0x83
#define INST_SYNC_REG_WRITE 0x84

//Hardware Dependent Item 
#define DEFAULT_BAUD_RATE 34 //57600bps at 16MHz

void PingAX(byte id);
void uart_init (void);

typedef struct {
	char id;
	char error;
	char length;
	// TODO: 5??
	char params[5];
	char checksum;
} ResponsePacket;
