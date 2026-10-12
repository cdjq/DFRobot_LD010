/*!
 * @file readLiquidLevel.ino
 * @brief Read SEN0778 liquid level over RS485
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
 * ESP32      RX D3                  TX D2         Serial1
 */

#include "DFRobot_LD010.h"

#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include <SoftwareSerial.h>
SoftwareSerial softSerial(4, 5);
DFRobot_LD010 sensor(&softSerial, 9600, 1);
#elif defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MEGA)
DFRobot_LD010 sensor(&Serial3, 9600, 1);
#elif defined(ESP32)
DFRobot_LD010 sensor(&Serial1, 9600, 1, /*rx*/D3, /*tx*/D2);
#else
DFRobot_LD010 sensor(&Serial1, 9600, 1);
#endif

static void printResult(const char *name, bool ok) {
  Serial.print(name);
  Serial.println(ok ? " ok" : " failed");
}

void setup(void) {
  Serial.begin(115200);
  while (!sensor.begin()) {
    Serial.println("LD010 begin failed, check 9-28V and RS485 A/B");
    delay(1000);
  }
  Serial.println("LD010 online");

  uint8_t address = 0;
  eBaudRate_t baud = eBaud9600;
  uint16_t alarm = 0;
  uint16_t installation = 0;
  bool gotAddress = sensor.getDeviceAddress(&address);
  bool gotBaud = sensor.getBaudRate(&baud);
  bool gotAlarm = sensor.getLiquidAlarm(&alarm);
  bool gotInstallation = sensor.getInstallationHeight(&installation);

  Serial.print("address: ");
  if (gotAddress) Serial.println(address); else Serial.println("read failed");
  Serial.print("baud code: ");
  if (gotBaud) Serial.println((int)baud); else Serial.println("read failed");
  Serial.print("alarm mm: ");
  if (gotAlarm) Serial.println(alarm); else Serial.println("read failed");
  Serial.print("installation mm: ");
  if (gotInstallation) Serial.println(installation); else Serial.println("read failed");

  if (gotAddress) printResult("setDeviceAddress", sensor.setDeviceAddress(address));
  if (gotBaud) printResult("setBaudRate", sensor.setBaudRate(baud));
  if (gotAlarm) printResult("setLiquidAlarm", sensor.setLiquidAlarm(alarm));
  if (gotInstallation && installation >= 20 && installation <= 10000) {
    printResult("setInstallationHeight", sensor.setInstallationHeight(installation));
  } else if (gotInstallation) {
    Serial.println("installation outside 20-10000, skip write-back");
  }
}

void loop(void) {
  uint16_t installation = 0;
  uint16_t emptyHeight = 0;
  uint16_t liquid = 0;
  uint16_t status = 0;

  Serial.print("installation mm: ");
  if (sensor.getInstallationHeight(&installation)) Serial.println(installation);
  else Serial.println("failed");

  Serial.print("empty mm: ");
  if (sensor.getEmptyHeight(&emptyHeight)) Serial.println(emptyHeight);
  else Serial.println("failed");

  Serial.print("liquid mm: ");
  if (sensor.getLiquidHeight(&liquid)) Serial.println(liquid);
  else Serial.println("failed");

  if (sensor.getSystemStatus(&status)) {
    Serial.print("status: 0x");
    Serial.println(status, HEX);
    Serial.println((status & LD010_STATUS_LEVEL_ALARM) ? "level alarm: yes" : "level alarm: no");
    Serial.println((status & LD010_STATUS_CAL_FAIL) ? "calibration bit: not success" : "calibration bit: success");
    Serial.println((status & LD010_STATUS_INIT_FAIL) ? "init bit: not success" : "init bit: success");
  } else {
    Serial.println("status failed");
  }
  Serial.println("---");
  delay(500);
}
