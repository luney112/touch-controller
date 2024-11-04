#include "air.h"
#include "layout.h"
#include <Arduino.h>

constexpr uint8_t AirSensorPins[AIR_SENSOR_COUNT] = {GPIO_IR_DATA_0, GPIO_IR_DATA_1, GPIO_IR_DATA_2,
                                                     GPIO_IR_DATA_3, GPIO_IR_DATA_4, GPIO_IR_DATA_5};

constexpr int CharlieplexMap[AIR_SENSOR_COUNT][6] = {
    {OUTPUT, OUTPUT, INPUT, HIGH, LOW, LOW}, {OUTPUT, OUTPUT, INPUT, LOW, HIGH, LOW},
    {INPUT, OUTPUT, OUTPUT, LOW, HIGH, LOW}, {INPUT, OUTPUT, OUTPUT, LOW, LOW, HIGH},
    {OUTPUT, INPUT, OUTPUT, HIGH, LOW, LOW}, {OUTPUT, INPUT, OUTPUT, LOW, LOW, HIGH},
};

constexpr int CalibrationCount = 200;
constexpr double ThresholdRatio = 0.8;
constexpr uint32_t IrToggleSettleTimeMicroseconds = 50;
constexpr uint32_t CalibrationIntervalMillis = 10;

void AirController::init() {
  pinMode(GPIO_IR_LED_0, INPUT);
  pinMode(GPIO_IR_LED_1, INPUT);
  pinMode(GPIO_IR_LED_2, INPUT);

  for (int i = 0; i < AIR_SENSOR_COUNT; i++) {
    pinMode(AirSensorPins[i], INPUT);
  }
}

void AirController::calibrate() {
  AirSensorData data;
  unsigned long sum[AIR_SENSOR_COUNT] = {0};

  for (int i = 0; i < CalibrationCount; i++) {
    this->get(data);
    for (int j = 0; j < AIR_SENSOR_COUNT; j++) {
      sum[j] += data.values[j];
    }
    delay(CalibrationIntervalMillis);
  }

  for (int i = 0; i < AIR_SENSOR_COUNT; i++) {
    uint16_t avg = sum[i] / CalibrationCount;
    this->thresholds[i] = avg * ThresholdRatio;
  }
}

void AirController::enableIrSensor(int sensor) {
  if (sensor >= 0 && sensor < AIR_SENSOR_COUNT) {
    pinMode(GPIO_IR_LED_0, CharlieplexMap[sensor][0]);
    pinMode(GPIO_IR_LED_1, CharlieplexMap[sensor][1]);
    pinMode(GPIO_IR_LED_2, CharlieplexMap[sensor][2]);

    digitalWrite(GPIO_IR_LED_0, CharlieplexMap[sensor][3]);
    digitalWrite(GPIO_IR_LED_1, CharlieplexMap[sensor][4]);
    digitalWrite(GPIO_IR_LED_2, CharlieplexMap[sensor][5]);
  } else {
    pinMode(GPIO_IR_LED_0, INPUT);
    pinMode(GPIO_IR_LED_1, INPUT);
    pinMode(GPIO_IR_LED_2, INPUT);
  }
}

void AirController::get(AirSensorData &data) {
  for (int i = 0; i < AIR_SENSOR_COUNT; i++) {
    enableIrSensor(i);
    delayMicroseconds(IrToggleSettleTimeMicroseconds);
    data.values[i] = analogRead(AirSensorPins[i]);
  }
  enableIrSensor(-1);
}

uint8_t AirController::getBlockedSensors() {
  AirSensorData raw;
  this->get(raw);
  uint8_t blocked = 0;
  for (int i = 0; i < AIR_SENSOR_COUNT; i++) {
    blocked |= (raw.values[i] <= this->thresholds[i] ? 1 : 0) << i;
  }

  return blocked;
}
