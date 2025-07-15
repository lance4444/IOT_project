/*
项目名称：SGP30空气质量与人流量ESP32发送端程序
接线定义:
  SGP30: VCC--3.3V, GND--GND, SCL--GPIO 26, SDA--GPIO 27
  UART2: RX--GPIO 16, TX--GPIO 17 (发送到接收端)
  LED_PIN: GPIO 25
  左超声波: Trig--GPIO 18, Echo--GPIO 19
  右超声波: Trig--GPIO 23, Echo--GPIO 5
*/

#include "SGP30.h"

SGP mySGP30;
uint16_t CO2Data, TVOCData, prevCO2Data = 0, prevTVOCData = 0;
uint32_t sgp30_dat;
unsigned long lastUpdateTime = 0;
const int updateInterval = 10000; // 10秒监测间隔

#define LED_PIN 25
#define TRIG_PIN_LEFT 18
#define ECHO_PIN_LEFT 19
#define TRIG_PIN_RIGHT 23
#define ECHO_PIN_RIGHT 5

int peopleCount = 0;
bool leftTriggered = false;
bool rightTriggered = false;  // 新增：右传感器触发状态
const int distanceThreshold = 30; // 距离阈值30cm
unsigned long lastTriggerTime = 0;
const int debounceDelay = 1000; // 防抖延迟1秒
const int timeoutDelay = 5000; // 触发后5秒超时

long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Setup: Initialized");

  pinMode(TRIG_PIN_LEFT, OUTPUT);
  pinMode(ECHO_PIN_LEFT, INPUT);
  pinMode(TRIG_PIN_RIGHT, OUTPUT);
  pinMode(ECHO_PIN_RIGHT, INPUT);

  mySGP30.SGP30_Init();
  mySGP30.SGP30_Write(0x20, 0x08);
  sgp30_dat = mySGP30.SGP30_Read();
  CO2Data = (sgp30_dat & 0xffff0000) >> 16;
  TVOCData = sgp30_dat & 0x0000ffff;
  prevCO2Data = CO2Data;
  prevTVOCData = TVOCData;
  while (CO2Data == 400 && TVOCData == 0) {
    mySGP30.SGP30_Write(0x20, 0x08);
    sgp30_dat = mySGP30.SGP30_Read();
    CO2Data = (sgp30_dat & 0xffff0000) >> 16;
    TVOCData = sgp30_dat & 0x0000ffff;
    delay(500);
  }
}

void loop() {
  unsigned long currentTime = millis();

  // 每10秒监测空气数据
  if (currentTime - lastUpdateTime >= updateInterval) {
    mySGP30.SGP30_Write(0x20, 0x08);
    sgp30_dat = mySGP30.SGP30_Read();
    CO2Data = (sgp30_dat & 0xffff0000) >> 16;
    TVOCData = sgp30_dat & 0x0000ffff;
    Serial.print("Loop: SGP30 - CO2: ");
    Serial.print(CO2Data);
    Serial.print(" ppm, TVOC: ");
    Serial.println(TVOCData);

    if (CO2Data != prevCO2Data || TVOCData != prevTVOCData) {
      String airData = String(CO2Data) + "," + String(TVOCData);
      Serial2.println(airData);
      Serial.print("Loop: Sent air data: ");
      Serial.println(airData);
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      prevCO2Data = CO2Data;
      prevTVOCData = TVOCData;
    }
    lastUpdateTime = currentTime;
  }

  long leftDistance = readUltrasonic(TRIG_PIN_LEFT, ECHO_PIN_LEFT);
  long rightDistance = readUltrasonic(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);

  if (leftDistance >= 0 && rightDistance >= 0) {
    // 从左到右检测（进入，人数+1）
    if (leftDistance < distanceThreshold && !leftTriggered && !rightTriggered && rightDistance > distanceThreshold) {
      leftTriggered = true;
      lastTriggerTime = currentTime;
      Serial.println("Loop: Left sensor triggered (entering)");
    } 
    else if (leftTriggered && leftDistance > distanceThreshold && rightDistance < distanceThreshold && (currentTime - lastTriggerTime >= debounceDelay)) {
      peopleCount++;
      String peopleData = "PeopleCount:" + String(peopleCount);
      Serial2.println(peopleData);
      Serial.print("Loop: Person entered - Sent people count: [");
      Serial.print(peopleData);
      Serial.println("]");
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(100);
      lastTriggerTime = currentTime;
      leftTriggered = false;
    }
    
    // 从右到左检测（离开，人数-1）
    else if (rightDistance < distanceThreshold && !rightTriggered && !leftTriggered && leftDistance > distanceThreshold) {
      rightTriggered = true;
      lastTriggerTime = currentTime;
      Serial.println("Loop: Right sensor triggered (exiting)");
    }
    else if (rightTriggered && rightDistance > distanceThreshold && leftDistance < distanceThreshold && (currentTime - lastTriggerTime >= debounceDelay)) {
      if (peopleCount > 0) {  // 确保人数不会小于0
        peopleCount--;
      }
      String peopleData = "PeopleCount:" + String(peopleCount);
      Serial2.println(peopleData);
      Serial.print("Loop: Person exited - Sent people count: [");
      Serial.print(peopleData);
      Serial.println("]");
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(100);
      lastTriggerTime = currentTime;
      rightTriggered = false;
    }
    
    // 超时重置
    else if ((leftTriggered || rightTriggered) && (currentTime - lastTriggerTime >= timeoutDelay)) {
      if (leftTriggered) {
        Serial.println("Loop: Left trigger timed out");
      }
      if (rightTriggered) {
        Serial.println("Loop: Right trigger timed out");
      }
      leftTriggered = false;
      rightTriggered = false;
    }
  }

  delay(100);
}