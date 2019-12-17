/*TIMER IMPORT LINKS AND STUFF*/
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
Adafruit_7segment matrix = Adafruit_7segment();
uint16_t counter;
unsigned long lastTime = 0;
unsigned long currentTime = 0;
char processingVal;
/* RED = 0, YELLOW = 1, GREEN = 2, BLUE = 3*/
boolean pressed[] = {false, false, false, false}; //RED0, YELLOW1, GREEN2, BLUE3
int buttons[] = {6, 3, 4, 5}; //RED, YELLOW, GREEN, BLUE
int lights[] = {8, 12, 11, 10}; //RED, YELLOW, GREEN, BLUE
String colors[] = {"red", "yellow", "green", "blue"};
size_t lightsLength = sizeof(lights)/sizeof(lights[0]); //this works for both buttons & lights bc they are same length
/*INSTRUCTION SETS*/
int instructionMaster[8][4] = {  {6, 4, 5, 3},{6, 5, 4, 3},{5,6,4,3}, {3,4,6,5} , {4,3,5,6}, {5,6,3,4}, {6,3,4,5}, {4, 6, 5, 3} }; //overall instruction set
size_t masterLength = sizeof(instructionMaster)/sizeof(instructionMaster[0]); 
/*STATE TRACKING VARIABLES: THESE NEED TO BE RESET WHEN CONDITIONS ARE MET
Not in here but also needs to be reset: pressed array*/
int stepTracker = 0;
int instructionTracker = 0;
String gameState; //"win" or "lose"
boolean startGame = false;
/*GLOBAL EMPTY VAR*/
int redSensorVal;
int blueSensorVal;
int greenSensorVal;
int yellowSensorVal;
int buttonPressed;
void setup() {
  //start serial connection
  Serial.begin(9600);
  for (int i=0;i<lightsLength;i++){
    pinMode(lights[i], OUTPUT);
    pinMode(buttons[i], INPUT_PULLUP);
  }
  //TURN ALL THE LIGHTS ON TO BEGIN WITH
  lightsOn();
  //START THE TIMER MATRIX
  matrix.begin(0x70);
  counter = 60; //initialize the counter
}
void loop() {  
  /*READ DATA FROM PROCESSING*/
  if (Serial.available()) { // If data is available to read,
    processingVal = Serial.read();
    if (processingVal == '1'){
      startGame = true;
    }
    
    //startGame = true;
  }
  if (startGame == true){ //start the game
    currentTime = millis(); //time tracking purposes. use instead of delay
    
    /*WIN SEQUENCE. if you win, reset everything [steptracker, instructiontracker, pressed] 
    and blink the lights 3 times, then turn them back on again for resetting of the game. also reset timer*/
    if (instructionTracker >= masterLength){
      resetAll();
      for (int i=0;i<3;i++){
        lightsOn();
        lightsOff();
      }
      lightsOn();
      Serial.println("You Win");
    }
    /*if the sequence has been successfully completed and levels still have to be completed,
    show a flashing pattern, reset the steptracker & pressed sequence, turn all the lights on again
    and increment the instructiontracker*/
    if (stepTracker == lightsLength){
      for (int i=0;i<lightsLength;i++){
        digitalWrite(lights[i], HIGH);
        delay(100);
        digitalWrite(lights[i], LOW);
      }
      lightsOn(); //turn all the lights back on for the next level
      Serial.println("Yay");
      delay(300); //need this here so if they put their finger on it for too long it doens't detonate
      instructionTracker += 1;
      moveOnReset();
      
    }
    
    /*always check to see if any pressed is true and if they are turn those lights off*/
    for (int i=0;i<lightsLength;i++){
      if (pressed[i] == true){
        digitalWrite(lights[i], LOW);
      }
    }
    
    /*read the pushbutton value into a variable*/
    redSensorVal = digitalRead(buttons[0]);
    yellowSensorVal = digitalRead(buttons[1]);
    greenSensorVal = digitalRead(buttons[2]);
    blueSensorVal = digitalRead(buttons[3]);
  
    /*get the number of the button being pressed at any given moment. if none are pressed, this returns -1*/
    buttonPressed = buttonCheck();
    
    /*IF THE RIGHT BUTTON IS PRESSED, THEN CHANGED THE ASSOCIATED PRESSED STATE TO TRUE AND INCREMENT THE STEPTRACKER*/
  
    if (buttonPressed == instructionMaster[instructionTracker][stepTracker]){ 
      int rightIndex = findIndex(buttons, buttonPressed);
      pressed[rightIndex] = true;
      stepTracker += 1;
    }
  
    /*LOSING CONDITION 1: flash all lights then turn them off, reset pressed sequence, reset steptracker,
    reset instructiontracker, turn all lights off and turn them back on. 
    DON'T PUNISH THEM FOR PUSHING AN ALREADY DEACTIVATED BUTTON*/
    
    else if (buttonPressed > 0 && buttonPressed != instructionMaster[instructionTracker][stepTracker]){
      //FIRST CHECK TO SEE IF THE BUTTON IS DEACTIVATED. IF IT IS, THEN DON'T PUNISH THEM FOR PUSHING IT AGAIN
      int rightIndex2 = findIndex(buttons, buttonPressed);
      //only  execute thius code if the button is wrong and they haven't pushed an already deactivated button
      if (pressed[rightIndex2] == false){
        resetAll();  
        for (int i=0;i<lightsLength;i++){
          digitalWrite(lights[i], HIGH);
          delay(100);
          digitalWrite(lights[i], LOW);
        }
        lightsOn();
        Serial.println("Time ran out, you lose");
      }
      
    }
  
    /*LOSING CONDITION 2 if the clock runs out*/  
    if (counter == 0){
      for (int i=0;i<lightsLength;i++){
          digitalWrite(lights[i], HIGH);
          delay(100);
          digitalWrite(lights[i], LOW);
      }
      lightsOn(); 
      resetAll();
      Serial.println("Time ran out, you lose");
    }
  
    
  
    /*CLOCK CODE*/
    boolean drawDots = true;
    matrix.writeDigitNum(0, 0, drawDots);
    matrix.writeDigitNum(1, (int)(counter / 60), drawDots);
    matrix.drawColon(drawDots);
    matrix.writeDigitNum(3, (int)((counter % 60) / 10), drawDots);
    matrix.writeDigitNum(4, (int)((counter % 60) % 10), drawDots);
    //matrix.println(counter);
    matrix.writeDisplay();
    if (currentTime - lastTime >= 1000){
      counter -=1;
      lastTime = currentTime;
    }
    /*END CLOCK CODE*/
  } //bracket for ending start game bracket
  else{ //else if startGame is false
    //make the clock 00 
    boolean drawDots2 = true;
    matrix.writeDigitNum(0, 0, drawDots2);
    matrix.writeDigitNum(1, 0, drawDots2);
    matrix.drawColon(drawDots2);
    matrix.writeDigitNum(3, 0, drawDots2);
    matrix.writeDigitNum(4, 0, drawDots2);
    //matrix.println(counter);
    matrix.writeDisplay();
    
  }
}
/**START OF HELPER FUNCTIONS**/
/*FUNCTION: RETURN VALUE OF BUTTON PRESSED*/
int buttonCheck() { 
  //check if any of the four buttons are being pressed
  if (redSensorVal == LOW) {
    return buttons[0];
  } 
  else if (yellowSensorVal == LOW) {
    return buttons[1];
  } 
  else if (greenSensorVal == LOW) {
    return buttons[2];
  } 
  else if (blueSensorVal == LOW) {
    return buttons[3];
  } 
  else {
    return -1; //this will be the value for no button being pressed
  }
}
/*FUNCTION: RETURN INDEX OF SPECIFIED VALUE IN AN ARRAY*/
int findIndex(int list[], int &wantedVal){
  int wantedpos = -1;
  for (int i=0; i<lightsLength; i++) {
     if (wantedVal == list[i]) {
       wantedpos = i;
       break;
     }
  }
  return wantedpos;
}
/*FUNCTION: TURN ALL THE LIGHTS ON*/
void lightsOn(){
  digitalWrite(lights[0], HIGH); //turn on red 
  digitalWrite(lights[1], HIGH);//turn on yellow 
  digitalWrite(lights[2], HIGH);//turn on green  
  digitalWrite(lights[3], HIGH);//turn on blue 
}
void lightsOff(){
  digitalWrite(lights[0], LOW); //turn off red 
  digitalWrite(lights[1], LOW);//turn off yellow 
  digitalWrite(lights[2], LOW);//turn off green  
  digitalWrite(lights[3], LOW);//turn off blue 
}
void pressedFalse(){
  for (int i=0;i<lightsLength;i++){
    pressed[i] = false;
  }
}
void resetAll(){
  stepTracker = 0;
  instructionTracker = 0;
  pressedFalse();
  counter = 60;
  // this isn't working rn lol
  startGame = false;
}
void moveOnReset(){
  stepTracker = 0;
  pressedFalse();
  counter = 60;
}
