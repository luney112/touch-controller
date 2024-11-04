#ifndef TOUCH_H
#define TOUCH_H

#include "Adafruit_MPR121.h"

class SerialController;

struct TouchData {
  uint16_t touched[4];
};

class TouchController {
public:
  bool init(SerialController *serial);
  void getTouchStatus(TouchData &data);

private:
  Adafruit_MPR121 caps[4];

  SerialController *serial;
};

#endif
