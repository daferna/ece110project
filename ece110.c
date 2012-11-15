boolean allWhite = true;
int loopNumber = 0;
String stringone = "Analog Pin: ", stringtwo, stringthree;
int leftMotor = 5;
int rightMotor = 6;

void setup() {
  //Setting up left and right motor for PWM output
  
  pinMode(leftMotor, OUTPUT);
  pinMode(rightMotor, OUTPUT);
  
  Serial.begin(9600);          //  setup serial
}

void loop() {
  //If we see all white, stop motors
  allWhite = true;
  //Go through each sensor, and check if it's higher than 512
  //If a single sensor is not higher than 512, we don't need to keep checking
  for(int i = 0; (i < 6) && allWhite; i++){
    int sensorValue = analogRead(i);
    if(allWhite && sensorValue > 800){
      allWhite = true;
      loopNumber++;
    }
    else{
      allWhite = false;
      loopNumber = 0;
    }
    stringtwo = stringone + i + " ";
    stringthree = stringtwo + sensorValue;
    Serial.println(stringthree);
  }
  if(allWhite && loopNumber >= 5){
    Serial.println("We see white!");
    loopNumber = 0;
    //Tell motors to stop
    analogWrite(leftMotor,0);
    analogWrite(rightMotor,0);
  }
  delay(500);
}
