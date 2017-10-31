# Arduino for Neuroscientists
# AfN#4
# Clay Lacefield, 2017
#
# gratingArdSerial.py
# 
# this is based on a grating test script from
# http://www.psychopy.org/coder/tutorial1.html

from random import randint
import random #, shuffle


import time
from psychopy import visual, core, event #import some libraries from PsychoPy


import serial


#create a window
mywin = visual.Window([800,600],monitor="testMonitor", units="deg")

#create some stimuli
#grating = visual.GratingStim(win=mywin, mask='gauss', size=5, pos=[0,2], sf=0.8)
grating = visual.GratingStim(win=mywin, mask='none', size=50, pos=[0,0], sf=0.4)
#fixation = visual.GratingStim(win=mywin, size=0.2, pos=[0,0], sf=0, rgb=-1)

# session parameters
txtFilename = '171103_mouseA1.txt'
oriArr = [0 45 90 135 180 225 270 315]   # array of possible grating orientations



# init some vars
n=0
ori = 0

# some presets
phaseIncr = 0.1 # 0.05
contrIncr = 0.4
contrast = 0.5

# initialize position array (not using this right now)
#xpos = range(-10,10)
#print(xpos)
#xposArr = random.shuffle(xpos)
#print(xposArr)
#yposArr = random.shuffle(range(-8,8))
 
initTime = time.time()

# open arduino serial port
ard = serial.Serial('COM13', 9600, timeout=None)

# open file for writing text data
txtFile = open(txtFilename, 'w')
print('opening ' + txtFilename + ' for writing')


# look for arduino serial data, and if trial start, draw the stimuli and update the window
while True: #this creates a never-ending loop
    
    ardData = ard.readline()
    txtFile.write(ardData)
    print(ardData)
    
    
    if ((ardData.find("trialType="))!=-1):
        trialTypeString = ardData[10]
        trialType = int(trialTypeString)
        ori = ori + 30;
        grating.ori= ori
        contrast = randint(0,10)/10.0 #random.random()
        grating.contrast = contrast
        #xpos2 = randint(-10,10)    # (not using pos right now, i.e. full-field stim)
        #ypos2 = randint(-8,8)
        #grating.pos = [xpos2, ypos2] # [xposArr, yposArr]#
        
        #mywin.flip(clearBuffer=True)
        
        #time.sleep(2)
    
        n=n+1
        
        txtFile.write("time=" + str(time.time()-initTime) + ", ori=" + str(ori) + ", contrast=" + str(contrast))
        print("time=" + str(time.time()-initTime) + ", ori=" + str(ori) + ", contrast=" + str(contrast)) # + ", x=" + str(xpos2) + ", y=" + str(ypos2))

    # for set number of frames, loop to advance grating phase
    while ((n%100)!=0):
        grating.setPhase(phaseIncr, '+')#advance phase by 0.05 of a cycle
        grating.draw()
        #fixation.draw()
        mywin.flip()
        
        n=n+1
        
    
    # when grating stim over, tell arduino
    ard.write('1');  # note that only minimal information is sent to tell arduino to start looking for licks
    
    ardData = "none"
    mywin.flip(clearBuffer=True) # frame to gray

    if len(event.getKeys())>0: break    # break on keypress
    event.clearEvents()

#cleanup
mywin.close()
core.quit()