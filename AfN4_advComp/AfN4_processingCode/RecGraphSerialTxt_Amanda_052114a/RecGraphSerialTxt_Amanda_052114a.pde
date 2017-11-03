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


// NOTE: all these variables are global and therefore are modified inside all classes/methods
import processing.serial.*;    // import the Processing serial library
PrintWriter output;    // initialize the PrintWriter Java object (don't know much about this)

Serial myPort1;    // the serial port

long time=0;    // initialize variable for the current time since program started
String event;  // initialize variable for serial data from Arduino

int h = 0; // variable for recording hour of start/stop
int m = 0; // variable for recording minute of start/stop
int n = 0; 

int i = 0;  // variable for incrementing event strings and times
int j = 0;  // variable for incrementing time bins
int eventType = 0;  // variable for event type number
int binCtLev = 0;
float binCtTop = 0;
float binCtBot = 0;
float binCtUnrew = 0;
float binCtRew = 0;

float binPctUnrew = 0;
float binPctRew = 0;
int binDiscInd = 0;

long eventTime = 0;
//int eventType = 0;

ArrayList eventArray;
ArrayList eventTimeArray;

int rdBinPctUnrew = 0;
int rdBinPctRew = 0;

int xOffset = 30;
int yOffset = 20;

int binsize = 60000;

String animal = "052314_mouseR112";                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
String extension = "";
String filename = "";

int sessionMinutes = 60;
long sessionLength = sessionMinutes*60000;

void setup() 
{
  size(800, 800);
  smooth();
  // Create a new file in the sketch directory
  extension = ".txt";
  filename = animal.concat(extension);
  output = createWriter(filename);
  println(Serial.list());
  myPort1 = new Serial(this, "COM10", 9600); 
  myPort1.bufferUntil('\n');
}

void draw() {      //draw() can be empty, everything's in SerialEvent
  if (n==0) {
    output.println(animal);
    h = hour();
    m = minute();
    output.println("START time");
    output.print(h);
    output.print(":");
    output.println(m);

    println("START time");
    print(h);
    print(":");
    println(m);

    // DRAW and label graph axes
    //    line(200, 0, 200, 400);  // quadrant lines for diff graphs
    line(0, height/4, width, height/4);
    line(0, height/2, width, height/2);
    line(0, 3*height/4, width, 3*height/4);

    line(xOffset, 30, xOffset, height/4-yOffset);  // top graph vertical axis
    line(xOffset, height/4-20, width-30, height/4-yOffset);  // horizontal
    line(xOffset, height/4+30, xOffset, height/2-yOffset);  // second
    line(xOffset, height/2-yOffset-40, width-30, height/2-yOffset-40);
    line(xOffset, height/2+30, xOffset, 3*height/4-yOffset);  // third
    line(xOffset, 3*height/4-yOffset, width-30, 3*height/4-yOffset); 
    line(xOffset, 3*height/4+30, xOffset, height-yOffset);  // bottom
    line(xOffset, height-yOffset, width-30, height-yOffset);

    PFont font;
    font = loadFont("SimplifiedArabic-Bold-48.vlw");
    textFont(font, 16);
    text("    number stim triggers (by lever hold)", 10, 20);
    text("    Disc. Index", 10, height/4+20);
    text("    Pct. unrewarded lever presses", 10, height/2+20);
    text("    Pct. rewarded lever presses", 10, 3*height/4 + 20); 

    // graph axis labels
    textFont(font, 12);
    fill(0);
    text("0", 5, height/4-20);  // top graph
    text("0", 5, height/2-60);
    text("0", 5, 3*height/4-20);
    text("0", 5, height-20);
    text("5", 5, height/4-70);  // top graph
    text("20", 5, height/2-80);
    text("20", 5, 3*height/4-40);
    text("20", 5, height-40);
    text("10", 5, height/4-120);  // top graph
    text("40", 5, height/2-100);
    text("40", 5, 3*height/4-60);
    text("40", 5, height-60);
    text("15", 5, height/4-170);  // top graph
    text("60", 5, height/2-120);
    text("60", 5, 3*height/4-80);
    text("60", 5, height-80);
    //text("20", 5, height/4-110);  // top graph
    text("80", 5, height/2-140);
    text("80", 5, 3*height/4-100);
    text("80", 5, height-100);
    //text("12", 5, height/4-130);  // top graph
    text("-40", 5, height/2-20);
    text("100", 5, 3*height/4-120);
    text("100", 5, height-120);
    //text("14", 5, height/4-150);  // top graph
    text("-20", 5, height/2-40);
    //    text("14", 5, 3*height/4-150);
    //    text("14", 5, height-150);

    n = 1;
  }

  time = millis();    // see how long it's been since program started

  //////////////
  // this section computes bin totals for each event category
  if (time >= j*binsize) {    // at the end of each timebin

    if (binCtTop != 0) {
      binPctUnrew = binCtUnrew/binCtTop;
      rdBinPctUnrew = round(binPctUnrew*100);
    } 
    else {
      binPctUnrew = 0;
    }

    if (binCtBot != 0) { // && binCtUnrew != 0) {
      binPctRew = binCtRew/binCtBot*100;
      //      println("binPctRew");
      //      println(binPctRew);
      rdBinPctRew = round(binPctRew);
    } 
    else {
      binPctRew = 0;
    }

    binDiscInd = rdBinPctRew - rdBinPctUnrew;

    // display the data for the past bin
    fill(200);
    // make bar with rect(xpos, ypos, xVal, yVal)
    // NOTE: CORNERS mode for rect format: rect(topleftX, topleftY, botrightX, botrightY)
    rectMode(CORNERS);
    rect(j*10+xOffset, (height/4)-(binCtLev*10)-yOffset, (j*10)+xOffset+10, height/4-yOffset);    
    fill(255);
    rect(j*10+xOffset, (height/2)-(binDiscInd)-yOffset-40, (j*10)+xOffset+10, height/2-yOffset-40);
    fill(122);
    rect((j*10)+xOffset, (3*height/4)-rdBinPctUnrew-yOffset, (j*10)+xOffset+10, 3*height/4-yOffset );
    fill(44);
    rect((j*10)+xOffset, height-rdBinPctRew-yOffset, (j*10)+xOffset+10, height-yOffset );


    // then reset all bin counts
    binCtLev = 0;
    binCtTop = 0; 
    binCtBot = 0;
    binCtUnrew = 0;
    binCtRew = 0;

    j = j+1;    // increment the time bin number
  }

  // if total time has elapsed, then write everything to file
  if (time >= sessionLength) {
    output.flush(); // Write the remaining data
    output.close(); // Finish the file
    println("END TRIAL");
    extension = ".png";
    filename = animal.concat(extension);
    saveFrame(filename);  // added 121010 to save graph
    exit(); // Stop the program
  }
}

void serialEvent(Serial myPort1) {  // use serialEvent() because of intermittent Arduino data
  //time = millis();    // see how long it's been since program started
  //  output.println(time);
  //  println(time);
  //  if (time < 1200000)   // conditional for length of trial (usu. 10min or 600K msec)
  //  {
  if (myPort1.available() > 0) {  // when serial data is sent from Arduino (when the animal does something)
    // read serial data of trigger times (since start of this script) from Arduino
    //inBuffer = new byte[14];  // don't need this because of following...
    event = myPort1.readStringUntil('\n');  // use readStringUntil() to make sure we get entire serial package
    // Write the Arduino data to a file
    output.print(event);  // write the output to .txt file as string (will just be number in text)
    print(event);  // print trigger times to this display
    if (int(event) == 0) {  // if this serial data is an event type
      //String eventType = event;
      i = i + 1;  // count the events
      //eventArray.add(event);  // make an array list of all event strings

      if (event.contains("start")) {  // see what type of event
        eventType = 1;
        //println("break");
        binCtLev = binCtLev + 1;  // and count events in this bin
      } 
      else if(event.contains("top") || event.contains("catch")) {
        eventType = 2; 
        //println("top");
        binCtTop = binCtTop + 1;
      } 
      else if(event.contains("bottom")) {
        eventType = 3; 
        //println("bottom");
        binCtBot = binCtBot + 1;
      } 
      else if(event.contains("unrewarded")) {
        eventType = 4;
        //println("unrewarded");
        binCtUnrew = binCtUnrew + 1;
      } 
      else if(event.contains("REWARD!!")) {
        eventType = 5;
        //println("REWARD!!");
        binCtRew = binCtRew + 1;
      } // end ELSE for event type

    } 
    else {    // else if the serial data is a number (therefore an event time)
      eventTime = int(event);
      eventTimeArray.add(eventTime);  // record the time of the event in an array list
      //if (eventTime >= j*60000) {
      //j = j+1;
      // increment event number for this type for this bin

      //}    // end IF for which time bin
    }    // end ELSE for serial data is number/time
    //      output.println(time);  // uncomment to also output Processing time
  }  // end IF for myPort1.available
  //  }  // end IF for session duration
  //
  //  else {
  //    output.flush(); // Write the remaining data
  //    output.close(); // Finish the file
  //    println("END TRIAL");
  //    exit(); // Stop the program
  //  }
}  // end loop for serial input/print object
//  else {
void keyPressed() {
  output.flush(); // Write the remaining data
  output.close(); // Finish the file
  println("END TRIAL");
  extension = ".png";
  filename = animal.concat(extension);
  saveFrame(filename);  // added 121010 to save graph  // added 121010 to save graph f
  exit(); // Stop the program
  //  }  // end ELSE for writing end of file
}  // end keyPressed












