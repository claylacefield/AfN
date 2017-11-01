# -*- coding: utf-8 -*-
"""
Created on Fri Oct 27 01:05:54 2017

@author: Clay

from:
https://www.pyimagesearch.com/2015/05/25/basic-motion-detection-and-tracking-with-python-and-opencv/



"""
## import the necessary packages
import argparse
import datetime
import imutils
import time
import cv2

import serial


## session variables

ardCOM = 'COM15'
shockX = 100
shockY = 200

## construct the argument parser and parse the arguments (if any)
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video", help="path to the video file")
ap.add_argument("-a", "--min-area", type=int, default=5000, help="minimum area size")
args = vars(ap.parse_args())

## set up serial connection with arduino (change to COM# of your arduino)
ardSer = serial.Serial(ardCOM, 9600, timeout=None)

## if the video argument is None, then we are reading from webcam
if args.get("video", None) is None:
    camera = cv2.VideoCapture(0)
    time.sleep(0.25)
    
# otherwise, we are reading from a video file
else:
    camera = cv2.VideoCapture(args["video"])

# initialize the first frame in the video stream
firstFrame = None

startTime = time.time()

## loop over the frames of the video
while True:
    # grab the current frame and initialize the occupied/unocc. text
    (grabbed, frame) = camera.read()
    text = "Safe"
    
    # if the frame could not be grabbed, then we have reached end of video
    frame = imutils.resize(frame, width=500)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (21,21), 0)
    
    # if the first frame is None, initialize it
    if firstFrame is None:
        firstFrame = gray
        continue
    
    # compute the abs difference bet. the current frame and first frame
    frameDelta = cv2.absdiff(firstFrame, gray)
    thresh = cv2.threshold(frameDelta, 25, 255, cv2.THRESH_BINARY)[1]
    
    # dilate the thresholded image to fill in holes, then find contours
    thresh = cv2.dilate(thresh, None, iterations=2)
    (_, cnts, _) = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # loop over the contours
    for c in cnts:
        # if the contour is too small, ignore it
        if cv2.contourArea(c) < args["min_area"]:
            continue
        
        # compute the bounding box for the contour, draw it on the frame, and update the text
        (x, y, w, h) = cv2.boundingRect(c)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        
        if x<shockX & y<shockY:
            ardSer.write('1')
            shockTime = time.time() - startTime
            print('Shock, time=')
            print(shockTime)
            text = "Shock"
        
    
    # draw the text and timestamp on the frame
    cv2.putText(frame, "Zone: {}".format(text), (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
    cv2.putText(frame, datetime.datetime.now().strftime("%A %d %B %Y %I:%M:%S%p"), (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)
    
    # show the frame and record if the user presses a key
    cv2.imshow("Behav video feed", frame)
    cv2.imshow("Thresh", thresh)
    cv2.imshow("Frame Delta", frameDelta)
    key = cv2.waitKey(1) & 0xFF
    
    # if the "q" key is pressed, break from the loop
    if key == ord("q"):
        break

# clean up the camera and close any open windows
camera.release()
cv2.destroyAllWindows()

ardSer.close()
print('Closing serial connection with arduino')