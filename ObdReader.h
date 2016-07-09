#ifndef OBD_READER_H
#define OBD_READER_H

#define BAUDRATE 38400
#define BT_CMD_RETRIES 5     //Number of retries for each Bluetooth AT command in case of not responde with OK
#define OBD_CMD_RETRIES 5

#include <SoftwareSerial.h>

typedef enum {
  AT = 1,
  COM
} mode_t;
  
class ObdReader{
  public:
    ObdReader(unsigned int rxPin, unsigned int txPin, unsigned int resetPin, unsigned int atPin)
    :rxPin(rxPin), txPin(txPin), resetPin(resetPin), atPin(atPin){};
    void setup();
    int getRpm();
  private:
    mode_t mode;
    SoftwareSerial *serial;
    void setupBluetoothModule();
    unsigned int rxPin, txPin, resetPin, atPin;
    void enterComMode();
    void enterATMode();
    void reset();
    boolean sendATCommand(const char* command);
    void send_OBD_cmd(char *obd_cmd);
    void obd_init();
};
#endif
