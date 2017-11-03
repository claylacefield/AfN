/**
 * headFix
 * 
 * This sketch uses a PrintWriter object to write serial data from the Arduino continuously to a file
 * over the course of a 10min behavioral session. After the end of this period, the file closes
 * itself and the program is stopped. 
 * SOURCE SKETCHES: 
 * SaveFile2 (for saving the serial stream to a file), read() (for serial read)
 * 8/15/09: everything seems to be working,
 * NOTE: must rename .txt file of mouse behav data or it will be overwritten
 * ALSO: if you change the Arduino serial ouput, make certain the variables are "long", else
 *       one might overrun the variable and get weird times out.
 * BUG 082609: still have the problem where even if 10min has elapsed, program continues until
 * next piece of serial data is sent by Arduino, BECAUSE I'm doing everything in the SerialEvent 
 * function, which only executes when it gets a piece of serial data. Also, if last trigger is a 
 * REWARD trigger, it opens the feeder valve and leaves it open, flooding the cage.
 **/

import processing.serial.*;    // import the Processing serial library
PrintWriter output;    // initialize the PrintWriter Java object (don't know much about this)

Serial myPort;    // the serial port

long time=0;    // initialize variable for the current time since program started
String trigTime;  // initialize variable for serial data from Arduino
int mon =0;
int d = 0;
int h = 0;
int m = 0;
int n = 0;


void setup() 
{
  // Create a new file in the sketch directory
  output = createWriter("060610_mouse1.txt");
  println(Serial.list());
  myPort = new Serial(this, Serial.list()[0], 9600);  // might need to specify COM13, or Serial.list()[4 or 3] on laptop
  myPort.bufferUntil('\n');
}

void draw() {      //draw() can be empty, everything's in SerialEvent
  while (n==0) {
    mon=month();
    d = day();
    h = hour();
    m = minute();
    output.println("START time");
    output.print(mon);
    output.print("/");
    output.println(d);
    output.print(h);
    output.print(":");
    output.println(m);
    n = 1;
  }
}

void serialEvent(Serial myPort) {  // use serialEvent() because of intermittent Arduino data
  time = millis();    // see how long it's been since program started
  //  output.println(time);
  //  println(time);
  //  if (time < 1200000)   // conditional for length of trial (usu. 10min or 600K msec)
  //  {
  if (myPort.available() > 0) {  // when serial data is sent from Arduino (when the animal does something)
    // read serial data of trigger times (since start of this script) from Arduino
    //inBuffer = new byte[14];  // don't need this because of following...
    trigTime = myPort.readStringUntil('\n');  // use readStringUntil() to make sure we get entire serial package
    // Write the Arduino data to a file
    output.print(trigTime);  // write the output to .txt file as string (will just be number in text)
    print(trigTime);  // print trigger times to this display
    //      output.println(time);  // uncomment to also output Processing time
  }
  //  loop();    // loops anyway, so don't need
  //  }  // end loop for total duration
}  // end loop for serial input/print object
//  else {
void keyPressed() {
  output.flush(); // Write the remaining data
  output.close(); // Finish the file
  println("END TRIAL");
  exit(); // Stop the program
  //  }  // end ELSE for writing end of file
}  // end keyPressed



