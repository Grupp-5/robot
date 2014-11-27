import serial, time, struct
import serial.tools.list_ports as list_ports
import thread

DEVICE_HEX = 'D47F'

comport = None
print "Searching for device {}".format(DEVICE_HEX)
for port in list_ports.comports():
    # Knasig lista
    search = port[2].split('\\')[2]
    if DEVICE_HEX + '_' in search:
        print "Using " + port[0]
        comport = port[0]
if not comport:
    print "Device not found in com ports"
    sys.exit(1)

con = None

while not con:
    try:
        con = serial.Serial(
            comport,
            baudrate=115200,
            parity=serial.PARITY_NONE,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE)
    except Exception as e:
        print e
        print "Trying to connect..."
        time.sleep(0.1)

print "Connected to", con.name

import pyqtgraph as pg
import numpy as np

win = pg.GraphicsWindow(title="Error & P/D")
win.resize(1000,1000)
pg.setConfigOptions(antialias=True)

e_plot = win.addPlot(title='Error')
e_plot.showGrid(x=True, y=True)
win.nextRow()
p_plot = win.addPlot(title='P')
p_plot.showGrid(x=True, y=True)
win.nextRow()
d_plot = win.addPlot(title='D')
d_plot.showGrid(x=True, y=True)

x = np.array([])
e_data = np.array([])
p_data = np.array([])
d_data = np.array([])

timer = pg.QtCore.QTimer()
def update():
    e_plot.plot(x, e_data, clear=True, pen=(0, 3))
    p_plot.plot(x, p_data, clear=True, pen=(1, 3))
    d_plot.plot(x, d_data, clear=True, pen=(2, 3))
    #pw.plot(x, plot_data[i], clear=True, pen=(i, 3))

timer.timeout.connect(update)
timer.start(16)
def reader():
    global x
    global e_data
    global p_data
    global d_data
    counter = 0
    while True:
        raw = con.read(size=1)
        if raw == b'\x08':
            raw = con.read(size=4*3)
            error, p, d = struct.unpack('fff', raw)
            x = np.append(x, counter)
            counter += 1
            e_data = np.append(e_data, error)
            p_data = np.append(p_data, p)
            d_data = np.append(d_data, d)
            print error, p, d

t = thread.start_new_thread(reader, ())

while True:
    pg.QtGui.QApplication.processEvents()

