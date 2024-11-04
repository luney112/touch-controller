#ifndef AIR_H
#define AIR_H

#include "layout.h"
#include <cstdint>

struct AirSensorData {
  uint16_t values[AIR_SENSOR_COUNT];
};

class AirController {
public:
  void init();
  void calibrate();
  void enableIrSensor(int sensor);
  void get(AirSensorData &data);
  uint8_t getBlockedSensors();

private:
  uint16_t thresholds[AIR_SENSOR_COUNT];
};

#endif
