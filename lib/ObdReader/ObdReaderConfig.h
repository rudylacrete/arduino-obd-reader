#ifndef _OBD_READER_CONFIG_H
#define _OBD_READER_CONFIG_H

typedef struct {
  unsigned int rxPin;
  unsigned int txPin;
  unsigned int powerPin;
  unsigned int atPin;
  const char* slave_mac_addr;
  const char* password;
} obd_reader_conf_t;

#endif
