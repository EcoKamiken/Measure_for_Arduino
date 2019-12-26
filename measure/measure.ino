#include <avr/sleep.h>
#include <Wire.h>     
#include <SoftwareSerial.h>
                                                                                                                                                           
#include "INA226.h"
#include "SIGFOX.h"

SoftwareSerial mySerial(10, 11); // RX, TX

// 電文フォーマットのバージョン
const int FORMAT_VERSION = 0;   // フォーマットバージョン: 0 - 15

// User settings
const int SITE_ID = 15;          // 発電所ID: 0 - 4095
const int DEVICE_ID = 7;        // デバイスID: 0 - 15

// 電文関係
const int MAX_DATA_ID = 127;    // シーケンス番号: 0 - 127
const int MAX_WORD = 6;         // 2バイトで1ワードと定義した時の最大ワード数: 6
const int MAX_SEND_MSG = 12;    // メッセージ長: 12
int IS_MEASUREMENT_ERROR = 0;   // エラーフラグ: 0 - 1

// Sigfox関係
static const String device = "NOTUSED";
static const bool useEmulator = false;
static const bool echo = true;
static const Country country = COUNTRY_JP;
static UnaShieldV2S transceiver(country, useEmulator, device, echo);
static String response;

// シーケンスナンバー
int data_id = 0;

String generate_send_msg(INA226, int, int);

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop()
{  
  INA226 device;
  String s = generate_send_msg(device, 1, 10000); 
  s = "AT$SF=" + s + "\r";
  Serial.println(s);
  mySerial.print(s);
  data_id_counter();
}

void data_id_counter()
{
  data_id == MAX_DATA_ID ? data_id = 0 : data_id++; 
}

String generate_send_msg(INA226 device, int count = 15, int interval = 60000)
{
  static char send_msg[MAX_SEND_MSG + 1];
  int wd[MAX_WORD + 1] = {0};
  int filter = 0;
  
  // 0 byte
  filter = 0x00ff;
  wd[0] = SITE_ID & filter;

  // 1 byte
  filter = 0x00ff;
  wd[1] = (FORMAT_VERSION << 4) | ((SITE_ID & filter) >> 8);

  // 2 byte
  wd[2] = (IS_MEASUREMENT_ERROR << 7) | data_id;

  // 3 byte
  float average = device.get_voltage();
  delay(interval);
  for (int i = 0; i < count; i++) {
    Serial.println(average);
    average = (average + device.get_voltage()) / 2;
    delay(interval);
  }
  
  filter = 0x00ff;
  int avg = (int)average;
  wd[3] = avg & filter;

  // 4 byte
  filter = 0x0f00;
  wd[4] = (DEVICE_ID << 4) | (avg & filter) >> 8;

  sprintf(&send_msg[0], "%x", wd[0] & 0x0f);
  sprintf(&send_msg[1], "%x", wd[0] & 0xf0);
  sprintf(&send_msg[2], "%x", wd[1] & 0x0f);
  sprintf(&send_msg[3], "%x", wd[1] & 0xf0);
  sprintf(&send_msg[4], "%x", wd[2] & 0x0f);
  sprintf(&send_msg[5], "%x", wd[2] & 0xf0);
  sprintf(&send_msg[6], "%x", wd[3] & 0x0f);
  sprintf(&send_msg[7], "%x", wd[3] & 0xf0);
  sprintf(&send_msg[8], "%x", wd[4] & 0x0f);
  sprintf(&send_msg[9], "%x", wd[4] & 0xf0);
  send_msg[10] = '\0';

  String s = send_msg;

  return s;
}
