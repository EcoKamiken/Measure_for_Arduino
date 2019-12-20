#include <Wire.h>                                                                                                                                                                
#include "INA226.h"
#include "SIGFOX.h"

// Constant
const int SITE_ID = 4095;
const int DEVICE_ID = 15;
const int FORMAT_VERSION = 15;
const int DATA_ID_MAX = 127;
int IS_MEASUREMENT_ERROR = 1;

// Sigfox
static const String device = "NOTUSED";
static const bool useEmulator = false;
static const bool echo = true;
static const Country country = COUNTRY_JP;
static UnaShieldV2S transceiver(country, useEmulator, device, echo);
static String response;

// Variables
int data_id = 0;

void setup()
{
  // Wire.begin();
  Serial.begin(9200);
  // Serial.println(F("Running setup..."));
  // if (!transceiver.begin()) { stop(F("Unable to init SIGFOX module. may be missing")); }
}

void loop()
{  
  INA226 device;

  int wd[12] = {0};
  char send_msg[12] = {0};
  int filter = 0;

  // 0 byte
  filter = 0x00ff;
  // sprintf(&send_msg[0], "%x", SITE_ID & filter);
  wd[0] = SITE_ID & filter;
  // wdをパースしてsend_msg[0],[1]に割り当てるコードを後で書く.

  // 1 byte
  filter = 0x00ff;
  wd[1] = (FORMAT_VERSION << 4) | ((SITE_ID & filter) >> 8);

  // 2 byte
  wd[2] = (IS_MEASUREMENT_ERROR << 7) | data_id;

  // 3 byte
  int average = device.measurement();
  filter = 0x00ff;
  wd[3] = average & filter;

  // 4 byte
  filter = 0x0f00;
  wd[4] = (DEVICE_ID << 4) | (average & filter) >> 8;

  for (int i = 0; i < 5; i++) {
    Serial.println(send_msg[i]);
  }
  
  // transceiver.sendMessage(send_msg);
  
  data_id == DATA_ID_MAX
    ? data_id = 0
    : data_id++;
  delay(100000); // 本番では10秒にする
}
