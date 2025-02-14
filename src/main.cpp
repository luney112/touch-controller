#include <Arduino.h>

#include "air.h"
#include "led.h"
#include "serial.h"
#include "touch.h"

// #define TEST_MODE
#define REPORT_LATENCY_METRICS

void processSensorData();
void processAirSensorData();
void processSliderData();
void processSensorDataTest();

constexpr int SensorReadFrequencyMicros = 1000;
constexpr int SliderTouchedPressureValue = 128;

constexpr int LatencyMetricSampleCount = 1000;

LedController led;
TouchController touch;
AirController air;
SerialController serial;

TouchData touchData;
AirSensorData airSensorData;
int ledData[LED_KEY_COUNT] = {0};

unsigned long startTime = 0;

void setup() {
  serial.init(&led); // Should be first
  led.init(&serial);
  touch.init(&serial);
  air.init();

  uint32_t clock = Wire.getClock();
  serial.writeDebugLogf("Wire clock speed is %d hz", clock);

  serial.writeDebugLog("Preparing for calibration...")->processWrite();
  led.calibrate();
  delay(3000);
  serial.writeDebugLog("Calibrating...")->processWrite();
  air.calibrate();
  serial.writeDebugLog("Finished calibrating!")->processWrite();
  led.setAllUntouched();

  startTime = micros();
}

void loop() {
#ifdef REPORT_LATENCY_METRICS
  unsigned long start = micros();
#endif
  serial.processWrite();
#ifdef REPORT_LATENCY_METRICS
  static unsigned long sum = 0;
  static unsigned long count = 0;
  unsigned long dt = micros() - start;
  if (count >= LatencyMetricSampleCount * 8) {
    unsigned long us = sum / count;
    serial.writeDebugLogf("Latency metric for processing serial write: %d us", us);
    sum = 0;
    count = 0;
  }
  sum += dt;
  count++;
#endif

  if (micros() - startTime > SensorReadFrequencyMicros) {
#ifdef TEST_MODE
    processSensorDataTest();
#else
    processSensorData();
#endif
    startTime = micros();
  }

  delayMicroseconds(50);
}

void serialEvent() {
#ifdef REPORT_LATENCY_METRICS
  unsigned long start = micros();
#endif
  serial.read();
#ifdef REPORT_LATENCY_METRICS
  static unsigned long sum = 0;
  static unsigned long count = 0;
  unsigned long dt = micros() - start;
  if (count >= LatencyMetricSampleCount / 4) {
    unsigned long us = sum / count;
    serial.writeDebugLogf("Latency metric for processing serial read: %d us", us);
    sum = 0;
    count = 0;
  }
  sum += dt;
  count++;
#endif
}

void processSensorData() {
#ifdef REPORT_LATENCY_METRICS
  unsigned long start = micros();
#endif
  processAirSensorData();
  processSliderData();
#ifdef REPORT_LATENCY_METRICS
  static unsigned long sum = 0;
  static unsigned long count = 0;
  unsigned long dt = micros() - start;
  if (count >= LatencyMetricSampleCount) {
    unsigned long us = sum / count;
    serial.writeDebugLogf("Latency metric for processing sensor data: %d us", us);
    sum = 0;
    count = 0;
  }
  sum += dt;
  count++;
#endif
}

/* Poll JVS input.

   opbtn returns the cabinet test/service state, where bit 0 is Test and Bit 1
   is Service.

   beam returns the IR beams that are currently broken, where bit 0 is the
   lowest IR beam and bit 5 is the highest IR beam, for a total of six beams.

   Both bit masks are active-high.

   Note that you cannot instantly break the entire IR grid in a single frame to
   simulate hand movement; this will be judged as a miss. You need to simulate
   a gradual raising and lowering of the hands. Consult the proof-of-concept
   implementation for details.

   NOTE: Previous releases of Segatools mapped the IR beam inputs incorrectly.
   Please ensure that you advertise an API version of at least 0x0101 so that
   the correct mapping can be used.

   Minimum API version: 0x0100
   Latest API version: 0x0101 */
void processAirSensorData() {
  // Data is already in expected format
  uint8_t blocked = air.getBlockedSensors();
  serial.writeAirSensorData(&blocked, 1);
}

/* Chunithm touch slider layout:

                               ^^^ Toward screen ^^^

----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 31 | 29 | 27 | 25 | 23 | 21 | 19 | 17 | 15 | 13 | 11 |  9 |  7 |  5 |  3 |  1 |
----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 32 | 30 | 28 | 26 | 24 | 22 | 20 | 18 | 16 | 14 | 12 | 10 |  8 |  6 |  4 |  2 |
----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

   There are a total of 32 regions on the touch slider. Each region can return
   an 8-bit pressure value. The operator menu allows the operator to adjust the
   pressure level at which a region is considered to be pressed; the factory
   default value for this setting is 20. */

/* Callback function supplied to your IO DLL. This must be called with a
   pointer to a 32-byte array of pressure values, one byte per slider cell.
   See above for layout and pressure threshold information.

   The callback will copy the pressure state data out of your buffer before
   returning. The pointer will not be retained. */
void processSliderData() {
  touch.getTouchStatus(touchData);
  uint8_t keyData[32];
  int idx = 0;
  bool invert = false;
  for (int i = 0; i < 4; i++) {
    uint16_t touched = touchData.touched[i];
    for (int j = 0; j < 8; j += 2) {
      int k1 = touched & (1 << (j)) ? SliderTouchedPressureValue : 0;
      int k2 = touched & (1 << (j + 1)) ? SliderTouchedPressureValue : 0;
      if (invert) {
        keyData[idx++] = k2;
        keyData[idx++] = k1;
      } else {
        keyData[idx++] = k1;
        keyData[idx++] = k2;
      }
      invert = !invert;
    }
  }
  serial.writeSliderData(keyData, sizeof(keyData));
}

void processSensorDataTest() {
  // Read and update air sensor
  uint8_t blocked = air.getBlockedSensors();
  led.setBeamBroken(blocked > 0);

  // Read and update slider
  touch.getTouchStatus(touchData);
  for (int i = 0; i < 4; i++) {
    uint16_t touched = touchData.touched[i];
    for (int j = 0; j < 4; j++) {
      int idx = LED_KEY_COUNT - 1 - (4 * i + j);
      if (touched & (1 << (2 * j)) || touched & (1 << (2 * j + 1))) {
        ledData[idx] = 1;
        serial.writeDebugLogf("Key pressed %d", idx);
      } else {
        ledData[idx] = 0;
      }
    }
  }
  led.set(ledData, LED_KEY_COUNT);

  // air.get(airSensorData);
  // Serial.printf("[%d %d %d %d %d %d]\n", airSensorData.values[0], airSensorData.values[1], airSensorData.values[2],
  // airSensorData.values[3], airSensorData.values[4], airSensorData.values[5]);
}
