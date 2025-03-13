# -*- coding: utf-8 -*-
"""
Created on Mon Mar  3 12:39:03 2025

@author: vaucoi
"""

import matplotlib.pyplot as plt 
import pandas as pd
from sys import exit
import numpy as np



def extract_data(path, value):
    data_frame = pd.read_csv(path, sep='\t', header=11)
    
    start_value = data_frame.idxmin(axis=0, )['Temperature(C)'] # index of Beginning data collection
    offset = data_frame[['Time(s)']].iloc[start_value].values # Normalize such that beginning is 0
    peak_time = data_frame['Time(s)'].loc[data_frame['Temperature(C)'] >= value].iloc[1] # Get time where target_temperature is reached


    time = data_frame[['Time(s)']].iloc[start_value:].sub(offset)
    temperature = data_frame[['Temperature(C)']].iloc[start_value:]
    
    return time, temperature, peak_time - offset



def extract_raw(path):
    data_frame = pd.read_csv(path, sep='\t', header=11)
    
    time = data_frame[['Time(s)']]
    temperature = data_frame[['Temperature(C)']]
    
    return time, temperature

def extract_osc(path, value):
    data_frame = pd.read_csv(path, sep='\t', header=11)
    
    start_value = data_frame.idxmin(axis=0, )['Temperature(C)'] # index of Beginning data collection
    offset = data_frame[['Time(s)']].iloc[start_value].values # Normalize such that beginning is 0
    osc_start = data_frame['Time(s)'].index[data_frame['Temperature(C)'] >= value][0] # Get time where target_temperature is reached first time

    time = data_frame[['Time(s)']].iloc[osc_start:osc_start+4000].sub(offset)
    temperature = data_frame[['Temperature(C)']].iloc[osc_start:osc_start+4000]
    
    return time, temperature

def extract_rise(path, value):
    data_frame = pd.read_csv(path, sep='\t', header=11)
    
    start_value = data_frame.idxmin(axis=0, )['Temperature(C)'] # index of Beginning data collection
    offset = data_frame[['Time(s)']].iloc[start_value].values # Normalize such that beginning is 0
    osc_start = data_frame['Time(s)'].index[data_frame['Temperature(C)'] >= value][0] # Get time where target_temperature is reached first time

    time = data_frame[['Time(s)']].iloc[start_value: osc_start].sub(offset)
    temperature = data_frame[['Temperature(C)']].iloc[start_value: osc_start]
    
    return time, temperature

def draw_band(temp, time, set_temp, band_width, show_bands):
    average = round(np.average(temp), 3)
    plt.axhline(average, color='red', ls='--', label = f'Average Value: {average}')
    plt.axhline(set_temp, color='grey', ls='--', label=f'Set Temperature: {set_temp}.0')
    plt.legend()
    diff = -1*round(set_temp-average, 3)
    
    arrow_pos = time.iloc[1]["Time(s)"]
    end_pos = time.iloc[-1]["Time(s)"]
    plt.annotate("",
            xy=(arrow_pos - 10, set_temp+0.02), xycoords='data',
            xytext=(arrow_pos-10, average-0.01), textcoords='data',
            arrowprops=dict(arrowstyle="<->",
                            connectionstyle="arc3", color='r', lw=1),
            )
    
    font = {'family': 'serif',
            'color':  'black',
            'weight': 'normal',
            'size': 9,
            }
    text_pos = 0.75*arrow_pos
    plt.text(text_pos, np.mean([average, set_temp])-0.03, f'$\\approx${diff}$\Delta$', fontdict=font)
    plt.xlim(text_pos, 1.01*end_pos)
    
    if show_bands == 'proportional':    
        plt.axhspan(set_temp - band_width/2, set_temp + band_width/2, alpha=0.1, color = 'grey') 
    elif show_bands == 'hysteresis':
        plt.axhspan(set_temp - band_width, set_temp, alpha=0.1, color = 'grey') 
    else: None

# ------------------------- Plot --------------------------- #

#path = "../Data/PID Control/lab5_sample1_ProportionalBand2.0_50000ms_25to100C.csv"
#path = "../Data/OnOff/lab5_sample1_OnOff_50000_25to100C.csv"

path_list = [
#"../Data/Hysteresis/lab5_sample1_Hysteresis-0.5_35000ms_25to100C.csv",
#"../Data/Hysteresis/lab5_sample1_Hysteresis-0.25_35000ms_25to100C.csv",
#"../Data/Hysteresis/lab5_sample1_Hysteresis-1_35000ms_25to100C.csv",
"../Data/Hysteresis/lab5_sample1_Hysteresis-2_35000ms_25to100C.csv",

#"../Data/PID Control/lab5_sample1_ProportionalBand5.0_50000ms_25to100C.csv",
#"../Data/PID Control/lab5_sample1_ProportionalBand2.0_50000ms_25to100C.csv",
]

for path in path_list:
    time, temperature, peak_time = extract_data(path, 100)
    raw_time, raw_temperature = extract_raw(path)
    rise_time, rise_temperature = extract_rise(path, 100)
    osc_time, osc_temperature = extract_osc(path, 100)
    
    ### Raw
    plt.plot(raw_time, raw_temperature)
    plt.title("Raw Data")
    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (C)')
    plt.legend()
    plt.style.use('science')
    plt.show()
    
    
    ### Full
    plt.plot(time, temperature, )
    plt.axvline(peak_time, color='red', ls='-', label = 'Target Reached')
    plt.title("Full Data")
    draw_band(temperature, time, 100, 5, show_bands='proportional')
    
    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (C)')
    plt.legend()
    #draw_band(100, 8)
    plt.style.use('science')
    plt.show()
    
    
    ### Rising
    plt.plot(rise_time, rise_temperature, )
    plt.title("Rise data")
    
    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (C)')
    plt.legend()
    plt.style.use('science')
    plt.show()
    
    
    ### Oscillations
    plt.plot(osc_time, osc_temperature, )
    osc_average = round(np.average(osc_temperature), 3)
    plt.title("Temperature Fluctuations due to Hysteresis Control")
    
    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (C)')
    draw_band(osc_temperature, osc_time, 100, 5, show_bands='proportional')
    plt.style.use('science')
    plt.show()