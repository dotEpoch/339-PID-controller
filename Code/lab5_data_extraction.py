# -*- coding: utf-8 -*-
"""
Created on Mon Mar  3 12:39:03 2025

@author: vaucoi
"""

import matplotlib.pyplot as plt 
import pandas as pd

data_frame = pd.read_csv("../Data/OnOff/lab5_sample2_OnOff_35000ms_25to100C.csv", sep='\t', header=11)
print(data_frame)

start_value = data_frame.idxmin(axis=0, )['Temperature(C)']
offset = data_frame[['Time(s)']].iloc[start_value]
end_value = len(data_frame)

time = data_frame[['Time(s)']].iloc[start_value:]
time = time.sub(offset)
print(time)
temperature = data_frame[['Temperature(C)']].iloc[start_value:]
print(temperature)

plt.plot(time, temperature, )