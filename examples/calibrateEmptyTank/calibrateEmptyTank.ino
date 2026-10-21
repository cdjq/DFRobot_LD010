/*!
 * @file calibrateEmptyTank.ino
 * @brief Calibrate SEN0778 only when the tank is empty
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
 * Calibration stores the current distance as the empty-tank distance. Do not send a character while liquid is in the tank.
 * Open the serial monitor and send any character to calibrate once.
 * The library only sends the command. This example polls status for 5 seconds. A missing echo during calibration is not a failure.
 */

#include "DFRobot_LD010.h"

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

static const uint32_t CALIBRATE_WINDOW_MS = 5000;
static const uint16_t CALIBRATE_POLL_MS = 200;
static bool calibrated = false;

void setup(void) {
  Serial.begin(115200);
  while (!sensor.begin()) {
    Serial.println("LD010 begin failed, check 9-28V and RS485 A/B");
    delay(1000);
  }
  Serial.println("LD010 online");
  Serial.println("Tank must be empty. Send any character to calibrate once.");

  // If the empty-tank distance is already known, write it directly instead of calibrating. Range is 20-10000 mm.
  // uint16_t heightMm = 0;
  // sensor.setInstallationHeight(1000);
  // sensor.getInstallationHeight(&heightMm);
}

void loop(void) {
  if (!calibrated && Serial.available()) {
    while (Serial.available()) Serial.read();
    Serial.println(sensor.calibrateEmptyDistance() ? "command echoed" : "no echo yet");
    Serial.println("polling status");

    uint32_t start = millis();
    uint16_t status = 0;
    bool sawNotDone = false;
    bool success = false;
    while ((uint32_t)(millis() - start) < CALIBRATE_WINDOW_MS) {
      if (sensor.getSystemStatus(&status)) {
        if ((status & LD010_STATUS_CAL_FAIL) != 0) {
          sawNotDone = true;
        } else if (sawNotDone) {
          success = true;
          break;
        }
      }
      delay(CALIBRATE_POLL_MS);
    }
    if (!success && sensor.getSystemStatus(&status)) {
      success = (status & LD010_STATUS_CAL_FAIL) == 0;
    }

    uint16_t installation = 0;
    Serial.print("status: 0x");
    Serial.println(status, HEX);
    Serial.println(success ? "calibration bit: success" : "calibration bit: not success");
    Serial.print("installation mm: ");
    if (sensor.getInstallationHeight(&installation)) Serial.println(installation);
    else Serial.println("failed");
    calibrated = true;
  }
}
