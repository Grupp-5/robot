#!/usr/bin/env python
# -*- coding: utf-8 -*-

from serial_robot import *
from robot_plotter import *

from PyQt4 import QtGui, QtCore
from PyQt4.QtGui import *
from PyQt4.QtCore import pyqtSignal, pyqtSlot
import numpy as np
import pyqtgraph as pg
import sys, collections, random

class ConnectThread(QtCore.QThread):
    send_text = pyqtSignal(str)
    start_reader = pyqtSignal()

    def __init__(self, form):
        QtCore.QThread.__init__(self)
        self.form = form
        self.send_text.connect(form.submitCommand)
        self.start_reader.connect(form.startReader)

    def run(self):
        self.form.con = connect_to_robot(self.send_text.emit)
        self.start_reader.emit()


class ReaderThread(QtCore.QThread):
    send_text = pyqtSignal(str)

    def __init__(self, form):
        QtCore.QThread.__init__(self)
        self.form = form
        self.send_text.connect(form.receiveCommand)

    def run(self):
        reader(self.form.plot_arrays, self.form.con, self.form.update_stats)


class Form(QtGui.QWidget):
    def __init__(self):
        super(Form, self).__init__()
        self.plot_arrays = create_arrays()
        self.initUI()
        self.cthread = ConnectThread(self)
        self.cthread.start()

    def initUI(self):
        # TABB-WIDGET
        self.tab_widget = QtGui.QTabWidget()
        stats_tab = QtGui.QWidget()
        self.tab_widget.addTab(stats_tab, "Statistics")
        ir_tab = QtGui.QWidget()
        self.tab_widget.addTab(ir_tab, "IR-sensor")
        gyro_tab = QtGui.QWidget()
        self.tab_widget.addTab(gyro_tab, "Gyro")
        pd_tab = QtGui.QWidget()
        self.tab_widget.addTab(pd_tab, "Reglering")

        # Labels för sensordata
        self.labels = collections.OrderedDict()
        # IR
        self.labels['f'] = [QLabel("Fram"), QLabel("...")]
        self.labels['fr'] = [QLabel(u"Fram höger"), QLabel("...")]
        self.labels['fl'] = [QLabel(u"Fram vänster"), QLabel("...")]
        self.labels['bl'] = [QLabel(u"Bak vänster"), QLabel("...")]
        self.labels['br'] = [QLabel(u"Bak höger"), QLabel("...")]
        # Gyro
        self.labels['g'] = [QLabel("Vinkel"), QLabel("...")]
        self.labels['ar'] = [QLabel("Vinkelhastighet"), QLabel("...")]
        # PD
        self.labels['e'] = [QLabel("Fel"), QLabel("...")]
        self.labels['p'] = [QLabel("Proportionellt del"), QLabel("...")]
        self.labels['d'] = [QLabel("Deriverande del"), QLabel("...")]
        self.labels['a'] = [QLabel("Justering"), QLabel("...")]

        #self.switch1 = QCheckBox("hej")
        # Knappar
        self.clearButton = QPushButton(u"&Rensa")
        self.goButton = QPushButton(u"&Go")

        self.pEdit = QDoubleSpinBox()
        self.pEdit.setDecimals(4)
        self.pEdit.setValue(0.005)
        self.pEdit.setPrefix("P ")
        self.pEdit.setSingleStep(0.0005)
        self.dEdit = QDoubleSpinBox()
        self.dEdit.setDecimals(4)
        self.dEdit.setValue(0.02)
        self.dEdit.setPrefix("D ")
        self.dEdit.setSingleStep(0.0005)
        self.speedEdit = QDoubleSpinBox()
        self.speedEdit.setDecimals(2)
        self.speedEdit.setValue(1.7)
        self.speedEdit.setPrefix("SPD ")
        self.speedEdit.setSingleStep(0.1)
        self.speedEdit.setMinimum(0.1)
        self.speedEdit.setMaximum(4.0)
        # Consolen
        self.console = QTextEdit()
        self.console.setReadOnly(True)

        # Graf
        self.irplot = pg.GraphicsLayoutWidget()
        self.gyroplot = pg.GraphicsLayoutWidget()
        self.pdplot = pg.GraphicsLayoutWidget()
        pg.setConfigOptions(antialias=True)

        # Tabblayouten
        boxLayoutTop = QHBoxLayout()
        buttonsLayout = QVBoxLayout()

        boxLayoutTop.addWidget(self.tab_widget)
        boxLayoutTop.addLayout(buttonsLayout)

        buttonsLayout.addStretch(1)
        buttonsLayout.addWidget(self.clearButton)
        buttonsLayout.addWidget(self.goButton)
        buttonsLayout.addWidget(self.pEdit)
        buttonsLayout.addWidget(self.dEdit)
        buttonsLayout.addWidget(self.speedEdit)
        buttonsLayout.addStretch(1)

        # Statistiklayouter
        boxLayoutLeft1 = QVBoxLayout()
        boxLayoutLeft2 = QVBoxLayout()
        for k, v in self.labels.iteritems():
            boxLayoutLeft1.addWidget(v[0])
            boxLayoutLeft2.addWidget(v[1])
        boxLayoutLeft1.addStretch(1)
        boxLayoutLeft2.addStretch(1)


        #boxLayoutLeft1.addWidget(self.switch1)
        boxLayoutRight = QVBoxLayout()
        boxLayoutRight.addWidget(self.console)

        # Main layout
        mainLayout = QGridLayout()
        mainLayout.addLayout(boxLayoutTop, 0, 0)
        # Stat layout
        statsLayout = QGridLayout(stats_tab)
        statsLayout.addLayout(boxLayoutLeft1, 0, 0)
        statsLayout.addLayout(boxLayoutLeft2, 0, 1)
        statsLayout.addLayout(boxLayoutRight, 0, 3, 2, 1)
        # IR layout
        irLayout = QGridLayout(ir_tab)
        irLayout.addWidget(self.irplot)
        # Gyro layout
        gyroLayout = QGridLayout(gyro_tab)
        gyroLayout.addWidget(self.gyroplot)
        # PD layout
        pdLayout = QGridLayout(pd_tab)
        pdLayout.addWidget(self.pdplot)

        # Plots-grejer
        create_gyro_plots(self.plot_arrays, self.gyroplot)
        create_sensor_plots(self.plot_arrays, self.irplot)
        create_pd_plots(self.plot_arrays, self.pdplot)
        add_plot_grids(self.plot_arrays)

        self.connect(self.clearButton, QtCore.SIGNAL("clicked()"), make_clear_plots(self.plot_arrays))
        self.connect(self.goButton, QtCore.SIGNAL("clicked()"), self.send_go_stop)
        self.connect(self.pEdit, QtCore.SIGNAL("valueChanged(double)"),
                     self.make_sender(create_p_command, 'P'))
        self.connect(self.dEdit, QtCore.SIGNAL("valueChanged(double)"),
                     self.make_sender(create_d_command, 'D'))
        self.connect(self.speedEdit, QtCore.SIGNAL("valueChanged(double)"),
                     self.make_sender(create_speed_command, 'Speed'))

        self.setLayout(mainLayout)
        self.setWindowTitle("Robotdestroyer v1.23 beta [build.{}]".format(random.randint(10000, 99999)))

    @pyqtSlot(str)
    def submitCommand(self, command):
        if command != "":
            self.console.append("Dator -> " + command)

    def receiveCommand(self, command):
        if command != "":
            self.console.append("Robot -> " + command)

    def keyPressEvent(self, qKeyEvent):
        pass
        #if qKeyEvent.key() == QtCore.Qt.Key_Return:
        #    self.submitCommand()

    @pyqtSlot()
    def startReader(self):
        self.readerThread = ReaderThread(self)
        self.readerThread.start()
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(make_updater(self.plot_arrays))
        self.timer.start(100)

    def update_stats(self, label, value):
        self.labels[label][1].setText(value)

    auto_started = False
    def send_go_stop(self):
        if not self.auto_started:
            self.con.write(create_changemode_command(1))
            self.submitCommand(u"Autonomt läge")
            self.auto_started = True
        else:
            self.con.write(create_changemode_command(0))
            self.submitCommand(u"Manuellt läge")
            self.auto_started = False

    def make_sender(self, cmd_creater, name):
        def send_new_value(val):
            self.submitCommand(u"Nytt {}-värde: {}".format(name, val))
            self.con.write(cmd_creater(val))
        return send_new_value


def make_clear_plots(plots):
    def _clearer():
        clear_plots(plots)
    return _clearer


def main():
    app = QtGui.QApplication(sys.argv)
    screen = Form()
    screen.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
