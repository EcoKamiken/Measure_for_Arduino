#ifndef __INA226_H_INCLUDED__
#define __INA226_H_INCLUDED__

class INA226
{
  public:
    INA226();
    
    // Wrapper
    void write_register(byte, unsigned short);
    short read_register(byte);

    // Setter
    void set_i2c_addr(int);
    void set_config(short);

    // Getter
    int get_i2c_addr();
    int get_config();
    
    float fetch_register(int, int);
    float get_voltage();
    float get_power();
    float get_ampere();
    
  private:
    int _I2C_ADDR = 0x4a;
    const int _CONFIG_ADDR = 0x00;
    const int _SHUNT_ADDR = 0x01;
    const int _BUS_ADDR = 0x02;
    const int _POWER_ADDR = 0x03;
    const int _CURRENT_ADDR = 0x04;
    const int _CALIBRATION_ADDR = 0x05;
};

#endif
