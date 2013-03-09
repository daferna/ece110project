#include <EEPROM.h>
#include <PID_v1.h>
#include <avr/eeprom.h>

//TODO: Get rid of motor macros, since motor control should only be in loop()

//Port Declarations
int leftMotor = 5;
int rightMotor = 6;
int hBridgeRight = 2;
int hBridgeLeft = 3;
int ledGreySplit = 11;
int ledWhiteSplit = 12;

//Sensor Declarations
byte sensorPin[] = {A0,A1,A2,A3,A4};

//String Declarations

//Constants Declarations
enum robotOp { normal, CW, CCW, halt };
robotOp state = normal;
int timeCount = 0;
int splits = 0;

enum side {left, right};
side powerSide = left;

void setup() {
  //Initialize sensors and motors
  pinMode(leftMotor,OUTPUT);
  pinMode(rightMotor,OUTPUT);
  pinMode(hBridgeRight,OUTPUT);
  pinMode(hBridgeLeft,OUTPUT);
  for(int i=0; i<(sizeof(sensorPin)/sizeof(byte)); i++){ //TODO: check that this works
    pinMode(sensorPin[i],INPUT);
  }
  //We delcare the two outer IR sensors seperately, as an indication that they will not be in the sensor[] array
  pinMode(A9, INPUT); pinMode(A10, INPUT);
  pinMode(A5, INPUT); pinMode(A6, INPUT);
  
  //Set motors to zero initially
  analogWrite(leftMotor,0); analogWrite(rightMotor,0);
  
  //Setup LEDS
  pinMode(ledGreySplit,OUTPUT);
  pinMode(ledWhiteSplit,OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  /*analogWrite(leftMotor,255);
  analogWrite(rightMotor,255); 
  digitalWrite(hBridgeLeft,HIGH);
  digitalWrite(hBridgeRight,HIGH);
  return;*/
  
  //test for stop
  if(stopTest()){ //Returns True if we need to stop
    analogWrite(leftMotor,0);
    analogWrite(rightMotor,0);
    //state = halt;
    return;
  } 
  
  if(analogRead(A2) > getWhite(A2) && state != normal){
        powerLeft(0); powerRight(0);
        state = normal;
        Serial.println("normal State Set");
  }
  switch(state){
    case CW:
        powerLeft(-50); powerRight(50);
        return;
    break;
    case CCW:
        powerLeft(50); powerRight(-50);
        return;
    break;
  }
  
  //test for sharp turn
  //if A9 and A0-3 lights up we need to right, if A10 and A4-1 lights up, we need to turn left
  if((analogRead(A9) > getWhite(A9)) && (analogRead(A0) > getWhite(A0)|| analogRead(A1) > getWhite(A1)|| analogRead(A2) > getWhite(A2)||analogRead(A3) > getWhite(A3))){
    state = CCW;
    Serial.println("CCW State Set");
    powerLeft(80); powerRight(80);
    delay(250);
  }
  if((analogRead(A10) > getWhite(A10))&& (analogRead(A4) > getWhite(A4)|| analogRead(A3) > getWhite(A3)|| analogRead(A2) > getWhite(A2)||analogRead(A1) > getWhite(A1))){
    state = CW;
    Serial.println("CW State Set");
    powerLeft(80); powerRight(80);
    delay(250);
  }
  
  //test for split
  if(analogRead(A5) > getWhite(A5) && analogRead(A6) > getWhite(A6)){ //If we see white on the two outer sensors
      powerLeft(0); powerLeft(0);
      delay(1000);
      splits++;
    if(analogRead(A2) < getWhite(A2)){ //If we see grey
      digitalWrite(ledGreySplit,HIGH);
      Serial.println("We see a grey split!");
      powerLeft(200); powerRight(0); //TODO: adjust these values
      delay(400);
      digitalWrite(ledGreySplit,LOW);
    } else {
      digitalWrite(ledWhiteSplit,HIGH);
      Serial.println("We see a white split!");
      powerLeft(0); powerRight(200); //TODO: adjust these values
      delay(400);
      digitalWrite(ledWhiteSplit,LOW);
    }
  }

  /* //Tape Follower
  if(analogRead(A1) < getBlack(A1)){
      powerLeft(100); powerRight(200);
  }else if (analogRead(A3) < getBlack(A3)){
      powerLeft(200); powerRight(100);
  } else {
    powerLeft(200); powerRight(200);
  }*/
  
  //Tape Seeking & tape avoiding hybrid
  if(analogRead(A1) > getWhite(A1)){ //If right side of the car see the white tape...
     powerSide = left; //Power the other side
     Serial.println("Left");
  } else if (analogRead(A3) > getWhite(A3)){ //Same for the other side
     powerSide = right;
     Serial.println("Rgiht");
  }
  
  switch(powerSide){
    case left:
      powerLeft(100); powerRight(0); //TODO: adjust these values
      break;
    case right:
      powerLeft(0); powerRight(100); //TODO: adjust these values
    break;
  }
  
  timeCount++;
  if(timeCount >= 1000){
    timeCount = 0;
    for(int i = 0; i < sizeof(sensorPin)/sizeof(byte); i++){
      Serial.print(analogRead(sensorPin[i]));
      Serial.print(" ");
    }
    Serial.print("\n");
    switch (state){
      //enum robotOp { normal, CW, CCW, halt };
      case normal:
        Serial.println("We are normal");
      break;
      case CW:
        Serial.println("We are clockwise");
      break;
      case CCW:
        Serial.println("We are counterclockwise");
      break;
      case halt:
        Serial.println("We are halted");
      break;
    }
    Serial.println("Error score is:");
    Serial.println(errorScore());
  }
}

//Tests
int getBlack(int pin){
  if(pin < 18 || pin > (18 + 11)-1) {
    return -1;
  }
  word blackWord = eeprom_read_word((uint16_t *) ((pin-18)*2));
  word whiteWord = eeprom_read_word((uint16_t *) ((pin-18)*2 + 200));
  word midWord = (blackWord + whiteWord)/2; //TODO: Check that no overflow happens here
  word baseline = (blackWord + midWord)/2;
  return ((int) baseline);
}

int getWhite(int pin){
  if(pin < 18 || pin > (18 + 11)-1) {
    return -1;
  }
  word blackWord = eeprom_read_word((uint16_t *) ((pin-18)*2));
  word whiteWord = eeprom_read_word((uint16_t *) ((pin-18)*2 + 200));
  word midWord = (blackWord + whiteWord)/2; //TODO: Check that no overflow happens here
  word baseline = (whiteWord + midWord)/2;
  return ((int) baseline);
}

boolean stopTest(){
  if(splits >= 2 && (analogRead(A5) > getWhite(A5) && analogRead(A6) > getWhite(A6) && analogRead(A9) > getWhite(A9) && analogRead(A10) > getWhite(A10)))
    return true;
  return false;
}

int errorScore(){
  int winner = 0;
  int winnerVal = 0;
  for(int i = 0; i < sizeof(sensorPin)/sizeof(byte); i++){
    int tempVal = analogRead(sensorPin[i]);
    if(tempVal > winnerVal){
      winner = i;
      winnerVal = tempVal;
    }
  }
  //TODO: Contemplate this
  if(winnerVal < getWhite(sensorPin[winner])){
    return 0;
  }
  switch(winner){
    case 0: 
     return -2;
    case 1:
      return -1;
    case 2:
      return 0;
    case 3:
      return 1;
    case 4:
      return 2;
  }
  return 0;
}

byte turn(int sensor[]){
  boolean firstThreeOn = true;
  for(int i=0; i < 3 && firstThreeOn; i++){
    if(sensor[i]< getWhite(sensorPin[i])){
      firstThreeOn=false;
    }
  }
  if(firstThreeOn)
    return 1;
  
  boolean lastThreeOn = true;
  for(int i=0; i < 3 && lastThreeOn; i++){
    if(sensor[i]< getWhite(sensorPin[i])){
      lastThreeOn=false;
    }
  }
  if(lastThreeOn)
    return -1;
  
  return 0;
}

//Motor macros
void turnCCW(int power){
  if(power > 0){
    digitalWrite(hBridgeLeft,HIGH);
    digitalWrite(hBridgeRight,LOW);
  } else if (power < 0){
    digitalWrite(hBridgeLeft,HIGH);
    digitalWrite(hBridgeRight,LOW);
  }
   analogWrite(rightMotor, power); analogWrite(leftMotor, power);
   return;
}

void powerLeft(int power){
    if (power > 0){
      digitalWrite(hBridgeLeft,HIGH);
    } else if ( power < 0){
      digitalWrite(hBridgeLeft,LOW);
    }
    analogWrite(leftMotor,power);
}

void powerRight(int power){
    if (power > 0){
      digitalWrite(hBridgeRight,HIGH);
    } else if (power < 0){
      digitalWrite(hBridgeRight,LOW);
    }
    analogWrite(rightMotor,power);
}

