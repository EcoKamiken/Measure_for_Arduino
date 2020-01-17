#ifndef __INA226_H_INCLUDED__
#define __INA226_H_INCLUDED__

class INA226
{
  public:
    // Wrapper
    void write_register(byte, unsigned short);
    short read_register(byte);

    // Setter
    void set_i2c_addr(int);
    void set_config(short);

    // Getter
    int get_i2c_addr();
    int get_config();
    uint16_t get_mode(uint16_t);
    float get_voltage();
    float get_ampere();
    float get_power();
    
  private:
    uint8_t _I2C_ADDR = 0x4a;
    const uint8_t _CONFIG_ADDR = 0x00;
    const uint8_t _SHUNT_ADDR = 0x01;
    const uint8_t _BUS_ADDR = 0x02;
    const uint8_t _POWER_ADDR = 0x03;
    const uint8_t _CURRENT_ADDR = 0x04;
    const uint8_t _CALIBRATION_ADDR = 0x05;

    const uint16_t _NORMAL_MODE = 0b0110000100100111;
    const uint16_t _SLEEP_MODE = 0b0110000100100000;
};

#endif
