/*
This source code is inspired from the following:
http://www.kokoras.com/OBD/Arduino_HC-05_ELM327_OBD_RPM_Shift_Light.htm
*/
#include <inttypes.h>
#include <Arduino.h>
#include "ObdReader.h"

void ObdReader::setup() {
  pinMode(config.rxPin, INPUT);
  pinMode(config.txPin, OUTPUT);
  pinMode(config.atPin, OUTPUT);
  pinMode(config.powerPin, OUTPUT);

  digitalWrite(config.atPin, LOW);
  digitalWrite(config.powerPin, HIGH);

  serial = new SoftwareSerial(config.rxPin, config.txPin);
  serial->begin(BAUDRATE);
  connectToBluetoothModule();
  obd_init();
}

void ObdReader::connectToBluetoothModule() {
  char _mac[15] = {'\0'};
  char linkCmd[20] = "LINK=";
  char bindCmd[20] = "BIND=";
  char pairCmd[23] = "PAIR=";

  getElm327MacAddrFormat(_mac);
  strcat(pairCmd, _mac);
  strcat(pairCmd, ",20");
  strcat(bindCmd, _mac);
  strcat(linkCmd, _mac);

  enterATMode();                          //enter HC-05 AT mode
  delay(500);

  sendATCommand("RESET");                  //send to HC-05 RESET
  delay(500);
  sendATCommand("ORGL");                   //send ORGL, reset to original properties
  sendATCommand("ROLE=1");                 //send ROLE=1, set role to master
  sendATCommand("CMODE=0");                //send CMODE=0, set connection mode to specific address
  if(strlen(config.password) > 0) {
    char pwdCmd[10] = {'\0'};
    snprintf(pwdCmd, sizeof(pwdCmd), "PSWD=%s", config.password);
    sendATCommand(pwdCmd);
  }
  sendATCommand(bindCmd);    //send BIND=??, bind HC-05 to OBD bluetooth address
  sendATCommand("INIT");                   //send INIT, cant connect without this cmd
  delay(500);
  sendATCommand(pairCmd); //send PAIR, pair with OBD address
  delay(500);
  sendATCommand(linkCmd);    //send LINK, link with OBD address
  delay(500);
  enterComMode();                          //enter HC-05 comunication mode
  delay(500);
}

bool ObdReader::sendATCommand(const char* command) {
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
    delay(500);
  }

  return OK_flag;
}

void ObdReader::send_OBD_cmd(const char* obd_cmd) {
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
  delay(500);
  send_OBD_cmd("ATSP0");    //send ATSP0, protocol auto

  send_OBD_cmd("0100");     //send 0100, retrieve available pid's 00-19
  delay(500);
  send_OBD_cmd("0120");     //send 0120, retrieve available pid's 20-39
  delay(500);
  send_OBD_cmd("0140");     //send 0140, retrieve available pid's 40-??
  delay(500);
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
  digitalWrite(config.atPin, LOW);
  delay(500);
  serial->begin(BAUDRATE);
  mode = COM;
}

void ObdReader::enterATMode() {
  Serial.println("Entering AT mode ...");
  serial->flush();
  delay(500);
  digitalWrite(config.atPin, HIGH);
  delay(500);
  serial->begin(BAUDRATE);
  mode = AT;
}

void ObdReader::reset() {
  digitalWrite(config.powerPin, LOW);
  delay(2000);
  digitalWrite(config.powerPin, HIGH);
  mode = COM;
}

void ObdReader::getElm327MacAddrFormat(char* dst) {
  // source format is : FF:FF:FF:FF:FF:FF
  // expected format is : FFFF,FF,FFFFFF
  const char* mac_addr = config.slave_mac_addr;
  memcpy(dst, mac_addr, 2);
  memcpy(dst + 2, mac_addr + 3, 2);
  dst[4] = ',';
  memcpy(dst + 5, mac_addr + 6, 2);
  dst[7] = ',';
  memcpy(dst + 8, mac_addr + 9, 2);
  memcpy(dst + 10, mac_addr + 12, 2);
  memcpy(dst + 12, mac_addr + 15, 2);
}
