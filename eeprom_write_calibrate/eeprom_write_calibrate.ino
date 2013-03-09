/*
 * EEPROM Write
 *
 * Stores values read from analog input 0 into the EEPROM.
 * These values will stay in the EEPROM when the board is
 * turned off and may be retrieved later by another sketch.
 */

#include <EEPROM.h>
#include <avr/eeprom.h>

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;
//TODO: fix this
byte sensorPin[] = {A0,A1,A2,A3,A4,A9,A10};
int sensorIndex = 0;
enum configState { prompt, pinNum, colorVal, printOutVal, writeOutVal };
enum color {black, grey, white};
configState state = prompt;
color colorState;
void setup()
{
  for(int i = 0; i < (sizeof(sensorPin)/sizeof(byte)); i++){
    pinMode(sensorPin[i], INPUT);
  }
  Serial.begin(9600);
  delay(2000);
}

void loop()
{
  switch(state){
    case prompt:
      Serial.flush();
      Serial.println("What sensor index?");
      state = pinNum;
    break;
    
    case pinNum:
      if(Serial.available() > 0){
        sensorIndex = Serial.parseInt();
        Serial.flush();
        Serial.println("What are we calibrating for? (b,g,w)");
        state = colorVal;
      }
    break;
    
    case colorVal:
      if(Serial.available() > 0){
        byte colorLetter = Serial.read();
        switch(colorLetter){
          case 'b':
            colorState = black;
          break;
          
          case 'g':
            colorState = grey;
          break;
          
          case 'w':
            colorState = white;
          break;
        }
        Serial.flush();
        state = printOutVal;
        Serial.println("Your Color is");
        Serial.println((int) colorState);
        Serial.println("and your pin is");
        Serial.println(sensorIndex);
      }
    break;
      
    case printOutVal:
      word value = analogRead(sensorIndex+18);
      Serial.println(value);
      //If we recieve 'k', write the value out
      if(Serial.available()>0){
        byte incomingByte;
        incomingByte = Serial.read();
        if(incomingByte == 'k'){
          //Alright, we are now ready to store the sensor value
          switch(colorState){
            case black:
              eeprom_write_word((uint16_t*) (2*sensorIndex),value);
              break;
            case grey:
              eeprom_write_word((uint16_t*) ((2*sensorIndex)+100),value);
              break;
            case white:
              eeprom_write_word((uint16_t*) ((2*sensorIndex)+200),value);
              break;
          }
          Serial.println("The Value Stored was");
          Serial.println(value);
          Serial.println("At Location:");
          Serial.println((2*sensorIndex)+200);
          Serial.read();
          state = prompt;
        } 
      }
     break;
  }
  delay(100);
  return;
  // need to divide by 4 because analog inputs range from
  // 0 to 1023 and each byte of the EEPROM can only hold a
  // value from 0 to 255.
  int val = analogRead(0) / 4;
  
  // write the value to the appropriate byte of the EEPROM.
  // these values will remain there when the board is
  // turned off.
  EEPROM.write(addr, val);
  
  // advance to the next address.  there are 512 bytes in 
  // the EEPROM, so go back to 0 when we hit 512.
  addr = addr + 1;
  if (addr == 512)
    addr = 0;
  
  delay(100);
}
