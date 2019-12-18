#include <math.h>

#include <Wire.h>

class INA226
{
  public:
    int set_i2c_addr(int addr) {
      _I2C_ADDR = addr;
    }

    float fetch_register(int addr, int byte_len) {
      Wire.beginTransmission(_I2C_ADDR);
      Wire.write(addr);
      Wire.endTransmission(false);
      Wire.requestFrom(_I2C_ADDR, byte_len);
      
      if (addr == _BUS_ADDR) {
        int high = Wire.read() << 8;
        int low = Wire.read();
        float lsb = 1.25;
        return (high + low) * lsb * 0.001;
      } else if (addr == _CURRENT_ADDR) {
        float data = Wire.read();
        // Do something.
        return data;
      } else if (addr == _POWER_ADDR) {
        float data = Wire.read();
        // Do something.
        return data; 
      }
    }

    float get_voltage() {
      return fetch_register(_BUS_ADDR, 2);
    }

    float get_power() {
      Wire.beginTransmission(_I2C_ADDR);
      Wire.write(_POWER_ADDR);
      Wire.endTransmission(false);
      Wire.requestFrom(_I2C_ADDR, 2);

      int tmp = Wire.read();

      return tmp;
    }

    float get_ampere() {
      Wire.beginTransmission(_I2C_ADDR);
      Wire.write(_CURRENT_ADDR);
      Wire.endTransmission(false);
      Wire.requestFrom(_I2C_ADDR, 1);

      float tmp = Wire.read();
      return tmp;
    }
    
  private:
    int _I2C_ADDR = 0x4a;
    const int _SHUNT_ADDR = 0x01;
    const int _BUS_ADDR = 0x02;
    const int _POWER_ADDR = 0x03;
    const int _CURRENT_ADDR = 0x04;
    const int _CALIBRATION_ADDR = 0x05;
};

void setup()
{
  Wire.begin();
  Serial.begin(9200);
}

void loop()
{  
  INA226 device;
  float voltage = device.get_voltage();
  float ampere = device.get_ampere();
  float power = device.get_power();
  
  Serial.println(voltage);
  Serial.println(ampere);
  Serial.println(power);
  delay(1000);
}
