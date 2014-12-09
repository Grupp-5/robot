#!/bin/env python
# -*- coding: utf-8 -*-
from serial_robot import *

from PyQt4 import QtGui
import thread, time
import pyqtgraph as pg
import numpy as np


def create_arrays():
    arys = {}
    ## En tom array för varje typ av data
    for d in ['e','p','d', 'i', 'a', 'bl', 'fl' ,'f', 'fr', 'br', 'g', 'ar']:
        arys[d] = [None, np.array([]), np.array([])]
    return arys

def create_plot(win, title, **kwargs):
    return win.addPlot(title=title, **kwargs)

def create_pd_plots(plots, win):
    plots['e'][0] = create_plot(win, 'Error', colspan=5)
    win.nextRow()
    plots['p'][0] = create_plot(win, 'P', colspan=5)
    win.nextRow()
    plots['d'][0] = create_plot(win, 'D', colspan=5)
    win.nextRow()
    plots['i'][0] = create_plot(win, 'I', colspan=5)
    win.nextRow()
    plots['a'][0] = create_plot(win, 'Adjustment', colspan=5)
    win.nextRow()

def create_sensor_plots(plots, win):
    plots['fl'][0] = create_plot(win, 'FL')
    plots['f' ][0] = create_plot(win, 'F' )
    plots['fr'][0] = create_plot(win, 'FR')
    win.nextRow()
    plots['bl'][0] = create_plot(win, 'BL')
    win.nextColumn()
    plots['br'][0] = create_plot(win, 'BR')
    win.nextRow()

def create_gyro_plots(plots, win):
    plots['g' ][0] = create_plot(win, 'Gyro', colspan=4)
    plots['ar'][0] = create_plot(win, 'Angular Rate')

def add_plot_grids(plots):
    for k, v in plots.iteritems():
        v[0].showGrid(x=True, y=True)

def create_plots(plots, win):
    create_pd_plots(plots, win)
    create_sensor_plots(plots, win)
    create_gyro_plots(plots, win)
    add_plot_grids(plots)

def plot_update(plots):
    count = 0
    for k, v in plots.iteritems():
        v[0].plot(v[2], v[1], clear=True, pen=(count, len(plots)))
        count += 1

def make_updater(plots):
    def updater():
        plot_update(plots)
    return updater

def clear_plots(plots):
    for key, value in plots.iteritems():
        value[1] = np.array([])
        value[2] = np.array([])

def create_reader(plots, con):
    def _reader():
        reader(plots, con)
    return _reader

def reader(ALL_PLOTS, con, extra_fun=None):
    counter = 0
    pd_counter = 0
    while True:
        raw = con.read(size=1)
        print_raw(raw)
        if raw == COMMANDS['PD_DATA']:
            raw = con.read(size=4*5)
            error, p, d, i, adj = struct.unpack('fffff', raw)
            pd_counter += 1
            for plt, val in zip(['e',  'p', 'd',  'i', 'a'],
                                [error, p ,  d ,   i, adj]):
                ALL_PLOTS[plt][1] = np.append(ALL_PLOTS[plt][1], val)
                if extra_fun:
                    extra_fun(plt, str(int(val)))

            for x in ['e', 'p', 'i', 'd', 'a']:
                ALL_PLOTS[x][2] = np.append(ALL_PLOTS[x][2], pd_counter)

            print error, p, d
            # print_raw(raw)
        if raw == COMMANDS['SENSOR_DATA']:
            raw = con.read(size=4*6+2)
            fr, br, fl, f, bl, g, ar = struct.unpack('ffffffh', raw)

            counter += 1
            for plt, val in zip(['bl', 'br', 'fl', 'fr', 'f', 'g', 'ar'],
                                [ bl ,  br ,  fl ,  fr ,  f ,  g ,  ar ]):
                ALL_PLOTS[plt][1] = np.append(ALL_PLOTS[plt][1], val)
                if extra_fun:
                    extra_fun(plt, str(int(val)))

            for x in ['bl', 'br', 'fl', 'fr', 'f', 'g', 'ar']:
                ALL_PLOTS[x][2] = np.append(ALL_PLOTS[x][2], counter)
            print fr, br, fl, f, bl, g, ar
            # print_raw(raw)

if __name__ == "__main__":
    win = pg.RemoteGraphicsWindow(title="Telemetri")
    pg.setConfigOptions(antialias=True)

    ALL_PLOTS = create_arrays()
    create_plots(ALL_PLOTS, win)

    con = connect_to_robot()

    t = thread.start_new_thread(create_reader(ALL_PLOTS, con), ())

    timer = pg.QtCore.QTimer()
    timer.timeout.connect(make_updater(ALL_PLOTS))
    timer.start(20)

    while True:
        pg.QtGui.QApplication.processEvents()
