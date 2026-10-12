# DFRobot_LD010

- [English Version](./README.md)

60GHz 非接触雷达液位传感器，通过 RS485 的 Modbus RTU 输出。配合 Gravity 隔离型 RS485 转 UART 模块 DFR0845，可测量 0.2 m 到 10 m 的液位或料位。SKU SEN0778，模块型号 XKC-LD010-R7R1。

## 产品链接

SKU SEN0778

## 目录

- [概述](#概述)
- [库安装](#库安装)
- [方法](#方法)
- [兼容性](#兼容性)
- [历史](#历史)
- [创作者](#创作者)

## 概述

* 读取空高、液位高度和安装高度，单位毫米
* 设置从机地址、波特率、报警阈值和空罐距离
* 标定空罐，并恢复出厂通信参数
* 通过依赖库 DFRobot_RTU 收发 Modbus RTU，功能码 03H/06H，默认地址 1，9600 8N1
* RS485 经 DFR0845。传感器电源为 DC 9-28 V

## 库安装

1. 先在 Arduino 库管理器中安装 `DFRobot_RTU`。本库用它做 Modbus 组帧和 CRC。
2. 再搜索 `DFRobot_LD010` 安装，或把本目录放到 Arduino 的 `libraries` 目录。
3. 打开 `examples/readLiquidLevel`。

本包没有单独的产品照片。

## 方法

```C++
  /**
   * @fn begin
   * @brief 打开串口并确认从机有应答
   * @return 寄存器 0x0010 读回的地址与构造时一致时返回 true
   * @n 默认链路是 9600 8N1，从机地址 1。
   * @n begin 失败通常是 RS485 的 A/B 接反、传感器没有 9-28 V 电源，或地址/波特率已经被改过。
   */
  bool begin(void);

  /**
   * @fn setDeviceAddress
   * @brief 设置 Modbus 从机地址
   * @param address 新地址，1-247
   * @return 传感器回显写入时返回 true
   * @n 地址 0 是广播地址，会被拒绝。
   * @n 写入成功后，后续调用使用新地址。这一次回显仍使用旧地址。
   */
  bool setDeviceAddress(uint8_t address);

  /**
   * @fn getDeviceAddress
   * @brief 从寄存器 0x0010 读取从机地址
   * @param address 接收地址
   * @return 寄存器读取成功时返回 true
   * @n 这是直接读保持寄存器，不是通知缓存。
   */
  bool getDeviceAddress(uint8_t *address);

  /**
   * @fn setBaudRate
   * @brief 设置传感器波特率
   * @param baud 波特率线值，不是裸 bps
   * @n eBaud1200 = 1，1200 bps
   * @n eBaud2400 = 2，2400 bps
   * @n eBaud4800 = 3，4800 bps
   * @n eBaud9600 = 4，9600 bps，出厂默认
   * @n eBaud19200 = 5，19200 bps
   * @n eBaud38400 = 6，38400 bps
   * @n eBaud57600 = 7，57600 bps
   * @n eBaud115200 = 8，115200 bps
   * @return 传感器回显写入时返回 true
   * @n 回显在旧波特率上返回。本方法随后等待，并按新波特率重新打开本地串口。
   */
  bool setBaudRate(eBaudRate_t baud);

  /**
   * @fn getBaudRate
   * @brief 从寄存器 0x0015 读取波特率线值
   * @param baud 接收 1-8 的线值
   * @return 读到八个定义值之一时返回 true
   * @n 返回的是线上的编号。4 表示 9600 bps，不是从 0 开始的第 4 项。
   */
  bool getBaudRate(eBaudRate_t *baud);

  /**
   * @fn setLiquidAlarm
   * @brief 设置液位高度报警阈值
   * @param thresholdMm 阈值，单位毫米，0-10000
   * @return 传感器回显写入时返回 true
   * @n 液位高度大于等于该阈值时，状态 bit0 变为 1。
   * @n 阈值为 0 时，只要上报的液位高度大于等于 0 就满足报警条件。
   */
  bool setLiquidAlarm(uint16_t thresholdMm);

  /**
   * @fn getLiquidAlarm
   * @brief 读取液位高度报警阈值
   * @param thresholdMm 接收阈值，单位毫米
   * @return 寄存器读取成功时返回 true
   * @n 单位是 mm。这个函数不读取当前液位高度。
   */
  bool getLiquidAlarm(uint16_t *thresholdMm);

  /**
   * @fn setInstallationHeight
   * @brief 写入空罐距离，也就是安装高度
   * @param heightMm 距离，单位毫米，20-10000
   * @return 传感器回显写入时返回 true
   * @n 超出 20-10000 mm 的值会被拒绝，不会发出去。
   * @n 这是传感器到罐底的距离，不是液位高度。
   */
  bool setInstallationHeight(uint16_t heightMm);

  /**
   * @fn getInstallationHeight
   * @brief 读取空罐距离
   * @param heightMm 接收安装高度，单位毫米
   * @return 寄存器读取成功时返回 true
   * @n 单位是 mm。通信失败返回 false，不会用 0 表示失败。
   */
  bool getInstallationHeight(uint16_t *heightMm);

  /**
   * @fn calibrateEmptyDistance
   * @brief 按当前看到的距离开始标定空罐
   * @return 传感器回显写入 1 时返回 true
   * @n 返回值只表示命令被接受。
   * @n 之后读 getSystemStatus()。bit1 = 0 表示标定成功，bit1 = 1 表示未成功。
   * @n 罐里有液体时不要标定，当前距离会被记成空罐距离。
   * @n 手册没有给出标定耗时。
   */
  bool calibrateEmptyDistance(void);

  /**
   * @fn getEmptyHeight
   * @brief 读取传感器到液面的距离
   * @param heightMm 接收距离，单位毫米
   * @return 读到寄存器 0x0011 时返回 true
   * @n 0 到 200 mm 在近距盲区，读数不可信。
   * @n 大于 10000 mm 超出远距上限，读数不可信。
   * @n 对空时常常返回 0。这个 0 是传感器读数，不是通信失败。
   */
  bool getEmptyHeight(uint16_t *heightMm);

  /**
   * @fn getLiquidHeight
   * @brief 读取传感器算出的液位高度
   * @param heightMm 接收液位高度，单位毫米
   * @return 读到寄存器 0x001B 时返回 true
   * @n 传感器按空罐距离换算。本库自己不做减法。
   * @n 通信失败返回 false。读到 0 mm 仍然算读取成功。
   */
  bool getLiquidHeight(uint16_t *heightMm);

  /**
   * @fn restoreFactory
   * @brief 恢复出厂通信设置
   * @return 传感器回显写入 1 时返回 true
   * @n 手册写明的默认值：从机地址 1，上传方式 0，波特率编号 4（9600）。
   * @n 手册没有给出空罐距离和报警阈值的出厂值。
   * @n 回显在旧地址和旧波特率上返回。本方法随后等待，并把串口重新打开为地址 1、9600 8N1。
   */
  bool restoreFactory(void);

  /**
   * @fn setUploadMode
   * @brief 选择传感器是否主动推送距离
   * @param mode 上传方式
   * @n eUploadOff = 0，不主动上传，出厂默认
   * @n eUploadOn = 1，主动上传。多只传感器级联时必须关闭。
   * @return 传感器回显写入时返回 true
   * @n 手册没有定义主动上传帧。距离一律用功能码 03H 读取。
   */
  bool setUploadMode(eUploadMode_t mode);

  /**
   * @fn getUploadMode
   * @brief 读取距离上传方式
   * @param mode 接收 0 或 1
   * @return 寄存器值为 0 或 1 时返回 true
   * @n 0 表示轮询。1 表示主动上传，级联时必须保持关闭。
   */
  bool getUploadMode(eUploadMode_t *mode);

  /**
   * @fn getSystemStatus
   * @brief 读取状态字
   * @param status 接收寄存器 0x0017
   * @return 寄存器读取成功时返回 true
   * @n Bit0 = 1：液位高度已达到报警阈值。
   * @n Bit1 = 0：标定成功。Bit1 = 1：标定未成功。
   * @n Bit2 = 0：开机初始化成功。Bit2 = 1：开机初始化未成功。
   * @n 其余位手册未定义。
   */
  bool getSystemStatus(uint16_t *status);
```

## 兼容性

| 主板 | 状态 | 备注 |
|------|------|------|
| Arduino UNO | 未测试 | SoftwareSerial，RX 4，TX 5 |
| Arduino Mega 2560 | 未测试 | UART3，RX 15，TX 14 |
| Arduino Leonardo | 未测试 | Serial1，RX 0，TX 1 |
| FireBeetle-ESP32 | 未测试 | Serial1，RX D3，TX D2 |
| Raspberry Pi 4B/5B | 不在本库 | 本次只交付 Arduino 库 |

## 历史

- 2026/10/12 - V1.0.0 版本

## 创作者

Written by wxzed(xiao.wu@dfrobot.com), 2026. (欢迎访问我们的[网站](https://www.dfrobot.com/))
