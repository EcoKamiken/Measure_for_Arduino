#ifndef __INA226_H_INCLUDED__
#define __INA226_H_INCLUDED__

class INA226
{
  public:
    INA226(); 
    int set_i2c_addr(int addr);
    void set_calibration_register();
    float fetch_register(int addr, int byte_len);
    float get_voltage();
    float get_power();
    float get_ampere();
    int measurement();
    
  private:
    int _I2C_ADDR = 0x4a;
    const int _SHUNT_ADDR = 0x01;
    const int _BUS_ADDR = 0x02;
    const int _POWER_ADDR = 0x03;
    const int _CURRENT_ADDR = 0x04;
    const int _CALIBRATION_ADDR = 0x05;
};

#endif
