# -*- coding: utf-8 -*-
"""
Created on Mon Oct 23 23:58:00 2017

@author: Clay


Arduino for Neuroscientists 2017
AfN#3

recSerialTxtPlot.py

Reads serial data from an attached arduino, records the output to a text file, 
parses the output, and plots.

NOTE:
After installing Anaconda python, must also install packages:
(from Anaconda prompt)
conda install pyserial
pip install drawnow

"""

# import libraries
import time
import serial
import numpy as np
import matplotlib.pyplot as plt
from drawnow import drawnow #*

# session parameter variables
sessName = '171027_mouseA1'
sessionDurMin = 1  
binSec = 5  # number of seconds in each timebin
ardCOM = 'COM13'

plotYmax = 6

# derivative variables
txtFilename = sessName + '.txt'
figFilename = sessName + '.pdf'
sessionDur = sessionDurMin*60
numBins = sessionDur/binSec

# initialize counting vars
binNum = 0
binType1 = 0 # bin event counters
binType2 = 0
binType3 = 0
lastBinTime = 0

# list of all bin counts (init w. zeros)
allBinNum1 = np.zeros(numBins, dtype=np.int8)
allBinNum2 = np.zeros(numBins, dtype=np.int8)
allBinNum3 = np.zeros(numBins, dtype=np.int8)

# set up serial connection with arduino (change to COM# of your arduino)
ardSer = serial.Serial(ardCOM, 9600, timeout=None)

# open file for writing text data
txtFile = open(txtFilename, 'w')
print('opening ' + txtFilename + ' for writing')

startTime = time.time()  # program start time, in sec

plt.ion() # specify interactive plot

# define function of figure parameters for plotting with drawnow
def makeFig():
    plt.subplot(3,1,1)
    plt.ylim(0,plotYmax)
    plt.title(sessName + ' behavioral performance')
    plt.ylabel('numType1')
    n = len(allBinNum1)
    x = range(n)
    plt.bar(x, allBinNum1)
    
    plt.subplot(3,1,2)
    plt.ylim(0,plotYmax)
    plt.ylabel('numType2')
    plt.bar(x, allBinNum2)
    
    plt.subplot(3,1,3)
    plt.ylim(0,plotYmax)
    plt.ylabel('numType3')
    plt.xlabel('time')
    plt.bar(x, allBinNum3)
    
# print out some header info
print('Starting session')
txtFile.write('Starting session \n')
time.sleep(1)

# look for serial data
while True: #this creates a never-ending loop
    
    ardData = ardSer.readline() # read serial data from arduino
    
    print(ardData)
    txtFile.write(ardData) # write it to the text file
    
    # parse the string of serial data, and count event types
    if ((ardData.find('trial'))!=-1):
        trialStrInd = ardData.find('trial')
        millisStrInd = ardData.find('millis')
        
        trialStr = ardData[trialStrInd + 10:millisStrInd - 2]
        
        try:
            trialType = int(trialStr)
            
            if (trialType==1):
                binType1 += 1
            elif (trialType==2):
                binType2 += 1
            elif (trialType==3):
                binType3 += 1
        except:
            pass            
            print('Error parsing serial data')

    # plot extracted data for next timebin
    if (time.time()-lastBinTime>binSec):

        allBinNum1[binNum] = binType1
        allBinNum2[binNum] = binType2
        allBinNum3[binNum] = binType3

        drawnow(makeFig) # draw the figure with drawnow

        binType1 = 0 # reset bin event counters
        binType2 = 0
        binType3 = 0
        lastBinTime = time.time()
        binNum +=1
        
    # end session if session duration elapsed
    if (time.time()-startTime>sessionDur):
        print('END session')
        txtFile.write('END session')
        plt.savefig(figFilename)
        break
        
        
## must close txtFile and serial connection
txtFile.close()
ardSer.close()
print('Closing serial connection with arduino')
        
        
        







