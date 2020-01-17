#include <Wire.h>
#include <SoftwareSerial.h>

#include "INA226.h"
#include "deepsleep.h"

SoftwareSerial mySerial(10, 11); // RX, TX

// 電文フォーマットのバージョン
const uint8_t FORMAT_VERSION = 0; // フォーマットバージョン: 0 - 15

// User settings
const uint16_t SITE_ID = 0;  // 発電所ID: 0 - 4095
const uint8_t DEVICE_ID = 0; // デバイスID: 0 - 15

// 電文関係
const uint8_t MAX_WORD = 12;
const uint8_t MAX_MSG = 12; // メッセージ長: 12

// INA226
INA226 device;
const uint8_t normal_mode = device.get_mode(0); // Normal
const uint8_t sleep_mode = device.get_mode(1);  // Sleep

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
}

void loop()
{
  solver();
}

void solver()
{
  // INA226 データ取得
  device.set_config(normal_mode);
  delay(1000); // レジスタのリフレッシュ待ち
  float voltage = device.get_voltage();
  device.set_config(sleep_mode);

  // SigFoxメッセージ作成&送信
  String buf = create_sigfox_msg(voltage);
  String msg = "AT$SF=" + buf + "\r";
  digitalWrite(12, LOW);
  digitalWrite(12, HIGH);
  //mySerial.print(msg);
  mySerial.print("AT$P=2\r");

  Serial.println(msg);

  // 指定時間Sleep
  // deep_sleep(1);
}

String create_sigfox_msg(float voltage)
{
  uint8_t wd[MAX_WORD + 1] = {0};
  uint16_t v = voltage * 100;

  wd[0] = SITE_ID & 0x00ff;
  wd[1] = (FORMAT_VERSION << 4) | (SITE_ID & 0x0f00) >> 8;
  wd[2] = v & 0x00ff;
  wd[3] = (DEVICE_ID << 4) | (v & 0x0f00) >> 8;

  char buf[MAX_MSG + 1];
  sprintf(&buf[0], "%x", wd[0] & 0xf0);
  sprintf(&buf[1], "%x", wd[0] & 0x0f);
  sprintf(&buf[2], "%x", wd[1] & 0xf0);
  sprintf(&buf[3], "%x", wd[1] & 0x0f);
  sprintf(&buf[4], "%x", wd[2] & 0xf0);
  sprintf(&buf[5], "%x", wd[2] & 0x0f);
  sprintf(&buf[6], "%x", wd[3] & 0xf0);
  sprintf(&buf[7], "%x", wd[3] & 0x0f);
  buf[8] = '\0';

  String msg = buf;
  return msg;
}
