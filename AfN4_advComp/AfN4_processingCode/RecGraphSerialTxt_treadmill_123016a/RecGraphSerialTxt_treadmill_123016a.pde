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

/////// Trial variables

String animal = "010317_mouseTest1";        
int sessionMinutes = 60;
String serialPort = "COM6";

int toPlot = 1;
int binsize = 20000;
int pan1scale = 5;
int pan2scale = 20;
int pan3scale = 20;

// position plot parameters
int posPlotXorig = 0;
int posPlotYorig = 0;
int posPlotWidth = 0;
int posPlotHeight = 0;

int mouseXpos = 0;

/////////////  
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
int event1binCt = 0;
int event2binCt = 0;
int event3binCt = 0;
int event4binCt = 0;
int event5binCt = 0;

int binLick = 0;
int binTag = 0;
int binRew = 0;

long eventTime = 0;

//ArrayList eventArray;
//ArrayList eventTimeArray;

//int rdbinLick = 0;
//int rdbinTag = 0;

int xOffset = 30;
int yOffset = 20;

String extension = "";
String filename = "";

long sessionLength = sessionMinutes*60000;

// new variables Dec. 2016
String velTxt;
float velSum = 0;
float vel = 0;
String posTxt;
int pos = 0;
int trackLen = 0;
float relPos = 0;
int rewBegPos = 0;
int rewEndPos = 0;

void setup() 
{
  size(800, 800);
  smooth();
  // Create a new file in the sketch directory
  extension = ".txt";
  filename = animal.concat(extension);
  output = createWriter(filename);
  println(Serial.list());
  myPort1 = new Serial(this, serialPort, 38400); 
  myPort1.bufferUntil('\n');
  
  // print out a bit of header info and draw axes
  output.println(animal);
  h = hour();
  m = minute();
  output.println("START time");
  output.print(h);
  output.print(":");
  if (m<=9) {
   output.println("0"+m); 
  }
  else {
  output.println(m);
  }

  println("START time");
  print(h);
  print(":");
  println(m);

  // DRAW and label graph axes
  drawAxes();
  
}

void draw() {      //draw() can be empty, everything's in SerialEvent

  time = millis();    // see how long it's been since program started

  //////////////
  // this section computes bin totals for each event category
  if (time >= j*binsize) {    // at the end of each timebin
    plotBinTotals();
  }
  
  updatePosition();

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
} // end void draw()

////// use serialEvent() because of intermittent Arduino data
void serialEvent(Serial myPort1) {  

  if (myPort1.available() > 0) {  // when serial data is sent from Arduino (when the animal does something)
    // read serial data of trigger times (since start of this script) from Arduino
    //inBuffer = new byte[14];  // don't need this because of following...
    event = myPort1.readStringUntil('\n');  // use readStringUntil() to make sure we get entire serial package
    // Write the Arduino data to a file
    output.print(event);  // write the output to .txt file as string (will just be number in text)
    print(event);  // print trigger times to this display
    
    // parse and plot things from incoming strings
    if (toPlot == 1) {
      if (int(event) == 0) {  // if this serial data is an event type
        parseString();
      } // end IF event is a string (prob not necessary now)    
    } // end IF to plot event data
    //      output.println(time);  // uncomment to also output Processing time
  }  // end IF for myPort1.available
}  // end loop for serial input/print object

///// ends program and saves output TXT (and graph)  
void keyPressed() {
  output.flush(); // Write the remaining data
  output.close(); // Finish the file
  println("END TRIAL");
  extension = ".png";
  filename = animal.concat(extension);
  saveFrame(filename);  // added 121010 to save graph  
  exit(); // Stop the program
  //  }  // end ELSE for writing end of file
}  // end keyPressed

// draw the axes for plotting
void drawAxes() {
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
    //line(xOffset, 3*height/4+30, xOffset, height-yOffset);  // bottom
    //line(xOffset, height-yOffset, width-30, height-yOffset);
    
    String panel1title = "    Licks";
    String panel2title = "    Laps";
    String panel3title = "    Rewards";
    String panel4title = "    Position";

    PFont font;
    font = loadFont("SimplifiedArabic-Bold-48.vlw");
    textFont(font, 16);
    text(panel1title, 10, 20);
    text(panel2title, 10, height/4+20);
    text(panel3title, 10, height/2+20);
    text(panel4title, 10, 3*height/4 + 20); 

    // graph axis labels
    textFont(font, 12);
    fill(0);
    text("0", 5, height/4-20);  // top graph
    text("5", 5, height/4-70);
    text("10", 5, height/4-120);
    text("15", 5, height/4-170);
    //text("20", 5, height/4-110);
    //text("12", 5, height/4-130);
    //text("14", 5, height/4-150);
    
    
    text("0", 5, height/2-60);
    text("20", 5, height/2-80);
    text("40", 5, height/2-100);
    text("60", 5, height/2-120);
    text("80", 5, height/2-140);
    text("-40", 5, height/2-20);
    text("-20", 5, height/2-40);
    
    text("0", 5, 3*height/4-20);
    text("20", 5, 3*height/4-40);
    text("40", 5, 3*height/4-60);
    text("60", 5, 3*height/4-80);
    text("80", 5, 3*height/4-100);
    text("100", 5, 3*height/4-120);
    
    
    //text("0", 5, height-20);
    //text("20", 5, height-40);
    //text("40", 5, height-60);
    //text("60", 5, height-80);
    //text("80", 5, height-100);
    //text("100", 5, height-120);
    
    // Draw track plot
    
    posPlotXorig = 30;
    posPlotYorig = 3*height/4+50;
    posPlotWidth = width-60;
    posPlotHeight = 60;
    
    //rectMode(CORNER);
    //fill(255, 100);
    ////posPlotXorig = 30;
    ////posPlotYorig = 3*height/4+50;
    ////posPlotWidth = width-60;
    ////posPlotHeight = 60;
    //rect(30, 3*height/4+50, width-60, 60); 

}

void plotBinTotals() {
  
    if (event2binCt != 0) {    // lick
      binLick = event2binCt;
    } 
    else {
      binLick = 0;
    }

    if (event3binCt != 0) {  // tag
      binTag = event3binCt;
      //rdbinTag = round(binTag);
    } 
    else {
      binTag = 0;
    }
    
    if (event4binCt != 0) {    // reward
      binRew = event4binCt;
      //rdbinLick = round(binLick*100);
    } 
    else {
      binRew = 0;
    }

    //binDiscInd = rdbinTag - rdbinLick;

    // display the data for the past bin
   
    // make bar with rect(xpos, ypos, xVal, yVal)
    // NOTE: CORNERS mode for rect format: rect(topleftX, topleftY, botrightX, botrightY)
    rectMode(CORNERS);
    fill(200);
    rect(j*10+xOffset, (height/4)-(binLick*pan1scale)-yOffset, (j*10)+xOffset+10, height/4-yOffset);  // panel 1 (top)
    fill(255);
    rect(j*10+xOffset, (height/2)-(binTag*pan2scale)-yOffset-40, (j*10)+xOffset+10, height/2-yOffset-40);  // panel 2
    fill(122);
    rect((j*10)+xOffset, (3*height/4)-(binRew*pan3scale)-yOffset, (j*10)+xOffset+10, 3*height/4-yOffset );  // panel 3
    //fill(44);
    //rect((j*10)+xOffset, height-binTag-yOffset, (j*10)+xOffset+10, height-yOffset );  // panel 4


    // then reset all bin counts
    event1binCt = 0;
    event2binCt = 0; 
    event3binCt = 0;
    event4binCt = 0;
    event5binCt = 0;

    j = j+1;    // increment the time bin number 
}

void parseString() {
  //String eventType = event;
  i = i + 1;  // count the events
  //eventArray.add(event);  // make an array list of all event strings
  
  String event1string = "vel=";
  String event2string = "lick";
  String event3string = "tag";
  String event4string = "REWARD";
  String event5string = "length=";
  String event6string = "rewBegPos=";
  String event7string = "rewEndPos=";
  

  if (event.contains(event1string)) {  // see what type of event
    eventType = 1;
    //println("break");
    event1binCt = event1binCt + 1;  // and count events in this bin
    
    // extract velocity
    int ind1 = event.indexOf("vel=");
    int ind2 = event.indexOf("currPos");  // string at end of "vel" number
    velTxt = event.substring(ind1+4,ind2-2);
    //println(velTxt);

    // convert to number
    vel = float(velTxt);
    // add to vel array for this timebin
    velSum = velSum + vel;
     
    // extract position
    int ind3 = event.indexOf("millis");
    posTxt = event.substring(ind2+8, ind3-2);
    pos = int(posTxt);
    
    // update position graph
    if (pos>=0) {
    relPos = float(pos)/trackLen;
    }
    else {
     relPos = float(trackLen+pos)/trackLen; 
    }
    //println(relPos);
    
    //updatePosition();
    
    //mouseXpos = 200; //round(relPos*float(posPlotWidth)+posPlotXorig);
    //println(relPos);
    //println(mouseXpos);
    //rectMode(CORNER);
    //fill(255);
    //rect(mouseXpos, posPlotYorig, 20, 60);
    
  } 
  else if(event.contains(event2string)) {  // lick
    eventType = 2; 
    event2binCt = event2binCt + 1;
  } 
  else if(event.contains( event3string)) {  // tag
    eventType = 3; 
    event3binCt = event3binCt + 1;
  } 
  else if(event.contains(event4string)) {  // reward
    eventType = 4;
    event4binCt = event4binCt + 1;
  } 
  else if(event.contains(event5string)) {  // track length
    int evLen = event5string.length();//println(evLen);
    int evInd = event.indexOf(event5string);//println(evInd);
    String evTxt = event.substring(evInd+evLen, event.length()-2);println(evTxt); //println(evTxt.indexOf("7"));
    trackLen = int(evTxt);
    println("trackLength = " + trackLen);
  } // end ELSE for event type
  else if(event.contains(event6string)) {  // rewBegPos
    int evLen = event6string.length();
    int evInd = event.indexOf(event6string);
    String evTxt = event.substring(evInd+evLen, event.length()-2);
    rewBegPos = int(evTxt);
  } // end ELSE for event type
  else if(event.contains(event7string)) {  // rewEndPos
    int evLen = event7string.length();//println(evLen);
    int evInd = event.indexOf(event7string);//println(evInd);
    String evTxt = event.substring(evInd+evLen, event.length()-2);//println(evTxt);
    rewEndPos = int(evTxt);
  } // end ELSE for event type
} // end parseString();


void updatePosition() {
  
  // redraw track to clear each frame
  rectMode(CORNER);
  fill(255, 100);
  //posPlotXorig = 30;
  //posPlotYorig = 3*height/4+50;
  //posPlotWidth = width-60;
  //posPlotHeight = 60;
  rect(30, 3*height/4+50, width-60, 60); 
  
  // and draw reward zone
  int rewXbeg = round(float(rewBegPos)/trackLen*(posPlotWidth)+posPlotXorig);
  int rewXend = round(float(rewEndPos)/trackLen*(posPlotWidth)+posPlotXorig);
  
  rectMode(CORNERS);
  fill(200);
  rect(rewXbeg, posPlotYorig, rewXend, posPlotYorig+60);
  
  
  mouseXpos = round(relPos*float(posPlotWidth-30)+posPlotXorig);
  //println(relPos);
  //println(mouseXpos);
  rectMode(CORNER);
  fill(100);
  rect(mouseXpos, posPlotYorig, 20, 60);
  
}