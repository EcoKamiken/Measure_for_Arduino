#include <Wire.h>

const int DEVICE_ADDR = 0x4a;
const int VOLT_REG_ADDR = 0x02;
float avg;

void setup()
{
  avg = 0;
  
  Wire.begin();
  Serial.begin(9600);
  while(!Serial);
}

float get_voltage()
{
  Wire.beginTransmission(DEVICE_ADDR);
  Wire.write(VOLT_REG_ADDR);
  Wire.endTransmission(false);
  Wire.requestFrom(DEVICE_ADDR, 2);
  
  int high = Wire.read() << 8;
  int low = Wire.read();
  float lsb = 1.25;
  return (high + low) * lsb * 0.001;
}

void loop()
{
  if(!avg) {
    Serial.println("zero");
    avg = get_voltage();
  } else {
    Serial.println("o");
    avg = (avg + get_voltage()) / 2;
  }
  
  Serial.println(avg);
  delay(1000);
}
