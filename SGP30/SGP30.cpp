#include "SGP30.h"
#include <Arduino.h>

void I2CDelay(uint8_t t) {
  while (t--);
}

void SGP::SGP30_IO_Init() {
  pinMode(SCL_PIN, OUTPUT);
  pinMode(SDA_PIN, OUTPUT);
}

void SGP::I2CStart(void) {
  pinMode(SDA_PIN, OUTPUT);
  SDA_1;
  SCL_1;
  I2CDelay(50);
  SDA_0;
  I2CDelay(50);
  SCL_0;
  I2CDelay(50);
}

void SGP::I2CStop(void) {
  pinMode(SDA_PIN, OUTPUT);
  SDA_0;
  SCL_0;
  I2CDelay(50);
  SCL_1;
  I2CDelay(50);
  SDA_1;
  I2CDelay(50);
}

uint8_t SGP::I2C_Write_Byte(uint8_t Write_Byte) {
  uint8_t i;
  pinMode(SDA_PIN, OUTPUT);
  SCL_0;
  I2CDelay(10);
  for (i = 0; i < 8; i++) {
    if (Write_Byte & 0x80) SDA_1;
    else SDA_0;
    I2CDelay(5);
    SCL_1;
    I2CDelay(5);
    SCL_0;
    I2CDelay(5);
    Write_Byte <<= 1;
  }
  I2CDelay(1);
  SDA_1;
  I2CDelay(40);
  SCL_1;
  I2CDelay(40);
  pinMode(SDA_PIN, INPUT);
  if (digitalRead(SDA_PIN) == 1) {
    SCL_0;
    return 1; // NACK
  } else {
    SCL_0;
    return 0; // ACK
  }
}

uint8_t SGP::I2C_Read_Byte(uint8_t AckValue) {
  uint8_t i, RDByte = 0;
  pinMode(SDA_PIN, OUTPUT);
  SCL_0;
  I2CDelay(40);
  SDA_1;
  pinMode(SDA_PIN, INPUT);
  for (i = 0; i < 8; i++) {
    SCL_1;
    I2CDelay(20);
    RDByte <<= 1;
    if (digitalRead(SDA_PIN) == 1) RDByte |= 0x01;
    else RDByte &= 0xfe;
    I2CDelay(10);
    SCL_0;
    I2CDelay(60);
  }
  pinMode(SDA_PIN, OUTPUT);
  digitalWrite(SDA_PIN, AckValue);
  I2CDelay(30);
  SCL_1;
  I2CDelay(50);
  SCL_0;
  I2CDelay(150);
  return RDByte;
}

void SGP::SGP30_Init(void) {
  SGP30_IO_Init();
  SGP30_Write(0x20, 0x03); // 初始化命令
  delay(15); // 等待15ms
}

void SGP::SGP30_Write(uint8_t a, uint8_t b) {
  I2CStart();
  I2C_Write_Byte(0xB0); // SGP30 写地址
  I2C_Write_Byte(a);
  I2C_Write_Byte(b);
  I2CStop();
  delay(100);
}

unsigned long SGP::SGP30_Read(void) {
  unsigned long dat = 0;
  I2CStart();
  I2C_Write_Byte(0xB1); // SGP30 读地址
  dat = I2C_Read_Byte(0); // ACK
  dat <<= 8;
  dat += I2C_Read_Byte(0); // ACK
  dat <<= 8;
  dat += I2C_Read_Byte(0); // ACK
  dat <<= 8;
  dat += I2C_Read_Byte(1); // NACK
  I2CStop();
  return dat;
}