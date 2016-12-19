#ifndef OBD_READER_H
#define OBD_READER_H

#define BAUDRATE 38400
#define BT_CMD_RETRIES 2     //Number of retries for each Bluetooth AT command in case of not responde with OK
#define OBD_CMD_RETRIES 5

#include <SoftwareSerial.h>
#include <ObdReaderConfig.h>

typedef enum {
  AT = 1,
  COM
} mode_t;

class ObdReader{
  public:
    ObdReader(obd_reader_conf_t config): config(config) {};
    void setup();
    int getRpm();
  private:
    mode_t mode;
    SoftwareSerial *serial;
    obd_reader_conf_t config;
    void connectToBluetoothModule();
    void enterComMode();
    void enterATMode();
    void reset();
    bool sendATCommand(const char* command);
    void send_OBD_cmd(const char* obd_cmd);
    void obd_init();
    void getElm327MacAddrFormat(char* dst);
};
#endif
