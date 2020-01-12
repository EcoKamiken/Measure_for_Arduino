#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <Wire.h>     
#include <SoftwareSerial.h>
                                                                                                                                                           
#include "INA226.h"
#include "SIGFOX.h"

SoftwareSerial mySerial(10, 11); // RX, TX

// 電文フォーマットのバージョン
const uint8_t FORMAT_VERSION = 0;   // フォーマットバージョン: 0 - 15
 
// User settings
const uint16_t SITE_ID = 4095;          // 発電所ID: 0 - 4095
const uint8_t DEVICE_ID = 15;        // デバイスID: 0 - 15
const uint8_t INTERVAL = 1;         // 秒
const uint8_t COUNT = 3;            // 回

// 電文関係
const uint8_t MAX_DATA_ID = 127;    // シーケンス番号: 0 - 127
const uint8_t MAX_WORD = 6;         // 2バイトで1ワードと定義した時の最大ワード数: 6
const uint8_t MAX_SEND_MSG = 12;    // メッセージ長: 12
uint8_t IS_MEASUREMENT_ERROR = 0;   // エラーフラグ: 0 - 1

// シーケンスナンバー
uint8_t data_id = 0;

// プロトタイプ宣言
void data_id_counter();
String generate_send_msg(INA226, int, int);
void delayWDT2(unsigned long t);
void delayWDT_setup(unsigned int ii);
void wait_minutes(uint8_t);

// INA226
INA226 device;
const uint8_t normal_mode = device.get_mode(0);
const uint8_t sleep_mode = device.get_mode(1);

void setup()
{
  Serial.println(F("Initialize..."));
  Wire.begin();
  mySerial.begin(9600); 
  Serial.begin(9600);
     
  Serial.print(F("I2C ADDR: "));
  Serial.println(device.get_i2c_addr(), HEX);
  Serial.print(F("CONFIG: "));
  Serial.println(device.get_config(), BIN);                                                                                                                
}

void loop()
{  
  device.set_config(normal_mode);
  Serial.print(F("CONFIG: "));
  Serial.println(device.get_config(), BIN);
  delay(5000);

  float volt = device.get_voltage();
  Serial.println(volt);
  
  device.set_config(sleep_mode);
  Serial.print(F("CONFIG: "));
  Serial.println(device.get_config(), BIN);
  delay(5000);
  
  // solver()
}

void solver() {
  String s = generate_send_msg(device, COUNT, INTERVAL*1000);
  s = "AT$SF=" + s + "\r";
  mySerial.print(s);
  data_id_counter();
}

void data_id_counter()
{
  data_id == MAX_DATA_ID ? data_id = 0 : data_id++; 
}

/*
 * count (int): データ収集する回数
 * interval (int): データ収集の間隔(ミリ秒)
 */
String generate_send_msg(INA226 device, int count, int interval)
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
  Serial.println(F("SLEEP"));
  wait_minutes(1);
  Serial.println(F("WAKEUP"));
  for (int i = 0; i < count; i++) {
    average = (average + device.get_voltage()) / 2;
    Serial.println(F("SLEEP"));
    wait_minutes(1);
    Serial.println(F("WAKEUP"));
  }
  
  filter = 0x00ff;
  int avg = (average * 100.0) / 2; // 4.56[V] -> 456 -> 228
  wd[3] = avg & filter;

  // 4 byte
  wd[4] = DEVICE_ID;

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

void delayWDT2(unsigned long t) {       // パワーダウンモードでdelayを実行
  Serial.flush();                       // シリアルバッファが空になるまで待つ
  delayWDT_setup(t);                    // ウォッチドッグタイマー割り込み条件設定
 
  // ADCを停止（消費電流 147→27μA）
  ADCSRA &= ~(1 << ADEN);
 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // パワーダウンモード指定
  sleep_enable();
 
  // BODを停止（消費電流 27→6.5μA）
  MCUCR |= (1 << BODSE) | (1 << BODS);   // MCUCRのBODSとBODSEに1をセット
  MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);  // すぐに（4クロック以内）BODSSEを0, BODSを1に設定
 
  asm("sleep");                         // 3クロック以内にスリープ sleep_mode();では間に合わなかった
 
  sleep_disable();                      // WDTがタイムアップでここから動作再開
  ADCSRA |= (1 << ADEN);                // ADCの電源をON（BODはハードウエアで自動再開される）
}
 
void delayWDT_setup(unsigned int ii) {  // ウォッチドッグタイマーをセット。
  // 引数はWDTCSRにセットするWDP0-WDP3の値。設定値と動作時間は概略下記
  // 0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
  // 6=1sec, 7=2sec, 8=4sec, 9=8sec
  byte bb;
  if (ii > 9 ) {                        // 変な値を排除
    ii = 9;
  }
  bb = ii & 7;                          // 下位3ビットをbbに
  if (ii > 7) {                         // 7以上（7.8,9）なら
    bb |= (1 << 5);                     // bbの5ビット目(WDP3)を1にする
  }
  bb |= ( 1 << WDCE );
 
  MCUSR &= ~(1 << WDRF);                // MCU Status Reg. Watchdog Reset Flag ->0
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);   // ウォッチドッグ変更許可（WDCEは4サイクルで自動リセット）
  // set new watchdog timeout value
  WDTCSR = bb;                          // 制御レジスタを設定
  WDTCSR |= _BV(WDIE);
}
 
ISR(WDT_vect) {                         // WDTがタイムアップした時に実行される処理
  //  wdt_cycle++;                      // 必要ならコメントアウトを外す
}

void wait_minutes(uint8_t minute) {
  for(uint8_t i = 0; i < minute; i++) {
    for(uint8_t j = 0; j < 7; j++) {
      delayWDT2(9);
    }
    delayWDT2(8);
  }
}
