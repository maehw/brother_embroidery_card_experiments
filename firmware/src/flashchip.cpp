#include "flashchip.h"
// Note: This implementation targets the 4 Mbit (x8) Microchip SST39SF040 flash IC

void FlashChip::init() {
  pinMode(CE, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(WE, OUTPUT);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A7, OUTPUT);
  pinMode(A8, OUTPUT);
  pinMode(A9, OUTPUT);
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(A12, OUTPUT);
  pinMode(A13, OUTPUT);
  pinMode(A14, OUTPUT);
  pinMode(A15, OUTPUT);
  pinMode(A16, OUTPUT);
  pinMode(A17, OUTPUT);
  pinMode(A18, OUTPUT);


  digitalWrite(CE, HIGH);
  digitalWrite(WE, HIGH);
  digitalWrite(OE, HIGH);
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  digitalWrite(A6, LOW);
  digitalWrite(A7, LOW);
  digitalWrite(A8, LOW);
  digitalWrite(A9, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A11, LOW);
  digitalWrite(A12, LOW);
  digitalWrite(A13, LOW);
  digitalWrite(A14, LOW);
  digitalWrite(A15, LOW);
  digitalWrite(A16, LOW);
  digitalWrite(A17, LOW);
  digitalWrite(A18, LOW);

  // Configure data bus to input mode
  DATA_MODE(INPUT);
}

void FlashChip::DATA_MODE(uint8_t mod) {                        
  if (dataState != mod) {
    dataState = mod;     
    for (uint8_t addr = 0; addr < DATA_ADDRESSES_SIZE; addr++) {
      pinMode(data_addresses[addr], mod);                       
    }                                                           
  }                                                             
}

void FlashChip::setDataLines(uint8_t data) {
  for (uint8_t bit = 0; bit < DATA_PINS_SIZE; bit++) {
    digitalWrite(data_addresses[bit], (data >> bit) & 1);
  }
}

void FlashChip::setAddressLines(uint32_t addr) {
  for (uint8_t bit = 0; bit < ADDRESS_PINS_SIZE; bit++) {
    digitalWrite(address_pins[bit], (data >> bit) & 1);
  }
}

void FlashChip::sendCmd(uint32_t addr, uint8_t dat) {
  // "A command is written by asserting WE# low while keeping CE# low. The address bus is latched 
  //  on the falling edge of WE# or CE#, whichever occurs last. The data bus is latched on the 
  //  rising edge of WE# or CE#, whichever occurs first."

  DATA_MODE(OUTPUT);
  setAddressLines(addr);
  setDataLines(dat);

  digitalWrite(CE, LOW);
  digitalWrite(WE, LOW);
  delayNanoseconds(1); // TODO: fix this based on actual values
  // Note: The "Byte-Program Time" T_BP is typ. 14 us / max. 20 us
  digitalWrite(WE, HIGH);
  digitalWrite(CE, HIGH);
  DATA_MODE(INPUT);
}

void FlashChip::writeOneByte(uint32_t addr, uint8_t dat) {
  if (addr >= FLASH_CHIP_CAPACITY) {
    Serial.println("Address exceeds flash chip capacity");
    return;
  }

  // Send "Software Command Sequence" for "Byte-Program" (four bus cycles)
  sendCmd(CMD_UNLOCK1_ADDR, CMD_UNLOCK1_DATA);
  sendCmd(CMD_UNLOCK2_ADDR, CMD_UNLOCK2_DATA);
  sendCmd(CMD_PROGRAM_ADDR, CMD_PROGRAM_DATA);
  sendCmd(addr, dat);

  uint8_t old_val;
  uint8_t val;
  while (true) {
    old_val = readOneByte(addr);
    val = readOneByte(addr);

    // Busy-wait using toggle bit method:
    // "During the internal Program or Erase operation, any consecutive attempts to read DQ6 will 
    //  produce alternating 0s and 1s, i.e., toggling between 0 and 1. When the internal Program 
    //  or Erase operation is completed, the toggling will stop. The device is then ready for the 
    //  next operation."
    if (((val >> 6) & 0x01) == ((old_val >> 6) & 0x01))
      break;
  }
}

uint8_t FlashChip::readOneByte(uint32_t addr) {
  if (addr >= FLASH_CHIP_CAPACITY) {
    Serial.println("Address exceeds flash chip capacity");
    return 0;
  }

  DATA_MODE(INPUT);
  setAddressLines(addr);
  digitalWrite(CE, LOW);
  digitalWrite(OE, LOW);

  uint8_t dat = 0;
  for (uint8_t bit = 0; bit < DATA_PINS_SIZE; bit++) {
    dat |= digitalRead(data_addresses[bit]) << bit;
    delayNanoseconds(50); // Why?!
  }

  digitalWrite(CE, HIGH);
  digitalWrite(OE, HIGH);

  return dat;
}

void FlashChip::eraseAll() {
  // Send "Software Command Sequence" for "Chip-Erase" (six bus cycles)
  sendCmd(CMD_UNLOCK1_ADDR, CMD_UNLOCK1_DATA);
  sendCmd(CMD_UNLOCK2_ADDR, CMD_UNLOCK2_DATA);
  sendCmd(CMD_ERASE_ADDR, CMD_ERASE_DATA);
  sendCmd(CMD_UNLOCK1_ADDR, CMD_UNLOCK1_DATA);
  sendCmd(CMD_UNLOCK2_ADDR, CMD_UNLOCK2_DATA);
  sendCmd(CMD_CHIP_ERASE_ADDR, CMD_CHIP_ERASE_DATA);
  delay(200);
}

std::array<uint8_t,2> FlashChip::serialVersion() {
  // Send "Software Command Sequence" for "Software ID Entry" (three bus cycles)
  sendCmd(CMD_UNLOCK1_ADDR, CMD_UNLOCK1_DATA);
  sendCmd(CMD_UNLOCK2_ADDR, CMD_UNLOCK2_DATA);
  sendCmd(CMD_ENTER_SWID_ADDR, CMD_ENTER_SWID_DATA);

  std::array<uint8_t, 2> version;
  version[0] = readOneByte(MANUFACTURER_ID_ADDR);
  version[1] = readOneByte(DEVICE_ID_ADDR);

  // Send "Software Command Sequence" for "Software ID Exit" (three bus cycles)
  sendCmd(CMD_UNLOCK1_ADDR, CMD_UNLOCK1_DATA);
  sendCmd(CMD_UNLOCK2_ADDR, CMD_UNLOCK2_DATA);
  sendCmd(CMD_EXIT_SWID_ADDR, CMD_EXIT_SWID_DATA);

  return version;
}
