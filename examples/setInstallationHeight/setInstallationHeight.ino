/*!
 * @file setInstallationHeight.ino
 * @brief Set the SEN0778 empty-tank distance
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author wxzed(xiao.wu@dfrobot.com)
 * @version V1.0.0
 * @date 2026-10-12
 * @url https://github.com/DFRobot/DFRobot_LD010
 */

/**
 * 传感器经 DFR0845 接到开发板。传感器棕色 VCC 接 DC 9-28V，不要接开发板 5V。
 * 蓝色接 9-28V 电源负极，黑色 RS485-A 接模块 A，黄色 RS485-B 接模块 B。
 * 模块 UART 的 VCC/GND 接开发板。模块 TX 接 MCU RX，模块 RX 接 MCU TX。
 *
 * Board      MCU RX                 MCU TX
 * UNO        RX 4                   TX 5          SoftwareSerial(4, 5)
 * MEGA2560   RX 15                  TX 14         Serial3
 * Leonardo   RX 0                   TX 1          Serial1
 * ESP32      RX 16                  TX 17         Serial1
 *
 * INSTALLATION_MM 会覆盖传感器里保存的空罐距离，范围 20-10000 mm。
 * 液位高度由传感器按该距离计算，库不做减法。
 */

#include "DFRobot_LD010.h"

static const uint16_t INSTALLATION_MM = 1000;

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

  uint16_t installation = 0;
  Serial.print("installation mm now: ");
  if (sensor.getInstallationHeight(&installation)) Serial.println(installation);
  else Serial.println("read failed");

  Serial.print("set installation mm: ");
  Serial.println(INSTALLATION_MM);
  Serial.println(sensor.setInstallationHeight(INSTALLATION_MM) ? "setInstallationHeight ok" : "setInstallationHeight failed");
}

void loop(void) {
  uint16_t installation = 0;
  uint16_t emptyHeight = 0;
  uint16_t liquid = 0;

  Serial.print("installation mm: ");
  if (sensor.getInstallationHeight(&installation)) Serial.println(installation);
  else Serial.println("failed");

  Serial.print("empty mm: ");
  if (sensor.getEmptyHeight(&emptyHeight)) Serial.println(emptyHeight);
  else Serial.println("failed");

  Serial.print("liquid mm: ");
  if (sensor.getLiquidHeight(&liquid)) Serial.println(liquid);
  else Serial.println("failed");

  Serial.println("---");
  delay(500);
}
