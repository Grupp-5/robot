import serial
import time
import struct

COMMANDS = {
    'CHANGEMODE'  : '\x00',
    'MOVE'        : '\x01',
    'START_TIMER' : '\x02',
    'STOP_TIMER'  : '\x03',
    'SET_P'       : '\x04',
    'SET_D'       : '\x05',
    'SENSOR_DATA' : '\x06'
}

con = serial.Serial(
    "COM29",
    baudrate=115200,
    parity=serial.PARITY_NONE,
    bytesize=serial.EIGHTBITS,
    stopbits=serial.STOPBITS_ONE)

print con.name

def create_move_command(forward, side, rotation):
    ret = COMMANDS['MOVE']
    for x in [forward, side, rotation]:
        ret += struct.pack('f', x)
    return ret

con.write(create_move_command(0.2, 0, 0))
time.sleep(5)
con.write(create_move_command(-0.2, 0, 0))
time.sleep(5)
con.write(create_move_command(0, 0.2, 0))
time.sleep(5)
con.write(create_move_command(0, -0.2, 0))
time.sleep(5)
con.write(create_move_command(0, 0, 0.2))
time.sleep(5)
con.write(create_move_command(0, 0, -0.2))
time.sleep(5)
con.write(create_move_command(0.2, 0.2, 0.2))
time.sleep(5)
con.write(create_move_command(-0.2, -0.2, -0.2))
time.sleep(5)
con.write(create_move_command(0, 0, 0))
