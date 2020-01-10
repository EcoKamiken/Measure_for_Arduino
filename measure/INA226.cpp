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

float INA226::fetch_register(int addr, int byte_len) {
  Wire.beginTransmission(_I2C_ADDR);
  Wire.write(addr);
  Wire.endTransmission(false);
  Wire.requestFrom(_I2C_ADDR, byte_len);
 
  if (addr == _BUS_ADDR) {
    int high = Wire.read() << 8;
    int low = Wire.read();
    float lsb = 1.25;
    return (high + low) * lsb * 0.001; // [V]
  } else if (addr == _SHUNT_ADDR) {
    int high = Wire.read() << 8;
    int low = Wire.read();
    float lsb = 0.1;
    return (high + low) * lsb * 200; // [A]
  } else if (addr == _POWER_ADDR) {
    int high = Wire.read() << 8;
    int low = Wire.read();
    float lsb = 1;
    return (high + low) * lsb * 0.001; // [W]
  }
}

float INA226::get_voltage() {
  return fetch_register(_BUS_ADDR, 2);
}

float INA226::get_power() {
  return fetch_register(_POWER_ADDR, 2);
}

float INA226::get_ampere() {
  return fetch_register(_SHUNT_ADDR, 2);
}
