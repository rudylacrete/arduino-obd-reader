#include "ObdReader.h"

#define MAX_RPM 8000
#define NB_LCD_COL 16

ObdReader reader(2, 3, 4, 5);

void setup()
{
  Serial.begin(9600);
  reader.setup();
  Serial.println("setup done ....");
} 
 
 
void loop()
{
  Serial.println(reader.getRpm());
  delay(100);
}

