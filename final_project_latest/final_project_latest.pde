import processing.serial.*;
import processing.sound.*;

PImage explosionGif;


  Serial myPort;        // The serial port
  boolean explode;
  String inString = "";
  SoundFile explosion;
  SoundFile yay;
  SoundFile win;
  SoundFile ticking;
  boolean startMode = false;
  int lastTime = 0;
  int currentTime = 0;
  
  //button
  float x = 100;
  float y = 50;
  float w = 150;
  float h = 80;
  

void setup () {
    // set the window size:
    size(800, 800);
    // Open whatever port is the one you're using.
    myPort = new Serial(this, Serial.list()[3], 9600);

    // don't generate a serialEvent() unless you get a newline character:
    myPort.bufferUntil('\n');
    
    explosion = new SoundFile(this, "explosion.mp3");
    yay = new SoundFile(this, "nextlevelyay.wav");
    win = new SoundFile(this, "defusedyay.mp3");
    ticking = new SoundFile(this, "ticking.mp3");
    
    //file.play();
    
    explosionGif = loadImage( "explosion.gif" );

    
  }

  void draw() {
    if (startMode == true){
      println("this is supposed to be working");
      myPort.write("start"); // this sends data to arduino
    }
    if (startMode == false){
      image( explosionGif, 0, 0, width, height );
    }

    rect(x,y,w,h);
    fill(255);
    text("Press to Start", x,y,w,h);
    fill(0);
    textSize(20);
    if(mousePressed){
     if(mouseX>x && mouseX <x+w && mouseY>y && mouseY <y+h){
      println("The mouse is pressed and over the button");
      fill(255);
      //do stuff
      startMode = true;
      myPort.write('1');
     }
    } 
    currentTime = millis();
    
    if (inString.trim().equals("Time ran out, you lose")){
      explosion.play();
      inString = "";
      startMode = false;
    }
    if (inString.trim().equals("Yay")){
      yay.play();
      inString = "";
    }
    if (inString.trim().equals("You Win")){
      win.play();
      inString = "";
      startMode = false;
    }
    
    //DO NOT CHANGE THIS NUMBER AT ALL COSTS
    /*else if (currentTime - lastTime >= 6350){
      ticking.play();
      lastTime = currentTime;
    }*/
    
  }
  void serialEvent (Serial myPort) {
    // get the ASCII string:
    inString = myPort.readStringUntil('\n');
    println(inString);
    
  }
  
