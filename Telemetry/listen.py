from serial_robot import *

con = connect_to_robot()

import thread
import pyqtgraph as pg
import numpy as np

win = pg.GraphicsWindow(title="Telemetri")
win.resize(1000,1000)
pg.setConfigOptions(antialias=True)

# e_plot = win.addPlot(title='Error')
# e_plot.showGrid(x=True, y=True)
# win.nextRow()
# p_plot = win.addPlot(title='P')
# p_plot.showGrid(x=True, y=True)
# win.nextRow()
# d_plot = win.addPlot(title='D')
# d_plot.showGrid(x=True, y=True)

# win.nextRow()
g_plot = win.addPlot(title='Gyro')
win.nextRow()
ar_plot = win.addPlot(title='Angular rate')

x = np.array([])
# e_data = np.array([])
# p_data = np.array([])
# d_data = np.array([])
g_data = np.array([])
g_plot.showGrid(x=True, y=True)
ar_data = np.array([])
ar_plot.showGrid(x=True, y=True)

timer = pg.QtCore.QTimer()
def update():
    # e_plot.plot(x, e_data, clear=True, pen=(0, 5))
    # p_plot.plot(x, p_data, clear=True, pen=(1, 5))
    # d_plot.plot(x, d_data, clear=True, pen=(2, 5))
    g_plot.plot(x, g_data, clear=True, pen=(3, 5))
    ar_plot.plot(x, ar_data, clear=True, pen=(4, 5))

def reader():
    global x
    # global e_data, p_data, d_data
    global g_data, ar_data
    counter = 0
    while True:
        raw = con.read(size=1)
        # if raw == COMMANDS['PD_DATA']:
            # raw = con.read(size=4*3)
            # error, p, d = struct.unpack('fff', raw)
            # e_data = np.append(e_data, error)
            # p_data = np.append(p_data, p)
            # d_data = np.append(d_data, d)
            # print error, p, d
        if raw == COMMANDS['SENSOR_DATA']:
            raw = con.read(size=4*6+2)
            fr, br, fl, f, bl, g, ar = struct.unpack('ffffffh', raw)
            g_data = np.append(g_data, g)
            ar_data = np.append(ar_data, ar)
            x = np.append(x, counter)
            counter += 1
            print fr, br, fl, f, bl, g, ar
            print "".join(["{:02x}".format(ord(c)) for c in raw])
        else:
            print "".join(["{:02x}".format(ord(c)) for c in raw])

t = thread.start_new_thread(reader, ())

timer.timeout.connect(update)
timer.start(20)


while True:
    pg.QtGui.QApplication.processEvents()
