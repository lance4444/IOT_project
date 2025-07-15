#ifndef SGP30_H
#define SGP30_H

#include <Arduino.h> // 包含Arduino库

#define SDA_PIN 27
#define SCL_PIN 26
#define SDA_1 digitalWrite(SDA_PIN, HIGH)
#define SDA_0 digitalWrite(SDA_PIN, LOW)
#define SCL_1 digitalWrite(SCL_PIN, HIGH)
#define SCL_0 digitalWrite(SCL_PIN, LOW)

class SGP {
public:
  void SGP30_IO_Init();
  void I2CStart(void);
  void I2CStop(void);
  uint8_t I2C_Write_Byte(uint8_t Write_Byte);
  uint8_t I2C_Read_Byte(uint8_t AckValue);
  void SGP30_Init(void);
  void SGP30_Write(uint8_t a, uint8_t b);
  unsigned long SGP30_Read(void);
};

#endif