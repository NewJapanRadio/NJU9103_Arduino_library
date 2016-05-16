#include "NJU9103.h"

NJU9103Class NJU9103;

NJU9103Class::NJU9103Class(uint32_t clock) {
  this->spiSettings = SPISettings(clock, MSBFIRST, SPI_MODE1);
}

NJU9103Class::~NJU9103Class() {
}

void NJU9103Class::begin() {
  SPI.begin();
}

void NJU9103Class::end() {
  SPI.end();
}

void NJU9103Class::reset() {
  SPI.beginTransaction(spiSettings);
  SPI.transfer(0x7F);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.endTransaction();
  delayMicroseconds(1);
  this->wait_boot();
}

void NJU9103Class::read(Address address, uint8_t data[], size_t length) {
  uint8_t rw = 1;
  uint8_t bc = (length - 1);
  uint8_t spicmd = uint8_t(((rw << 1) + bc) << 2);

  SPI.beginTransaction(spiSettings);
  SPI.transfer((uint8_t)(address << 4) + spicmd);
  for (int i = 0; i < (int)length; i++) {
    data[i] = SPI.transfer(0x00);
  }
  SPI.endTransaction();
}

void NJU9103Class::write(Address address, uint8_t data[], size_t length) {
  uint8_t rw = 0;
  uint8_t bc = (length - 1);
  uint8_t spicmd = uint8_t(((rw << 1) + bc) << 2);

  SPI.beginTransaction(spiSettings);
  SPI.transfer((uint8_t)(address << 4) + spicmd);
  for (int i = 0; i < (int)length; i++) {
    SPI.transfer(data[i]);
  }
  SPI.endTransaction();
}

void NJU9103Class::get_adcdata(int16_t *adcdata, bool chop) {
  uint8_t rd[2] = { 0x00, 0x00 };

  // CHSEL:INP/INN, MODE:Single Conversion
  uint8_t mode;
  if (chop) {
    mode = CTRL_MODE_SINGLE_CHOP_CONVERSION;
  }
  else {
    mode = CTRL_MODE_SINGLE_CONVERSION;
  }
  uint8_t wd = CTRL_CHSEL_INP_INN | mode;
  this->write(NJU9103Class::CTRL, &wd, 1);

  // wait for end of conversion
  this->read(NJU9103Class::CTRL, rd, 1);
  while ((rd[0] & 0x80) == CTRL_RDYB_BUSY) {
    this->read(NJU9103Class::CTRL, rd, 1);
  }

  // read ADCDATA
  this->read(NJU9103Class::ADCDATA0, rd, 2);
  *adcdata = int16_t((rd[0] << 8) | rd[1]);
}

void NJU9103Class::adjust_zero() {
  uint8_t rd = 0x00;

  uint8_t wd = CTRL_CHSEL_INP_INN | CTRL_MODE_SYSTEM_OFFSET_CALIBRATION;
  this->write(NJU9103Class::CTRL, &wd, 1);

  // wait for end of calibration
  this->read(NJU9103Class::CTRL, &rd, 1);
  while ((rd & 0x80) == CTRL_RDYB_BUSY) {
    this->read(NJU9103Class::CTRL, &rd, 1);
  }

  // read ADCDATA for clearing RDYB
  this->read(NJU9103Class::ADCDATA0, &rd, 1);
}

void NJU9103Class::wait_boot() {
  uint8_t rd = 0x00;
  this->read(NJU9103Class::CTRL, &rd, 1);
  while ((rd & 0x0F) == CTRL_MODE_BOOT) {
    this->read(NJU9103Class::CTRL, &rd, 1);
  }
}
