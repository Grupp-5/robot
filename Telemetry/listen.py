from serial_robot import *
con = connect_to_robot()

import thread, time
import pyqtgraph as pg
import numpy as np

win = pg.GraphicsWindow(title="Telemetri")
win.resize(1000,1000)
pg.setConfigOptions(antialias=True)

def create_plot(title, **kwargs):
    #       0: plotItem               1:ydata       0:xdata
    return [win.addPlot(title=title, **kwargs), np.array([]), np.array([])]

ALL_PLOTS = {}
ALL_PLOTS['e'] = create_plot('Error', colspan=5)
win.nextRow()
ALL_PLOTS['p'] = create_plot('P', colspan=5)
win.nextRow()
ALL_PLOTS['d'] = create_plot('D', colspan=5)
win.nextRow()
ALL_PLOTS['a'] = create_plot('Adjustment', colspan=5)
win.nextRow()

ALL_PLOTS['bl'] = create_plot('BL')
ALL_PLOTS['fl'] = create_plot('FL')
ALL_PLOTS['f'] = create_plot('F')
ALL_PLOTS['fr'] = create_plot('FR')
ALL_PLOTS['br'] = create_plot('BR')
win.nextRow()

ALL_PLOTS['g'] = create_plot('Gyro', colspan=4)
ALL_PLOTS['ar'] = create_plot('Angular Rate')

x = np.array([])

for k, v in ALL_PLOTS.iteritems():
    v[0].showGrid(x=True, y=True)

def update():
    count = 0
    global ALL_PLOTS
    for k, v in ALL_PLOTS.iteritems():
        v[0].plot(v[2], v[1], clear=True, pen=(count, len(ALL_PLOTS)))
        count += 1

def print_raw(raw):
    print "".join(["{:02x}".format(ord(c)) for c in raw])

def reader():
    global x
    global ALL_PLOTS
    counter = 0
    pd_counter = 0
    while True:
        raw = con.read(size=1)
        print_raw(raw)
        if raw == COMMANDS['PD_DATA']:
            raw = con.read(size=4*4)
            error, p, d, adj = struct.unpack('ffff', raw)
            pd_counter += 1
            ALL_PLOTS['e'][1] = np.append(ALL_PLOTS['e'][1], error)
            ALL_PLOTS['p'][1] = np.append(ALL_PLOTS['p'][1], p)
            ALL_PLOTS['d'][1] = np.append(ALL_PLOTS['d'][1], d)
            ALL_PLOTS['a'][1] = np.append(ALL_PLOTS['a'][1], adj)
            for x in ['e', 'p', 'd', 'a']:
                ALL_PLOTS[x][2] = np.append(ALL_PLOTS[x][2], pd_counter)

            print error, p, d, adj
            print_raw(raw)
        if raw == COMMANDS['SENSOR_DATA']:
            raw = con.read(size=4*6+2)
            fr, br, fl, f, bl, g, ar = struct.unpack('ffffffh', raw)
            counter += 1
            ALL_PLOTS['bl'][1] = np.append(ALL_PLOTS['bl'][1], bl)
            ALL_PLOTS['br'][1] = np.append(ALL_PLOTS['br'][1], br)
            ALL_PLOTS['fl'][1] = np.append(ALL_PLOTS['fl'][1], fl)
            ALL_PLOTS['fr'][1] = np.append(ALL_PLOTS['fr'][1], fr)
            ALL_PLOTS['f'][1] = np.append(ALL_PLOTS['f'][1], f)
            ALL_PLOTS['g'][1] = np.append(ALL_PLOTS['g'][1], g)
            ALL_PLOTS['ar'][1] = np.append(ALL_PLOTS['ar'][1], ar)
            for x in ['bl', 'br', 'fl', 'fr', 'f', 'g', 'ar']:
                ALL_PLOTS[x][2] = np.append(ALL_PLOTS[x][2], counter)
            print fr, br, fl, f, bl, g, ar
            print_raw(raw)

timer = pg.QtCore.QTimer()
timer.timeout.connect(update)
timer.start(100)

def create_mode_command(mode):
    ret = COMMANDS['CHANGEMODE']
    ret += struct.pack('B', mode)
    return ret

mode = 1

print "Sending changemode {}.".format(mode)
con.write(create_mode_command(mode))

t = thread.start_new_thread(reader, ())

timer.timeout.connect(update)
timer.start(20)


while True:
    pg.QtGui.QApplication.processEvents()
