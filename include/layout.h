#ifndef LAYOUT_H
#define LAYOUT_H

enum LedDirection { LeftToRight = 0, RightToLeft = 1 };

//        Touch sensor layout (view from the front)
// ---------------------------------------------------------
// |    0x5D     |    0x5B     |    0x5A     |    0x5C     |
// ---------------------------------------------------------
// | 07 04 03 00 | 07 04 03 00 | 07 04 03 00 | 07 04 03 00 |
// | 06 05 02 01 | 06 05 02 01 | 06 05 02 01 | 06 05 02 01 |
//----------------------------------------------------------
// | 31 28 27 24 | 23 20 19 16 | 15 12 11 08 | 07 04 03 00 |
// | 30 29 26 25 | 22 21 18 17 | 14 13 10 09 | 06 05 02 01 |
// ---------------------------------------------------------
#define MPR121_ADDR_0 0x5C
#define MPR121_ADDR_1 0x5A
#define MPR121_ADDR_2 0x5B
#define MPR121_ADDR_3 0x5D

#define LED_COUNT 18
#define LED_KEY_COUNT 16
#define LED_OFFSET 2              // Offset from the first LED
#define LED_DIRECTION RightToLeft // 0 = left->right, 1 = right->left

#define AIR_SENSOR_COUNT 6

// --- Pins ---

#define GPIO_I2C_SCL 22
#define GPIO_I2C_SDA 21

#define GPIO_LED_DATA 4

#define GPIO_IR_LED_0 17
#define GPIO_IR_LED_1 18
#define GPIO_IR_LED_2 19

#define GPIO_IR_DATA_0 33
#define GPIO_IR_DATA_1 32
#define GPIO_IR_DATA_2 35
#define GPIO_IR_DATA_3 34
#define GPIO_IR_DATA_4 39
#define GPIO_IR_DATA_5 36

// Not used currently
#define GPIO_INT_0 23
#define GPIO_INT_1 16
#define GPIO_INT_2 14
#define GPIO_INT_3 27

#endif
