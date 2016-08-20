/*
This source code is inspired from the following:
http://www.kokoras.com/OBD/Arduino_HC-05_ELM327_OBD_RPM_Shift_Light.htm
*/
#include <inttypes.h>
#include <Arduino.h>
#include "ObdReader.h"

void ObdReader::setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(atPin, OUTPUT);
  pinMode(powerPin, OUTPUT);

  digitalWrite(atPin, LOW);
  digitalWrite(powerPin, HIGH);

  serial = new SoftwareSerial(rxPin, txPin);
  serial->begin(BAUDRATE);
  setupBluetoothModule();
  obd_init();
}

void ObdReader::setupBluetoothModule() {
  enterATMode();                          //enter HC-05 AT mode
  delay(500);

  sendATCommand("RESET");                  //send to HC-05 RESET
  delay(1000);
  sendATCommand("ORGL");                   //send ORGL, reset to original properties
  sendATCommand("ROLE=1");                 //send ROLE=1, set role to master
  sendATCommand("CMODE=0");                //send CMODE=0, set connection mode to specific address
  sendATCommand("PSWD=1234");
  sendATCommand("BIND=1122,33,DDEEFF");    //send BIND=??, bind HC-05 to OBD bluetooth address
  sendATCommand("INIT");                   //send INIT, cant connect without this cmd 
  delay(1000); 
  sendATCommand("PAIR=1122,33,DDEEFF,20"); //send PAIR, pair with OBD address
  delay(1000);  
  sendATCommand("LINK=1122,33,DDEEFF");    //send LINK, link with OBD address
  delay(1000); 
  enterComMode();                          //enter HC-05 comunication mode
  delay(500);
}

boolean ObdReader::sendATCommand(const char* command) {
  char recvChar;
  char str[30] = {'\0'};
  char buf[40] = {'\0'};
  int i = 0, retries = 0;
  boolean OK_flag = false;

  sprintf(buf, "Sending AT command ... %s", command);
  Serial.println(buf);
  while ((retries < BT_CMD_RETRIES) && (!OK_flag)) {     //while not OK and bluetooth cmd retries not reached
      
     serial->print("AT");                       //sent AT cmd to HC-05
     if(strlen(command) > 1){
       serial->print("+");
       serial->print(command);
     }
     serial->print("\r\n");
    
    while (serial->available() <= 0);              //wait while no data
    
    while (serial->available() > 0){               // while data is available
      recvChar = serial->read();                 //read data from HC-05
      str[i] = recvChar;                               //put received char to str
      i++;
    }
    retries = retries + 1;                                  //increase retries
    
    OK_flag = (str[0] == 'O' && str[1] == 'K');   //if response is OK then OK-flag set to true
    sprintf(buf, "Reply: %s |||| %d", str, OK_flag);
    Serial.println(buf);
    delay(1000);
  }

  return OK_flag;
}

void ObdReader::send_OBD_cmd(char *obd_cmd) {
  char recvChar;
  boolean prompt;
  int retries;
    
  prompt = false;
  retries = 0;
  while((!prompt) && (retries < OBD_CMD_RETRIES)) {                //while no prompt and not reached OBD cmd retries
    serial->print(obd_cmd);                             //send OBD cmd
    serial->print("\r");                                //send cariage return

    while (serial->available() <= 0);                   //wait while no data from ELM
    
    while ((serial->available()>0) && (!prompt)){       //while there is data and not prompt
      recvChar = serial->read();                        //read from elm
      if (recvChar == 62) prompt=true;                            //if received char is '>' then prompt is true
    }
    retries++;                                          //increase retries
    delay(2000);
  }
}

void ObdReader::obd_init() {
  send_OBD_cmd("ATZ");      //send to OBD ATZ, reset
  delay(1000);
  send_OBD_cmd("ATSP0");    //send ATSP0, protocol auto
  
  send_OBD_cmd("0100");     //send 0100, retrieve available pid's 00-19
  delay(1000);
  send_OBD_cmd("0120");     //send 0120, retrieve available pid's 20-39
  delay(1000);
  send_OBD_cmd("0140");     //send 0140, retrieve available pid's 40-??
  delay(1000);
  send_OBD_cmd("010C1");    //send 010C1, RPM cmd
  delay(1000);
}

int ObdReader::getRpm() {
  boolean prompt = false, valid = false;  
  char recvChar;
  char bufin[15];
  int i = 0, rpm = 0;

   serial->print("010C1");                        //send to OBD PID command 010C is for RPM, the last 1 is for ELM to wait just for 1 respond (see ELM datasheet)
   serial->print("\r");                           //send to OBD cariage return char
   while (serial->available() <= 0);              //wait while no data from ELM

   while ((serial->available()>0) && (!prompt)){  //if there is data from ELM and prompt is false
     recvChar = serial->read();                   //read from ELM
     if ((i < 15) && (!(recvChar == 32))) {                     //the normal respond to previus command is 010C1/r41 0C ?? ??>, so count 15 chars and ignore char 32 which is space
       bufin[i] = recvChar;                                 //put received char in bufin array
       i++;                                             //increase i
     }  
     if (recvChar == 62) prompt = true;                       //if received char is 62 which is '>' then prompt is true, which means that ELM response is finished 
   }
  
  valid = ((bufin[6] == '4') && (bufin[7] == '1') && (bufin[8] == '0') && (bufin[9] == 'C')); //if first four chars after our command is 410C
  if (valid){                                                                    //in case of correct RPM response
    
   //start calculation of real RPM value
   //RPM is coming from OBD in two 8bit(bytes) hex numbers for example A=0B and B=6C
   //the equation is ((A * 256) + B) / 4, so 0B=11 and 6C=108
   //so rpm=((11 * 256) + 108) / 4 = 731 a normal idle car engine rpm                                                                                           
    for (i = 10; i < 14; i++) {                              //in that 4 chars of bufin array which is the RPM value
      if ((bufin[i] >= 'A') && (bufin[i] <= 'F')){        //if char is between 'A' and 'F'
        bufin[i] -= 55;                                 //'A' is int 65 minus 55 gives 10 which is int value for hex A
      } 
       
      if ((bufin[i] >= '0') && (bufin[i] <= '9')){        //if char is between '0' and '9'
        bufin[i] -= 48;                                 //'0' is int 48 minus 48 gives 0 same as hex
      }
      
      rpm = (rpm << 4) | (bufin[i] & 0xf);              //shift left rpm 4 bits and add the 4 bits of new char
     
    }
    rpm = rpm >> 2;                                     //finaly shift right rpm 2 bits, rpm=rpm/4
  }
  return rpm;
}

void ObdReader::enterComMode() {
  Serial.println("Entering COM mode ...");
  serial->flush();
  delay(500);
  digitalWrite(atPin, LOW);
  delay(500);
  serial->begin(BAUDRATE);
  mode = COM;
}

void ObdReader::enterATMode() {
  Serial.println("Entering AT mode ...");
  serial->flush();
  delay(500);
  digitalWrite(atPin, HIGH);
  delay(500);
  serial->begin(BAUDRATE);
  mode = AT;
}

void ObdReader::reset() {
  digitalWrite(powerPin, LOW);
  delay(2000);
  digitalWrite(powerPin, HIGH);
  mode = COM;
}

