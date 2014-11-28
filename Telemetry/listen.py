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

x = np.array([])
# e_data = np.array([])
# p_data = np.array([])
# d_data = np.array([])
g_data = np.array([])

timer = pg.QtCore.QTimer()
def update():
    # e_plot.plot(x, e_data, clear=True, pen=(0, 3))
    # p_plot.plot(x, p_data, clear=True, pen=(1, 3))
    # d_plot.plot(x, d_data, clear=True, pen=(2, 3))
    g_plot.plot(x, g_data, clear=True, pen=(2, 3))

timer.timeout.connect(update)
timer.start(16)
def reader():
    global x
    # global e_data
    # global p_data
    # global d_data
    global g_data
    counter = 0
    while True:
        raw = con.read(size=1)
        if raw == b'\x05':
            raw = con.read(size=4*3)
            #error, p, d = struct.unpack('fff', raw)
            fr, br, fl, f, bl, g = struct.unpack('ffffff', raw)
            x = np.append(x, counter)
            counter += 1
            # e_data = np.append(e_data, error)
            # p_data = np.append(p_data, p)
            # d_data = np.append(d_data, d)
            g_data = np.append(d_data, d)
            print fr, br, fl, f, bl, g

t = thread.start_new_thread(reader, ())

while True:
    pg.QtGui.QApplication.processEvents()

