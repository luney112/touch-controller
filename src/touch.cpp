#include "touch.h"
#include "layout.h"
#include "serial.h"

#define SET_WIRE_CLOCK_SPEED

constexpr uint32_t WireClockSpeed = 400000;

bool TouchController::init(SerialController *serial) {
  this->serial = serial;

  if (!caps[0].begin(MPR121_ADDR_0) || !caps[1].begin(MPR121_ADDR_1) || !caps[2].begin(MPR121_ADDR_2) ||
      !caps[3].begin(MPR121_ADDR_3)) {
    serial->writeDebugLog("[ERROR] Unable to find one or more MRP121 i2c device(s)")->processWrite();
    return false;
  }

#ifdef SET_WIRE_CLOCK_SPEED
  Wire.setClock(WireClockSpeed);
#endif

  serial->writeDebugLog("Initialized MRP121s")->processWrite();
  return true;
}

void TouchController::getTouchStatus(TouchData &data) {
  for (int i = 0; i < 4; i++) {
    data.touched[i] = caps[i].touched();
  }
}
