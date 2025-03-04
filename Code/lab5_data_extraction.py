# -*- coding: utf-8 -*-
"""
Created on Mon Mar  3 12:39:03 2025

@author: vaucoi
"""

import matplotlib.pyplot as plt 
import pandas as pd
from sys import exit

def extract_data(path, value):
    data_frame = pd.read_csv(path, sep='\t', header=11)
    
    start_value = data_frame.idxmin(axis=0, )['Temperature(C)'] # index of Beginning data collection
    offset = data_frame[['Time(s)']].iloc[start_value].values # Normalize such that beginning is 0
    peak_time = data_frame['Time(s)'].loc[data_frame['Temperature(C)'] >= value].iloc[0] # Get time where target_temperature is reached
    
    # try:
    #     peak_time = data_frame['Time(s)'].loc[data_frame['Temperature(C)'] >= value].iloc[0] # Get time where target_temperature is reached
    # except Exception as e:
    #     print("Value given is probably higher than actual target value", "\nERROR:", e, '\n')
    #     exit(1)

    time = data_frame[['Time(s)']].iloc[start_value:].sub(offset)
    temperature = data_frame[['Temperature(C)']].iloc[start_value:]
    
    return time, temperature, peak_time - offset



def extract_raw(path):
    data_frame = pd.read_csv(path, sep='\t', header=11)
    
    time = data_frame[['Time(s)']]
    temperature = data_frame[['Temperature(C)']]
    
    return time, temperature

# ------------------------- Plot --------------------------- #

path = "../Data/PID Control/lab5_sample1_ProportionalBand2.0_50000ms_25to100C.csv"

time, temperature, peak_time = extract_data(path, 100)
raw_time, raw_temperature = extract_raw(path)

plt.plot(raw_time, raw_temperature)
plt.title("Raw Data")
plt.show()
plt.plot(time, temperature, )
plt.axvline(peak_time, color='red', ls='--', label = 'Target Reached')