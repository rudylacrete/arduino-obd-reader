/* YourDuino.com Example Software Sketch
 16 character 2 line I2C Display
 Backpack Interface labelled "YwRobot Arduino LCM1602 IIC V1"
 terry@yourduino.com */
 
/*-----( Import needed libraries )-----*/
#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>

#include "ObdReader.h"

#define MAX_RPM 8000
#define NB_LCD_COL 16

/*-----( Declare Constants )-----*/
/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
ObdReader reader(2, 3, 4, 5);

void setCustomChar() {
  byte customChar[8];
  byte customCharVoid[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  };
  for(int i = 0; i < 8; i++){
    customChar[i] = B11111;
  }
  lcd.createChar(0, customChar);
  lcd.createChar(1, customCharVoid);
}

void writeRpmValue(int value) {
  int nb_col = (float(value) / MAX_RPM) * NB_LCD_COL;
  for(int i = 0; i < NB_LCD_COL; i++) {
    lcd.setCursor(i,1);
    if(i < nb_col)
      lcd.write(byte(0));
     else
      lcd.write(1);
  }
}

void setup()
{
  Serial.begin(9600);
  // initialize library
  lcd.begin(NB_LCD_COL,2);
  setCustomChar();
  lcd.clear();
  // blink backlight three times
  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight();
  // set cursor to positon x=0, y=0
  lcd.setCursor(0,0);
  // print Hello!
  lcd.print("Hello!");
  // wait a second.
  delay(1000);
  //reader.setup();
} 
 
 
void loop()
{
  Serial.println(analogRead(A0));
  writeRpmValue(map(analogRead(A0), 0, 1024, 0, MAX_RPM));
  /*for(int i = 0; i < MAX_RPM; i += 200) {
    writeRpmValue(i);
    delay(1000);
  }*/
  
}

