/*!
 * @file DFRobot_LD010.h
 * @brief 60GHz liquid-level radar, Modbus RTU over RS485
 * @details Host library for SEN0778 / XKC-LD010-R7R1. Register addresses match the vendor examples and are not decremented.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author wxzed(xiao.wu@dfrobot.com)
 * @version V1.0.0
 * @date 2026-10-12
 * @url https://github.com/DFRobot/DFRobot_LD010
 */

#ifndef __DFROBOT_LD010_H__
#define __DFROBOT_LD010_H__

#include <Arduino.h>
#include <DFRobot_RTU.h>

#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#endif

#define LD010_REG_ADDRESS         0x0010  ///< slave address register
#define LD010_REG_DISTANCE        0x0011  ///< distance from sensor to surface, mm
#define LD010_REG_CALIBRATE       0x0012  ///< write 1 to calibrate the empty tank
#define LD010_REG_UPLOAD          0x0013  ///< 0 = poll, 1 = automatic upload
#define LD010_REG_BAUD            0x0015  ///< baud-rate code 1-8
#define LD010_REG_FACTORY         0x0016  ///< write 1 to restore factory settings
#define LD010_REG_STATUS          0x0017  ///< system status bitmap
#define LD010_REG_EMPTY_TANK      0x001A  ///< empty-tank / installation height, mm
#define LD010_REG_LIQUID          0x001B  ///< liquid height, mm
#define LD010_REG_ALARM           0x001C  ///< liquid-level alarm threshold, mm
#define LD010_TIMEOUT_MS          500     ///< host timeout for one Modbus transaction
#define LD010_PORT_SWITCH_MS      200     ///< host wait after the sensor accepts a baud or factory change
#define LD010_STATUS_LEVEL_ALARM  0x0001  ///< bit0: liquid height has reached the threshold
#define LD010_STATUS_CAL_FAIL     0x0002  ///< bit1: 1 means calibration did not succeed
#define LD010_STATUS_INIT_FAIL    0x0004  ///< bit2: 1 means power-on init did not succeed

/**
 * @enum eBaudRate_t
 * @brief Baud-rate code stored in register 0x0015
 */
typedef enum {
  eBaud1200 = 1,    ///< 1200 bps
  eBaud2400 = 2,    ///< 2400 bps
  eBaud4800 = 3,    ///< 4800 bps
  eBaud9600 = 4,    ///< 9600 bps, factory default
  eBaud19200 = 5,   ///< 19200 bps
  eBaud38400 = 6,   ///< 38400 bps
  eBaud57600 = 7,   ///< 57600 bps
  eBaud115200 = 8   ///< 115200 bps
} eBaudRate_t;

/**
 * @enum eUploadMode_t
 * @brief Distance upload mode stored in register 0x0013
 */
typedef enum {
  eUploadOff = 0,  ///< do not push distance frames
  eUploadOn = 1    ///< push distance frames; disable this when sensors are cascaded
} eUploadMode_t;

class DFRobot_LD010 {
public:
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
  /**
   * @fn DFRobot_LD010
   * @brief Construct a sensor on a software serial port
   * @param sSerial Software serial port already assigned to the DFR0845 UART pins
   * @param baud Initial host baud rate in bps. Factory default is 9600
   * @param addr Initial Modbus slave address, 1-247. Factory default is 1
   * @n Modbus framing, CRC and the receive timeout go through DFRobot_RTU. DFR0845 switches direction by itself, so no DE pin is passed in.
   * @n The sensor itself is powered from DC 9-28 V, not from the MCU 5 V pin.
   */
  DFRobot_LD010(SoftwareSerial *sSerial, uint32_t baud = 9600, uint8_t addr = 1);
#else
  /**
   * @fn DFRobot_LD010
   * @brief Construct a sensor on a hardware serial port
   * @param hSerial Hardware serial port connected to DFR0845
   * @param baud Initial host baud rate in bps. Factory default is 9600
   * @param addr Initial Modbus slave address, 1-247. Factory default is 1
   * @param rxPin ESP32 RX pin. Leave 0 on AVR boards
   * @param txPin ESP32 TX pin. Leave 0 on AVR boards
   * @n Mega 2560 uses Serial3: MCU RX is pin 15 and MCU TX is pin 14.
   * @n Modbus framing, CRC and the receive timeout go through DFRobot_RTU. DFR0845 switches direction by itself, so no DE pin is passed in.
   * @n The sensor itself is powered from DC 9-28 V, not from the MCU 5 V pin.
   */
  DFRobot_LD010(HardwareSerial *hSerial, uint32_t baud = 9600, uint8_t addr = 1, uint8_t rxPin = 0, uint8_t txPin = 0);
#endif

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

protected:
  bool readHolding(uint16_t reg, uint16_t *value);
  bool writeHolding(uint16_t reg, uint16_t value);

private:
  void openPort(uint32_t baud);
  uint32_t baudToBps(eBaudRate_t baud);

  DFRobot_RTU _rtu;
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
  SoftwareSerial *_sSerial;
#else
  HardwareSerial *_hSerial;
  uint8_t _rxPin;
  uint8_t _txPin;
#endif
  uint32_t _baud;
  uint8_t _addr;
};

#endif
