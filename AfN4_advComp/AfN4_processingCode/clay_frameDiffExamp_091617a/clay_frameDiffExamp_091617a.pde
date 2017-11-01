/**
 * Frame Differencing 
 * by Golan Levin. 
 *
 * Quantify the amount of movement in the video frame using frame-differencing.
 */ 


import processing.video.*;

int numPixels;
int[] previousFrame;
Capture video;

//clay
int pixThresh = 100;
int sumDiff = 0;
int pixX = 0;
int pixY = 0;
int contX = 0;
int contY = 0;
int numContPix = 0;
int comX = 0;
int comY = 0;
int lastX = 0;
int lastY = 0;
int modX = 0;
int modY = 0;



void setup() {
  size(640, 480);
  
  // This the default video input, see the GettingStartedCapture 
  // example if it creates an error
  video = new Capture(this, width, height);
  
  // Start capturing the images from the camera
  video.start(); 
  
  numPixels = video.width * video.height;
  // Create an array to store the previously captured frame
  previousFrame = new int[numPixels];
  loadPixels();
}

void draw() {
  if (video.available()) {
    // When using video to manipulate the screen, use video.available() and
    // video.read() inside the draw() method so that it's safe to draw to the screen
    video.read(); // Read the new frame from the camera
    video.loadPixels(); // Make its pixels[] array available
    
    int movementSum = 0; // Amount of movement in the frame
    for (int i = 0; i < numPixels; i++) { // For each pixel in the video frame...
      color currColor = video.pixels[i];
      color prevColor = previousFrame[i];
      // Extract the red, green, and blue components from current pixel
      int currR = (currColor >> 16) & 0xFF; // Like red(), but faster
      int currG = (currColor >> 8) & 0xFF;
      int currB = currColor & 0xFF;
      // Extract red, green, and blue components from previous pixel
      int prevR = (prevColor >> 16) & 0xFF;
      int prevG = (prevColor >> 8) & 0xFF;
      int prevB = prevColor & 0xFF;
      // Compute the difference of the red, green, and blue values
      int diffR = abs(currR - prevR);
      int diffG = abs(currG - prevG);
      int diffB = abs(currB - prevB);
      // Add these differences to the running tally
      sumDiff = diffR + diffG + diffB;
      movementSum += sumDiff;
      // Render the difference image to the screen
      pixels[i] = color(diffR*4, diffG*4, diffB*4);
      // The following line is much faster, but more confusing to read
      //pixels[i] = 0xff000000 | (diffR << 16) | (diffG << 8) | diffB;
      // Save the current color into the 'previous' buffer
      previousFrame[i] = currColor;
      
      if (sumDiff > pixThresh) {
          numContPix += 1;
          pixY = floor(i/640);
          pixX = i - pixY*640;
          contX += pixX;
          contY += pixY;
      }
      
      
    }
    // To prevent flicker from frames that are all black (no movement),
    // only update the screen if the image has changed.
    if (movementSum > 0) {
      updatePixels();
      //println(movementSum); // Print the total amount of movement to the console
    }
    
    if (numContPix > 50) {
      comX = round(contX/numContPix);
      comY = round(contY/numContPix);
      
      modX = round((comX+lastX)/2); // smooth a little
      modY = round((comY+lastY)/2);
      
      fill(200);
      ellipse(modX, modY, 50, 50);
      lastX = modX;
      lastY = modY;

    }
    else {
      fill(200);
      ellipse(lastX, lastY, 50, 50);
    }
    
    pixX = 0; pixY = 0; contX = 0; contY = 0; comX = 0; comY = 0; numContPix = 0;
    
  }
}