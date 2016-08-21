#ifndef OBD_READER_H
#define OBD_READER_H

#define BAUDRATE 38400
#define BT_CMD_RETRIES 2     //Number of retries for each Bluetooth AT command in case of not responde with OK
#define OBD_CMD_RETRIES 5

#include <SoftwareSerial.h>

typedef enum {
  AT = 1,
  COM
} mode_t;
  
class ObdReader{
  public:
    ObdReader(unsigned int rxPin, unsigned int txPin, unsigned int powerPin, unsigned int atPin)
    :rxPin(rxPin), txPin(txPin), powerPin(powerPin), atPin(atPin){};
    void setup();
    int getRpm();
  private:
    mode_t mode;
    SoftwareSerial *serial;
    void setupBluetoothModule();
    unsigned int rxPin, txPin, powerPin, atPin;
    void enterComMode();
    void enterATMode();
    void reset();
    boolean sendATCommand(const char* command);
    void send_OBD_cmd(char *obd_cmd);
    void obd_init();
};
#endif

