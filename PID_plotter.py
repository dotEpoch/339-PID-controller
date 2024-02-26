'''
Example script for use in the PHYS 339 PID lab at McGill University.
Written by Brandon Ruffolo in Feb 2024.

'''

import time    as _time
import spinmob as _s
import serial  as _serial
import spinmob.egg as _egg
_g = _egg.gui

terminator  = '\r\n' # Serial COM marker

class plotter():
    
    def __init__(self, name, dt, port, baudrate, timeout):
        self.window    = _g.Window(name, size = [1000,800])
        
        # Define grid space
        self.grid_top     = self.window.place_object(_g.GridLayout(False))
        self.window.new_autorow()
        self.grid_bottom  = self.window.place_object(_g.GridLayout(False), alignment=0)
        
        # Temperature numberbox
        self.grid_top.place_object(_g.Label('Temperature:').set_style('font-size: 20pt; color: mediumspringgreen'),alignment=2)
        self.number_temperature = self.grid_top.place_object(_g.NumberBox(0.000, suffix = '°C').enable().set_width(250).set_style('font-size: 20pt; color: mediumspringgreen'),alignment=2)
        
        # Plot databox
        self.plot_raw = self.grid_bottom.place_object(_g.DataboxPlot('*.csv', autoscript=1), column_span=10)
        
        # Timer
        self.timer    = _g.Timer(interval_ms=dt, single_shot=False)
        self.timer.signal_tick.connect(self._timer_tick)
        
        #
        self.arduino = _serial.Serial(port = port, baudrate = baudrate, timeout = timeout)
        _time.sleep(2)             # Give the arduino time to reset and run setup()
        self.arduino.flushInput()  # Flush any data in the input buffer
        
        # Show the GUI
        self.window.show()
        
        # Start the timer
        self.timer.start()

    
    def _timer_tick(self):
        packet = self.arduino.read_until(terminator.encode())
        data = packet.decode().strip(terminator).split(',')
        try:
            t, T, H = float(data[0]), float(data[1]), float(data[2])
        except:
            return
        
        # Write the data into the GUI
        self.plot_raw.append_row( [t/1000, T, 100*H/62499], ckeys=['Time (s)', 'Temperature (C)', 'Power (%)'])
        self.plot_raw.plot()
        self.number_temperature.set_value(T)
        
        # Update the gui
        self.window.process_events()

self = plotter(name = 'PID', dt = 80, port = 'COM4', baudrate = 115200, timeout = 1)  