#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "deepsleep.h"

void delayWDT(unsigned long t) {       // パワーダウンモードでdelayを実行
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
 
ISR(WDT_vect) {}                         // WDTがタイムアップした時に実行される処理

void deep_sleep(uint8_t minute) {
  for(uint8_t i = 0; i < minute; i++) {
    for(uint8_t j = 0; j < 7; j++) {
      delayWDT(9);
    }
    delayWDT(8);
  }
}
