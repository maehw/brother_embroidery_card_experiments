#ifndef FLASHCHIP_H
#define FLASHCHIP_H

#include <Arduino.h>
#include <array>

// MCU hardware pin assignment

// The three control lines: chip enable (#CE), output enable (#OE) and write enable (#WE)
#define CE 0
#define OE 2
#define WE 22

// The eight data bus lines
#define Q0 1
#define Q1 23
#define Q2 5
#define Q3 19
#define Q4 18
#define Q5 7
#define Q6 39
#define Q7 26

// The nineteen address bus lines
#define A0  38
#define A1  3
#define A2  21
#define A3  4
#define A4  20
#define A5  17
#define A6  16
#define A7  9
#define A8  15
#define A9  14
#define A10 11
#define A11 12
#define A12 24
#define A13 25
#define A14 36
#define A15 28
#define A16 37
#define A17 27
#define A18 10

// Flash capacity in bytes: 2^19=524288 bytes=512 kBytes or 4 Mbit
#define FLASH_CHIP_CAPACITY 524288


class FlashChip {
 private:
  int ceState = -1;
  int oeState = -1;
  int weState = -1;
  int dataState = -1;

  // Vendor specific command sequences; these values are for Microchip
  const uint32_t CMD_UNLOCK1_ADDR = 0x5555;
  const uint8_t CMD_UNLOCK1_DATA = 0xAA;
  const uint32_t CMD_UNLOCK2_ADDR = 0x2AAA;
  const uint8_t CMD_UNLOCK2_DATA = 0x55;
  const uint32_t CMD_PROGRAM_ADDR = 0x5555;
  const uint8_t CMD_PROGRAM_DATA = 0xA0;
  const uint32_t CMD_ERASE_ADDR = 0x5555;
  const uint8_t CMD_ERASE_DATA = 0x80;
  const uint32_t CMD_CHIP_ERASE_ADDR = 0x5555;
  const uint8_t CMD_CHIP_ERASE_DATA = 0x10;
  const uint32_t CMD_ENTER_SWID_ADDR = 0x5555;
  const uint8_t CMD_ENTER_SWID_DATA = 0x90;
  const uint32_t CMD_EXIT_SWID_ADDR = 0x5555;
  const uint8_t CMD_EXIT_SWID_DATA = 0xF0;
  const uint32_t MANUFACTURER_ID_ADDR = 0x0001;
  const uint32_t DEVICE_ID_ADDR = 0x0002;
  
 public:
  const uint8_t DATA_PINS_SIZE = 8;
  const uint8_t ADDRESS_PINS_SIZE = 19;
  const uint8_t MANUFACTURER_ID_MICROCHIP = 0xBF;
  const uint8_t DEVICE_ID_MICROCHIP_SST39SF040 = 0xB7;

  void init();
  const uint8_t data_pins[DATA_PINS_SIZE] = {Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7};
  const uint8_t address_pins[ADDRESS_PINS_SIZE] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18};

  void setAddressLines(uint32_t addr);
  void setDataLines(uint8_t dat);
  void sendCmd(uint32_t addr, uint8_t dat);

  void writeOneByte(uint32_t addr, uint8_t dat);
  uint8_t readOneByte(uint32_t addr);

  void eraseAll();
  std::array<uint8_t,2> serialVersion();

  void DATA_MODE(uint8_t mod);
};
#endif
