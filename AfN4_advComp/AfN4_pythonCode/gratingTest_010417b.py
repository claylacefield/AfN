# this is grating test script from
# http://www.psychopy.org/coder/tutorial1.html

from random import randint
import random #, shuffle


import time
from psychopy import visual, core, event #import some libraries from PsychoPy

#create a window
mywin = visual.Window([800,600],monitor="testMonitor", units="deg")

#create some stimuli
grating = visual.GratingStim(win=mywin, mask='circle', size=5, pos=[0,2], sf=0.8)
#fixation = visual.GratingStim(win=mywin, size=0.2, pos=[0,0], sf=0, rgb=-1)

n=0
ori = 0

# initialize position array
xpos = range(-10,10)
print(xpos)
xposArr = random.shuffle(xpos)
print(xposArr)
yposArr = random.shuffle(range(-8,8))

#draw the stimuli and update the window
while True: #this creates a never-ending loop
    
    if ((n%10)==0):
        ori = ori + 30;
        grating.ori= ori
        grating.pos = [randint(-10,10), randint(-8,8)] # [xposArr, yposArr]#
        
        mywin.flip(clearBuffer=True)
        
        time.sleep(2)
    
        n=n+1
    
    while ((n%100)!=0):
        grating.setPhase(0.05, '+')#advance phase by 0.05 of a cycle
        grating.draw()
        #fixation.draw()
        mywin.flip()
        
        n=n+1

    if len(event.getKeys())>0: break
    event.clearEvents()

#cleanup
mywin.close()
core.quit()