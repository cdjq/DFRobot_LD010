/*!
 * @file setLiquidAlarm.ino
 * @brief Set the SEN0778 liquid-level alarm threshold
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author wxzed(xiao.wu@dfrobot.com)
 * @version V1.0.0
 * @date 2026-10-12
 * @url https://github.com/DFRobot/DFRobot_LD010
 */

/**
 * Connect the sensor through DFR0845. Sensor brown VCC uses DC 9-28V, not the board 5V pin.
 * Blue goes to the 9-28V supply negative. Black RS485-A goes to module A. Yellow RS485-B goes to module B.
 * Connect the module UART VCC and GND to the board. Module TX goes to MCU RX. Module RX goes to MCU TX.
 *
 * Board      MCU RX                 MCU TX
 * UNO        RX 4                   TX 5          SoftwareSerial(4, 5)
 * MEGA2560   RX 15                  TX 14         Serial3
 * Leonardo   RX 0                   TX 1          Serial1
 * ESP32      RX 16                  TX 17         Serial1
 *
 * ALARM_MM is written to the sensor. Status bit0 becomes 1 when the liquid height is greater than or equal to this value.
 * A value of 0 means any non-negative liquid height meets the alarm condition.
 */

#include "DFRobot_LD010.h"

static const uint16_t ALARM_MM = 1000;

#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include <SoftwareSerial.h>
SoftwareSerial softSerial(4, 5);
DFRobot_LD010 sensor(&softSerial, 9600, 1);
#elif defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MEGA)
DFRobot_LD010 sensor(&Serial3, 9600, 1);
#elif defined(ESP32)
DFRobot_LD010 sensor(&Serial1, 9600, 1, /*rx*/16, /*tx*/17);
#else
DFRobot_LD010 sensor(&Serial1, 9600, 1);
#endif

void setup(void) {
  Serial.begin(115200);
  while (!sensor.begin()) {
    Serial.println("LD010 begin failed, check 9-28V and RS485 A/B");
    delay(1000);
  }
  Serial.println("LD010 online");

  uint16_t alarm = 0;
  Serial.print("alarm mm now: ");
  if (sensor.getLiquidAlarm(&alarm)) Serial.println(alarm);
  else Serial.println("read failed");

  Serial.print("set alarm mm: ");
  Serial.println(ALARM_MM);
  Serial.println(sensor.setLiquidAlarm(ALARM_MM) ? "setLiquidAlarm ok" : "setLiquidAlarm failed");
}

void loop(void) {
  uint16_t alarm = 0;
  uint16_t liquid = 0;
  uint16_t status = 0;

  Serial.print("alarm mm: ");
  if (sensor.getLiquidAlarm(&alarm)) Serial.println(alarm);
  else Serial.println("failed");

  Serial.print("liquid mm: ");
  if (sensor.getLiquidHeight(&liquid)) Serial.println(liquid);
  else Serial.println("failed");

  if (sensor.getSystemStatus(&status)) {
    Serial.println((status & LD010_STATUS_LEVEL_ALARM) ? "level alarm: yes" : "level alarm: no");
  } else {
    Serial.println("status failed");
  }
  Serial.println("---");
  delay(500);
}
