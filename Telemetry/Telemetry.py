#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time
import thread

from serial_robot import *
from robot_plotter import *
con = connect_to_robot()

import math as m
import sys, pygame, pygame.gfxdraw
from pygame.locals import *

pygame.init()
size = width, height = 1024, 764
black = 10, 10, 10
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
    'no_move' : False,
}

ALL_PLOTS = create_arrays()

def toggle_plots():
    global ALL_PLOTS
    if toggle_plots.off:
        toggle_plots.win = pg.GraphicsWindow(title="Telemetri")
        toggle_plots.win.resize(1000,1000)
        pg.setConfigOptions(antialias=True)

        create_plots(ALL_PLOTS, toggle_plots.win)

        toggle_plots.timer = pg.QtCore.QTimer()
        toggle_plots.timer.timeout.connect(make_updater(ALL_PLOTS))
        toggle_plots.timer.start(100)

        toggle_plots.off = False
    else:
        toggle_plots.timer.stop()
        # lol dunno
        toggle_plots.win.hide()
        toggle_plots.win.close()
        toggle_plots.win = None

        toggle_plots.off = True

toggle_plots.off = True
toggle_plots.timer = None

def plots_clear():
    global ALL_PLOTS
    clear_plots(ALL_PLOTS)

def send_go_stop():
    if not send_go_stop.started:
        print "Sending changemode {}.".format(1)
        _vars['no_move'] = True
        time.sleep(0.05*4)
        con.write(create_changemode_command(1))
        send_go_stop.started = True
    else:
        print "Sending changemode {}.".format(0)
        con.write(create_changemode_command(0))
        time.sleep(0.05*4)
        _vars['no_move'] = False
        send_go_stop.started = False
send_go_stop.started = False

def send_p():
    dialog = QtGui.QInputDialog()
    text, ok = dialog.getDouble(None, 'Set P', 'P', decimals=10)
    if ok:
        con.write(create_p_command(float(text)))

def send_d():
    dialog = QtGui.QInputDialog()
    text, ok = dialog.getDouble(None, 'Set D', 'D', decimals=10)
    if ok:
        con.write(create_d_command(float(text)))

def app_quit():
    sys.exit(0)

single_actions = [
    (K_p, toggle_plots), # Start / Stop plots
    (K_c, plots_clear),  # Clear        plots
    (K_g, send_go_stop), # Go / Stop auto mode
    (K_h, None), # Save plots
    (K_ESCAPE, app_quit),
    (K_t, send_p),
    (K_y, send_d),
]

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
        if _vars['no_move']:
            time.sleep(1)
            continue
        con.write(create_move_command(_vars['f_speed'], _vars['s_speed'], _vars['r_speed']))
        time.sleep(0.05)
        con.write(create_height_command(_vars['height']))
        time.sleep(0.05)
        con.write(create_rotation_command(_vars['xrot'], _vars['yrot']))
        time.sleep(0.05)
        print _vars['f_speed'], _vars['s_speed'], _vars['r_speed'], _vars['height']

t = thread.start_new_thread(command_sender, ())
reader_t = thread.start_new_thread(create_reader(ALL_PLOTS, con), ())

## TODO: Ifsatser beroende på om man har en joystick eller inte inkopplad
#print j.get_numaxes()
#j = pygame.joystick.Joystick(0)
#j.init()


MAX_MARGIN = 0.05

IS_PRESSED = {}
for key, action in single_actions:
    IS_PRESSED[key] = False

while 1:
    pygame.event.pump()
    delta_t = clock.tick(FRAMES_PER_SECOND)
    screen.fill(black)

    # for i in range(6):
    #     print i, j.get_axis(i)


    #_vars['f_speed'] = -j.get_axis(1)
    #_vars['s_speed'] = j.get_axis(0)
    #_vars['yrot'] = -j.get_axis(2)
    #_vars['xrot'] = -j.get_axis(3)

    #_vars['r_speed'] = (j.get_axis(4)+1)/2 -(j.get_axis(5)+1)/2

    for v in ['f_speed', 's_speed', 'r_speed', 'xrot', 'yrot']:
        if abs(_vars[v]) < MAX_MARGIN:
            _vars[v] = 0

    keys = pygame.key.get_pressed()

    for key, _actions in actions:
        if not keys[key[0]] and not keys[key[1]]:
            _actions['neither'](_actions['var'], _actions['step_size'])
        if keys[key[0]]:
            _actions['action_p'](_actions['var'], _actions['step_size'])
        if keys[key[1]]:
            _actions['action_n'](_actions['var'], _actions['step_size'])

    for key, _action in single_actions:
        if not IS_PRESSED[key] and keys[key]:
            IS_PRESSED[key] = True
            if _action:
                _action()
        elif not keys[key]:
            IS_PRESSED[key] = False

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
    pg.QtGui.QApplication.processEvents()
