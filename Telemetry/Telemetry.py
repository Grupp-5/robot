import sys
import serial
import time
import struct
import thread

COMMANDS = {
    'CHANGEMODE'  : '\x00',
    'MOVE'        : '\x01',
    'STOP_TIMER'  : '\x02',
    'SET_P'       : '\x03',
    'SET_D'       : '\x04',
    'SENSOR_DATA' : '\x05',
    'SET_HEIGHT'  : '\x06',
    'ROTATION'    : '\x07',
}

con = None
print sys.argv[1]


while not con:
    try:
        con = serial.Serial(
            sys.argv[1],
            baudrate=115200,
            parity=serial.PARITY_NONE,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE)
    except Exception as e:
        print e
        print "Trying to connect..."
        time.sleep(5)

print "Connected to", con.name

def create_height_command(height):
    ret = COMMANDS['SET_HEIGHT']
    ret += struct.pack('f', height)
    return ret

def create_move_command(forward, side, rotation):
    ret = COMMANDS['MOVE']
    for x in [forward, side, rotation]:
        ret += struct.pack('f', x)
    return ret

def create_rotation_command(xrot, yrot):
    ret = COMMANDS['ROTATION']
    for x in [xrot, yrot]:
        ret += struct.pack('f', x)
    return ret

import math as m
import sys, pygame, pygame.gfxdraw
from pygame.locals import *

pygame.init()
size = width, height = 1024, 764
black = 0, 0, 0
red = 200, 0, 0
screen = pygame.display.set_mode(size)

clock = pygame.time.Clock()
FRAMES_PER_SECOND = 30

STEP_SIZE = 0.05

def inc(x, step_size):
    _vars[x] += step_size
    if _vars[x] > 1:
        _vars[x] = 1

def dec(x, step_size):
    _vars[x] -= step_size
    if _vars[x] < -1:
        _vars[x] = -1

def restore(x, step_size):
    if _vars[x] > 0:
        _vars[x] -= step_size
    if _vars[x] < 0:
        _vars[x] += step_size
    if abs(_vars[x]-0.1) < 0.1:
        _vars[x] = 0

def no_action(x, step_size):
    pass

_vars = {
    'f_speed' : 0,
    's_speed' : 0,
    'r_speed' : 0,
    'height'  : 0,
    'xrot'    : 0,
    'yrot'    : 0,
}

actions = [
    ((K_w, K_s),
     {'action_p' : inc, 'action_n' : dec, 'neither': restore, 'var': 'f_speed', 'step_size': STEP_SIZE}),
    ((K_d, K_a),
     {'action_p' : inc, 'action_n' : dec, 'neither': restore, 'var': 's_speed', 'step_size': STEP_SIZE}),
    ((K_e, K_q),
     {'action_p' : inc, 'action_n' : dec, 'neither': restore, 'var': 'r_speed', 'step_size': STEP_SIZE}),
    ((K_o, K_l),
     {'action_p' : inc, 'action_n' : dec, 'neither': no_action, 'var': 'height', 'step_size': 0.05}),
    ((K_UP, K_DOWN),
     {'action_p' : inc, 'action_n' : dec, 'neither': no_action, 'var': 'xrot', 'step_size': 0.1}),
    ((K_LEFT, K_RIGHT),
     {'action_p' : inc, 'action_n' : dec, 'neither': no_action, 'var': 'yrot', 'step_size': 0.1}),
]

def command_sender():
    while 1:
        con.write(create_move_command(_vars['f_speed'], _vars['s_speed'], _vars['r_speed']))
        time.sleep(0.05)
        con.write(create_height_command(_vars['height']))
        time.sleep(0.05)
        con.write(create_rotation_command(_vars['xrot'], _vars['yrot']))
        time.sleep(0.05)
        print _vars['f_speed'], _vars['s_speed'], _vars['r_speed'], _vars['height']

t = thread.start_new_thread(command_sender, ())

j = pygame.joystick.Joystick(0)
j.init()

print j.get_numaxes()

MAX_MARGIN = 0.05

while 1:
    pygame.event.pump()
    delta_t = clock.tick(FRAMES_PER_SECOND)
    screen.fill(black)

    # for i in range(6):
    #     print i, j.get_axis(i)


    _vars['f_speed'] = -j.get_axis(1)
    _vars['s_speed'] = j.get_axis(0)
    _vars['yrot'] = -j.get_axis(2)
    _vars['xrot'] = -j.get_axis(3)

    _vars['r_speed'] = (j.get_axis(4)+1)/2 -(j.get_axis(5)+1)/2

    for v in ['f_speed', 's_speed', 'r_speed', 'xrot', 'yrot']:
        if abs(_vars[v]) < MAX_MARGIN:
            _vars[v] = 0

    #Keys = pygame.key.get_pressed()

    # for key, _actions in actions:
    #     if not keys[key[0]] and not keys[key[1]]:
    #         _actions['neither'](_actions['var'], _actions['step_size'])
    #     if keys[key[0]]:
    #         _actions['action_p'](_actions['var'], _actions['step_size'])
    #     if keys[key[1]]:
    #         _actions['action_n'](_actions['var'], _actions['step_size'])

    draw_at = 10
    for key, value in _vars.iteritems():
        draw_height = -value*height/2
        pygame.draw.rect(screen, red, (draw_at, height/2, 10, draw_height))
        draw_at += 20


    theta = m.atan2(_vars['f_speed'], _vars['s_speed'])
    pygame.draw.aaline(screen, red, (width/2, height/2),
                       (width/2+m.cos(theta)*200*abs(_vars['s_speed']),
                        height/2-m.sin(theta)*200*abs(_vars['f_speed'])))
    if _vars['r_speed'] > 0:
        pygame.gfxdraw.arc(screen, width/2, height/2, 200, -90, int(-90+_vars['r_speed']*180), red)
    else:
        pygame.gfxdraw.arc(screen, width/2, height/2, 200, int(-90+_vars['r_speed']*180), -90,  red)

    for event in pygame.event.get():
        if event.type == pygame.QUIT: sys.exit()

    pygame.display.flip()
