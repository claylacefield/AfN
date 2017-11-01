/**
 * headFix
 * 
 * This sketch uses a PrintWriter object to write serial data from the Arduino continuously to a file
 * over the course of a 10min behavioral session. After the end of this period, the file closes
 * itself and the program is stopped. 
 * SOURCE SKETCHES: 
 * SaveFile2 (for saving the serial stream to a file), read() (for serial read)
 * NOTE: must rename .txt file of mouse behav data or it will be overwritten
 * ALSO: if you change the Arduino serial ouput, make certain the variables are "long", else
 *       one might overrun the variable and get weird times out.

 
 Types of events to look for:
 - nosepokes
 - licks (whether rewarded or not), on one port or another
 - rewards
 - % correct (overall)
 
 These are in serialEvent, around line 200
 
 040117:
 Trying to make this script more generic, to make it easier to modify for different behaviors.
 - now using default font for text, so don't need to put font in data folder for script
 - putting together axes info for each plot, so that you can change them all together (or to add/delete a whole plot? still have to figure out how to do this)
 - making scale variable for each plot, to change Y axis ticks along with scaling for each plot
 
 **/


// NOTE: all these variables are global and therefore are modified inside all classes/methods

/////// Trial variables

String animal = "052314_mouseR112";        
int sessionMinutes = 60;
String comPort = "COM3";

/////////////  
// other variables to make the program easier to change

int numPanels = 4;  // number of panels to plot
int sizeY = numPanels*200;

// these are tagstrings for identifying events
String event1tag = "nosepoke";
String event2tag = "lick";
String event3tag = "REWARD";
String event4tag = "port = 1";

int plotScale1 = 10;  // #pix corresponding to variable value of 1
int plotScale2 = 10;
int plotScale3 = 10;
int plotScale4 = 10;

import processing.serial.*;    // import the Processing serial library
PrintWriter output;    // initialize the PrintWriter Java object (don't know much about this)

Serial myPort1;    // the serial port

long time=0;    // initialize variable for the current time since program started
String event;  // initialize variable for serial data from Arduino

int h = 0; // variable for recording hour of start/stop
int m = 0; // variable for recording minute of start/stop
String m2;
int n = 0; 

int i = 0;  // variable for incrementing event strings and times
int j = 0;  // variable for incrementing time bins
int eventType = 0;  // variable for event type number

// event counters
int event1binCt = 0;
float event2binCt = 0;
float event3binCt = 0;
float event4binCt = 0;
float event5binCt = 0;

int event6binCt = 0;
int event7binCt = 0;

// and percents for plotting
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

String extension = "";
String filename = "";

long sessionLength = sessionMinutes*60000;

void setup() 
{
  // set up plotting window
  size(800, 800);
  //surface.setResizable(true);
  //surface.setSize(800, sizeY);  // this allows you to set a variable size in Processing 3.x
  smooth();
  
  // Create a new file in the sketch directory and set up serial port
  extension = ".txt";
  filename = animal.concat(extension);
  output = createWriter(filename);
  println(Serial.list());
  myPort1 = new Serial(this, comPort, 9600); 
  myPort1.bufferUntil('\n');
  
  drawAxes(); // draw the axes for subsequent plotting
  
  // print out start time from computer
  output.println(animal);
  h = hour();
  m = minute();
  if(m<9) {m2 = "0" + str(m);}
  else{m2 = str(m);}
  
  output.println("START time");
  output.print(h);
  output.print(":");
  output.println(m2);
  println("START time");
  print(h);
  print(":");
  println(m2);
  
}

void draw() {      //draw() can be empty, everything's in SerialEvent
 
  time = millis();    // see how long it's been since program started

  //////////////
  // this section computes bin totals for each event category
  if (time >= j*binsize) {    // at the end of each timebin
    
    // for variables that are not simple counts of events
    if (event2binCt != 0) {  // if/else to avoid dividing by zero if no events of a certain type
      binPctUnrew = event4binCt/event2binCt;
      rdBinPctUnrew = round(binPctUnrew*100);
    } 
    else {
      binPctUnrew = 0;
    }

    if (event3binCt != 0) { // && event4binCt != 0) {
      binPctRew = event5binCt/event3binCt*100;
      //      println("binPctRew");
      //      println(binPctRew);
      rdBinPctRew = round(binPctRew);
    } 
    else {
      binPctRew = 0;
    }

    binDiscInd = rdBinPctRew - rdBinPctUnrew;

    // display the data for the past bin

    // make bar with rect(xpos, ypos, xVal, yVal)
    // NOTE: CORNERS mode for rect format: rect(topleftX, topleftY, botrightX, botrightY)
    rectMode(CORNERS);
    
    // plot #1
    fill(200);
    rect(j*10+xOffset, (height/4)-(event6binCt*plotScale1)-yOffset, (j*10)+xOffset+10, height/4-yOffset);   
    // plot #2
    fill(255);
    rect(j*10+xOffset, (height/2)-(event7binCt*plotScale2)-yOffset, (j*10)+xOffset+10, height/2-yOffset);
    
    // plot #3
    //fill(122);
    //rect((j*10)+xOffset, (3*height/4)-rdBinPctUnrew-yOffset, (j*10)+xOffset+10, 3*height/4-yOffset );
    
    // plot #4
    //fill(44);
    //rect((j*10)+xOffset, height-rdBinPctRew-yOffset, (j*10)+xOffset+10, height-yOffset );


    // then reset all bin counts
    event1binCt = 0;
    event2binCt = 0; 
    event3binCt = 0;
    event4binCt = 0;
    event5binCt = 0;
    
    event6binCt = 0;
    event7binCt = 0;

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

////////// parse serial input to extract event info
void serialEvent(Serial myPort1) {  // use serialEvent() because of intermittent Arduino data

  if (myPort1.available() > 0) {  // when serial data is sent from Arduino (when the animal does something)
    // read serial data of trigger times (since start of this script) from Arduino
    //inBuffer = new byte[14];  // don't need this because of following...
    event = myPort1.readStringUntil('\n');  // use readStringUntil() to make sure we get entire serial package, until endline
    // Write the Arduino data to a file
    output.print(event);  // write the output to .txt file as string (will just be number in text)
    print(event);  // print trigger times to this display
    if (int(event) == 0) {  // if this serial data is an event type (i.e. not a number, which when converted would not be = 0)
      //String eventType = event;
      i = i + 1;  // count the events (may not be using this right now)
      //eventArray.add(event);  // make an array list of all event strings
      
      String event1string = "start";
      String event2string = "top";
      String event3string = "bottom";
      String event4string = "unrewarded";
      String event5string = "REWARD";
      
      String event6string = "lickPort=1";
      String event7string = "lickPort=2";
      

      if (event.contains(event1string)) {  // see what type of event
        eventType = 1;
        event1binCt = event1binCt + 1;  // and count events in this bin
      } 
      else if(event.contains(event2string)) {
        eventType = 2; 
        event2binCt = event2binCt + 1;
      } 
      else if(event.contains( event3string)) {
        eventType = 3; 
        event3binCt = event3binCt + 1;
      } 
      else if(event.contains(event4string)) {
        eventType = 4;
        event4binCt = event4binCt + 1;
      } 
      else if(event.contains(event5string)) {
        eventType = 5;
        event5binCt = event5binCt + 1;
        
        if(event.contains(event6string)) { // for lickPort=1 reward
          event6binCt = event6binCt + 1;
        }
        else if(event.contains(event7string)) {  // for lickPort=2 reward
          event7binCt = event7binCt + 1;
        }
        
        
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

}  // end loop for serial input/print object

///////// to end via key press
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

//////////
void drawAxes() {
  // DRAW and label graph axes
  
    String panel1title = "    Reward, lickPort=1";
    String panel2title = "    Reward, lickPort=2";
    String panel3title = "    (not used)";
    String panel4title = "    (not used)";
  
  
    //    line(200, 0, 200, 400);  // quadrant lines for diff graphs
    line(0, height/4, width, height/4);
    line(0, height/2, width, height/2);
    line(0, 3*height/4, width, 3*height/4);

    line(xOffset, 30, xOffset, height/4-yOffset);  // top graph vertical axis
    line(xOffset, height/4-20, width-30, height/4-yOffset);  // horizontal
    
    line(xOffset, height/4+30, xOffset, height/2-yOffset);  // second
    line(xOffset, height/2-yOffset, width-30, height/2-yOffset);   // xIntercept = -40  ; height/2-yOffset-40
    
    line(xOffset, height/2+30, xOffset, 3*height/4-yOffset);  // third
    line(xOffset, 3*height/4-yOffset, width-30, 3*height/4-yOffset); 
    
    line(xOffset, 3*height/4+30, xOffset, height-yOffset);  // bottom
    line(xOffset, height-yOffset, width-30, height-yOffset);
    


    //PFont font;
    //font = loadFont("SimplifiedArabic-Bold-48.vlw");
    //textFont(font, 16);
    textSize(16);
    text(panel1title, 10, 20);
    text(panel2title, 10, height/4+20);
    text(panel3title, 10, height/2+20);
    text(panel4title, 10, 3*height/4 + 20); 

    // graph axis labels
    //textFont(font, 12);
    textSize(12);
    fill(0);
    text("0", 5, height/4-20);  // top graph
    text("5", 5, height/4-70);  // top graph
    text(str(plotScale1), 5, height/4-120);  // top graph
    text("15", 5, height/4-170);  // top graph
    //text("20", 5, height/4-110);  // top graph
    //text("12", 5, height/4-130);  // top graph
    //text("14", 5, height/4-150);  // top graph
    
    
    text("0", 5, height/2-60);
    text("20", 5, height/2-80);
    text("40", 5, height/2-100);
    text("60", 5, height/2-120);
    text("80", 5, height/2-140);
    text("-40", 5, height/2-20);
    text("-20", 5, height/2-40);
    
    // label plot#3
    for (int j = 0; j<11; j = j+2) {
      text(str(plotScale3*j), 5, 3*height/4-(plotScale3*j+20));
    }
    
    

    text("0", 5, height-20);
    text("20", 5, height-40);
    text("40", 5, height-60);
    text("60", 5, height-80);
    text("80", 5, height-100);
    text("100", 5, height-120);
    
    
    //    text("14", 5, 3*height/4-150);
    //    text("14", 5, height-150);
}