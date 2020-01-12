#include <Arduino.h>
#include <Wire.h>
#include "INA226.h"

INA226::INA226() {
  // constructor
}

/* -------------------------------------------
 * Wrapper
 * -------------------------------------------*/
 
void INA226::write_register(byte reg, unsigned short value) {
  Wire.beginTransmission(_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value >> 8);
  Wire.write(value & 0xff);
  Wire.endTransmission();
}

short INA226::read_register(byte reg) {
  short rest = 0;
  Wire.beginTransmission(_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom((int)_I2C_ADDR, 2);
  while (Wire.available()) {
    rest = (rest << 8) | Wire.read();
  }

  return rest;
}

/* -------------------------------------------
 * Setter
 * -------------------------------------------*/

void INA226::set_i2c_addr(int addr) {
  _I2C_ADDR = addr;
}


void INA226::set_config(short value) {
  write_register(_CONFIG_ADDR, value);
}

/* -------------------------------------------
 * Getter
 * -------------------------------------------*/

int INA226::get_config() {
  return read_register(_CONFIG_ADDR);
}

int INA226::get_i2c_addr() {
  return _I2C_ADDR;
}

uint16_t INA226::get_mode(uint16_t mode) {
  switch (mode) {
    case 0:
      return _NORMAL_MODE;
      break;
    case 1:
      return _SLEEP_MODE;
      break;
  }
}

float INA226::get_voltage() {
  float lsb = 1.25;
  return read_register(_BUS_ADDR) * lsb * 0.001; // [V]
}

float INA226::get_ampere() {
  float lsb = 0.1;
  return read_register(_SHUNT_ADDR) * lsb * 200; // [A]
}

float INA226::get_power() {
  float lsb = 1;
  return read_register(_POWER_ADDR) * lsb * 0.001; // [W]
}
