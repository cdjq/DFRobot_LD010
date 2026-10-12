# DFRobot_LD010

- [中文版](./README_CN.md)

60GHz non-contact radar liquid-level sensor with Modbus RTU over RS485. It measures liquid or material level from 0.2 m to 10 m through the Gravity isolated RS485-to-UART module DFR0845. SKU SEN0778, module XKC-LD010-R7R1.

## Product Link

SKU SEN0778

## Table of Contents

- [Summary](#summary)
- [Installation](#installation)
- [Methods](#methods)
- [Compatibility](#compatibility)
- [History](#history)
- [Credits](#credits)

## Summary

* Reads empty height, liquid height and installation height in millimetres
* Sets slave address, baud rate, alarm threshold and empty-tank distance
* Calibrates the empty tank and restores factory communication settings
* Talks Modbus RTU through the DFRobot_RTU dependency, function 03H/06H, default address 1, 9600 8N1
* Uses DFR0845 for RS485. The sensor supply is DC 9-28 V

## Installation

1. Install `DFRobot_RTU` from the Arduino Library Manager. This library depends on it for Modbus framing and CRC.
2. Search `DFRobot_LD010` in the Library Manager and install it, or place this folder in the Arduino `libraries` directory.
3. Open `examples/readLiquidLevel`.

There is no product photo in this package.

## Methods

```C++
  /**
   * @fn begin
   * @brief Open the UART and confirm the slave answers
   * @return true when register 0x0010 reads back the address passed to the constructor
   * @n Default link is 9600 8N1, slave address 1.
   * @n A failed begin usually means the RS485 A/B pair is swapped, the sensor has no 9-28 V supply, or the address/baud was already changed.
   */
  bool begin(void);

  /**
   * @fn setDeviceAddress
   * @brief Set the Modbus slave address
   * @param address New address, 1-247
   * @return true when the sensor echoes the write
   * @n Address 0 is the broadcast address and is rejected.
   * @n After a successful write, later calls use the new address. The echo itself still uses the old address.
   */
  bool setDeviceAddress(uint8_t address);

  /**
   * @fn getDeviceAddress
   * @brief Read the slave address from register 0x0010
   * @param address Receives the address
   * @return true when the register was read
   * @n This is a direct holding-register read, not a cached notification.
   */
  bool getDeviceAddress(uint8_t *address);

  /**
   * @fn setBaudRate
   * @brief Set the sensor baud rate
   * @param baud Baud code, not a raw bps value
   * @n eBaud1200 = 1, 1200 bps
   * @n eBaud2400 = 2, 2400 bps
   * @n eBaud4800 = 3, 4800 bps
   * @n eBaud9600 = 4, 9600 bps, factory default
   * @n eBaud19200 = 5, 19200 bps
   * @n eBaud38400 = 6, 38400 bps
   * @n eBaud57600 = 7, 57600 bps
   * @n eBaud115200 = 8, 115200 bps
   * @return true when the sensor echoes the write
   * @n The echo arrives at the old baud rate. This method then waits and reopens the local UART at the new baud rate.
   */
  bool setBaudRate(eBaudRate_t baud);

  /**
   * @fn getBaudRate
   * @brief Read the baud-rate code from register 0x0015
   * @param baud Receives the code 1-8
   * @return true when the code is one of the eight defined values
   * @n The value is the wire code. 4 means 9600 bps, not the fourth item in a zero-based list.
   */
  bool getBaudRate(eBaudRate_t *baud);

  /**
   * @fn setLiquidAlarm
   * @brief Set the liquid-height alarm threshold
   * @param thresholdMm Threshold in millimetres, 0-10000
   * @return true when the sensor echoes the write
   * @n Status bit0 becomes 1 when the liquid height is greater than or equal to this threshold.
   * @n A threshold of 0 alarms whenever the reported liquid height is greater than or equal to 0.
   */
  bool setLiquidAlarm(uint16_t thresholdMm);

  /**
   * @fn getLiquidAlarm
   * @brief Read the liquid-height alarm threshold
   * @param thresholdMm Receives the threshold in millimetres
   * @return true when the register was read
   * @n The unit is mm. This does not read the live liquid height.
   */
  bool getLiquidAlarm(uint16_t *thresholdMm);

  /**
   * @fn setInstallationHeight
   * @brief Write the empty-tank distance, which is the installation height
   * @param heightMm Distance in millimetres, 20-10000
   * @return true when the sensor echoes the write
   * @n Values outside 20-10000 mm are rejected and are not sent.
   * @n This is the distance from the sensor to the tank bottom, not the liquid height.
   */
  bool setInstallationHeight(uint16_t heightMm);

  /**
   * @fn getInstallationHeight
   * @brief Read the empty-tank distance
   * @param heightMm Receives the installation height in millimetres
   * @return true when the register was read
   * @n The unit is mm. A communication failure returns false; it does not return 0.
   */
  bool getInstallationHeight(uint16_t *heightMm);

  /**
   * @fn calibrateEmptyDistance
   * @brief Start empty-tank calibration from the distance currently seen
   * @return true when the sensor echoes the write of 1
   * @n The return value only means the command was accepted.
   * @n Read getSystemStatus() afterwards. Bit1 = 0 means calibration succeeded, bit1 = 1 means it did not.
   * @n Do not calibrate while liquid is in the tank; the current distance becomes the empty-tank distance.
   * @n The manual does not specify how long calibration takes.
   */
  bool calibrateEmptyDistance(void);

  /**
   * @fn getEmptyHeight
   * @brief Read the distance from the sensor to the liquid surface
   * @param heightMm Receives the distance in millimetres
   * @return true when register 0x0011 was read
   * @n Readings from 0 to 200 mm are inside the near blind zone and are not trustworthy.
   * @n Readings above 10000 mm are outside the far limit and are not trustworthy.
   * @n An empty beam often returns 0. That 0 is a sensor reading, not a communication failure.
   */
  bool getEmptyHeight(uint16_t *heightMm);

  /**
   * @fn getLiquidHeight
   * @brief Read the liquid height calculated by the sensor
   * @param heightMm Receives the liquid height in millimetres
   * @return true when register 0x001B was read
   * @n The sensor derives this from the empty-tank distance. This library does not subtract the values itself.
   * @n A communication failure returns false. A reading of 0 mm is still a successful read.
   */
  bool getLiquidHeight(uint16_t *heightMm);

  /**
   * @fn restoreFactory
   * @brief Restore factory communication settings
   * @return true when the sensor echoes the write of 1
   * @n The manual states these defaults: slave address 1, upload mode 0, baud code 4 (9600).
   * @n The manual does not state the factory empty-tank distance or alarm threshold.
   * @n The echo arrives on the old address and baud. This method then waits and reopens the UART at address 1 and 9600 8N1.
   */
  bool restoreFactory(void);

  /**
   * @fn setUploadMode
   * @brief Choose whether the sensor pushes distance by itself
   * @param mode Upload mode
   * @n eUploadOff = 0, no automatic upload, factory default
   * @n eUploadOn = 1, automatic upload. Turn this off when several sensors share the bus.
   * @return true when the sensor echoes the write
   * @n The manual does not define the automatic-upload frame. Distance is always read with function 03H.
   */
  bool setUploadMode(eUploadMode_t mode);

  /**
   * @fn getUploadMode
   * @brief Read the distance upload mode
   * @param mode Receives 0 or 1
   * @return true when the register contains 0 or 1
   * @n 0 means polling. 1 means automatic upload, which must stay off on a cascaded bus.
   */
  bool getUploadMode(eUploadMode_t *mode);

  /**
   * @fn getSystemStatus
   * @brief Read the status word
   * @param status Receives register 0x0017
   * @return true when the register was read
   * @n Bit0 = 1: liquid height has reached the alarm threshold.
   * @n Bit1 = 0: calibration succeeded. Bit1 = 1: calibration did not succeed.
   * @n Bit2 = 0: power-on init succeeded. Bit2 = 1: power-on init did not succeed.
   * @n The other bits are not defined by the manual.
   */
  bool getSystemStatus(uint16_t *status);
```

## Compatibility

| Board | Status | Note |
|-------|--------|------|
| Arduino UNO | Untested | SoftwareSerial RX 4, TX 5 |
| Arduino Mega 2560 | Untested | UART3, RX 15, TX 14 |
| Arduino Leonardo | Untested | Serial1, RX 0, TX 1 |
| ESP32 / FireBeetle-ESP32 | Untested | Serial1, RX GPIO16, TX GPIO17 |
| Raspberry Pi 4B/5B | Not in this library | Arduino package only |

## History

- 2026/10/12 - V1.0.0 version

## Credits

Written by wxzed(xiao.wu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
