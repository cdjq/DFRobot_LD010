/*!
 * @file DFRobot_LD010.cpp
 * @brief Modbus RTU host for the SEN0778 liquid-level radar
 * @details Function 03H/06H and CRC are provided by DFRobot_RTU. PDU addresses are the manual addresses.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author wxzed(xiao.wu@dfrobot.com)
 * @version V1.0.0
 * @date 2026-10-12
 * @url https://github.com/DFRobot/DFRobot_LD010
 */

#include "DFRobot_LD010.h"

#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
DFRobot_LD010::DFRobot_LD010(SoftwareSerial *sSerial, uint32_t baud, uint8_t addr)
  : _rtu(sSerial), _sSerial(sSerial), _baud(baud), _addr(addr) {
  _rtu.setTimeoutTimeMs(LD010_TIMEOUT_MS);
}
#else
DFRobot_LD010::DFRobot_LD010(HardwareSerial *hSerial, uint32_t baud, uint8_t addr, uint8_t rxPin, uint8_t txPin)
  : _rtu(hSerial), _hSerial(hSerial), _rxPin(rxPin), _txPin(txPin), _baud(baud), _addr(addr) {
  _rtu.setTimeoutTimeMs(LD010_TIMEOUT_MS);
}
#endif

bool DFRobot_LD010::begin(void) {
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
  if (_sSerial == nullptr || _addr < 1 || _addr > 247) {
#else
  if (_hSerial == nullptr || _addr < 1 || _addr > 247) {
#endif
    return false;
  }
  openPort(_baud);
  delay(100);
  uint16_t id = 0;
  if (!readHolding(LD010_REG_ADDRESS, &id)) {
    return false;
  }
  return id == _addr;
}

bool DFRobot_LD010::setDeviceAddress(uint8_t address) {
  if (address < 1 || address > 247) {
    return false;
  }
  if (!writeHolding(LD010_REG_ADDRESS, address)) {
    return false;
  }
  _addr = address;
  return true;
}

bool DFRobot_LD010::getDeviceAddress(uint8_t *address) {
  uint16_t value = 0;
  if (address == nullptr || !readHolding(LD010_REG_ADDRESS, &value) || value > 247) {
    return false;
  }
  *address = (uint8_t)value;
  return true;
}

bool DFRobot_LD010::setBaudRate(eBaudRate_t baud) {
  uint32_t bps = baudToBps(baud);
  if (bps == 0 || !writeHolding(LD010_REG_BAUD, (uint16_t)baud)) {
    return false;
  }
  _baud = bps;
  delay(LD010_PORT_SWITCH_MS);
  openPort(_baud);
  return true;
}

bool DFRobot_LD010::getBaudRate(eBaudRate_t *baud) {
  uint16_t value = 0;
  if (baud == nullptr || !readHolding(LD010_REG_BAUD, &value)) {
    return false;
  }
  if (value < eBaud1200 || value > eBaud115200) {
    return false;
  }
  *baud = (eBaudRate_t)value;
  return true;
}

bool DFRobot_LD010::setLiquidAlarm(uint16_t thresholdMm) {
  if (thresholdMm > 10000) {
    return false;
  }
  return writeHolding(LD010_REG_ALARM, thresholdMm);
}

bool DFRobot_LD010::getLiquidAlarm(uint16_t *thresholdMm) {
  if (thresholdMm == nullptr) {
    return false;
  }
  return readHolding(LD010_REG_ALARM, thresholdMm);
}

bool DFRobot_LD010::setInstallationHeight(uint16_t heightMm) {
  if (heightMm < 20 || heightMm > 10000) {
    return false;
  }
  return writeHolding(LD010_REG_EMPTY_TANK, heightMm);
}

bool DFRobot_LD010::getInstallationHeight(uint16_t *heightMm) {
  if (heightMm == nullptr) {
    return false;
  }
  return readHolding(LD010_REG_EMPTY_TANK, heightMm);
}

bool DFRobot_LD010::calibrateEmptyDistance(void) {
  return writeHolding(LD010_REG_CALIBRATE, 1);
}

bool DFRobot_LD010::getEmptyHeight(uint16_t *heightMm) {
  if (heightMm == nullptr) {
    return false;
  }
  return readHolding(LD010_REG_DISTANCE, heightMm);
}

bool DFRobot_LD010::getLiquidHeight(uint16_t *heightMm) {
  if (heightMm == nullptr) {
    return false;
  }
  return readHolding(LD010_REG_LIQUID, heightMm);
}

bool DFRobot_LD010::restoreFactory(void) {
  if (!writeHolding(LD010_REG_FACTORY, 1)) {
    return false;
  }
  _addr = 1;
  _baud = 9600;
  delay(LD010_PORT_SWITCH_MS);
  openPort(_baud);
  return true;
}

bool DFRobot_LD010::setUploadMode(eUploadMode_t mode) {
  if (mode != eUploadOff && mode != eUploadOn) {
    return false;
  }
  return writeHolding(LD010_REG_UPLOAD, (uint16_t)mode);
}

bool DFRobot_LD010::getUploadMode(eUploadMode_t *mode) {
  uint16_t value = 0;
  if (mode == nullptr || !readHolding(LD010_REG_UPLOAD, &value)) {
    return false;
  }
  if (value != eUploadOff && value != eUploadOn) {
    return false;
  }
  *mode = (eUploadMode_t)value;
  return true;
}

bool DFRobot_LD010::getSystemStatus(uint16_t *status) {
  if (status == nullptr) {
    return false;
  }
  return readHolding(LD010_REG_STATUS, status);
}

bool DFRobot_LD010::readHolding(uint16_t reg, uint16_t *value) {
  if (value == nullptr || _addr < 1 || _addr > 247) {
    return false;
  }
  return _rtu.readHoldingRegister(_addr, reg, value, 1) == 0;
}

bool DFRobot_LD010::writeHolding(uint16_t reg, uint16_t value) {
  if (_addr < 1 || _addr > 247) {
    return false;
  }
  return _rtu.writeHoldingRegister(_addr, reg, value) == 0;
}

void DFRobot_LD010::openPort(uint32_t baud) {
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
  _sSerial->begin(baud);
#elif defined(ESP32)
  if (_rxPin != 0 || _txPin != 0) {
    _hSerial->begin(baud, SERIAL_8N1, _rxPin, _txPin);
  } else {
    _hSerial->begin(baud);
  }
#else
  (void)_rxPin;
  (void)_txPin;
  _hSerial->begin(baud);
#endif
}

uint32_t DFRobot_LD010::baudToBps(eBaudRate_t baud) {
  switch (baud) {
    case eBaud1200: return 1200;
    case eBaud2400: return 2400;
    case eBaud4800: return 4800;
    case eBaud9600: return 9600;
    case eBaud19200: return 19200;
    case eBaud38400: return 38400;
    case eBaud57600: return 57600;
    case eBaud115200: return 115200;
    default: return 0;
  }
}
