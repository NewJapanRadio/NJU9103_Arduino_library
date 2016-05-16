#include <NJU9103.h>

void setup() {
  Serial.begin(9600);
  NJU9103.begin();

  // reset NJU9103
  NJU9103.reset();

  // configuration
  uint8_t wd[3] = { 0x00, 0x00, 0x00 };
  wd[0] = PGACONF_PGA1EN_ENABLE | PGACONF_PGA1GAIN_X32;
  wd[1] = CLKCONF_CLKDIV_FOSC_2 | CLKCONF_OSR_256;
  wd[2] = DACCONF_CALDACEN_ENABLE | DACCONF_CALDAC_M16;
  NJU9103.write(NJU9103_ADDRESS_PGACONF, wd, 3);

  // zero point adjustment
  NJU9103.adjust_zero();

  Serial.println("Run NJU9103");
}

void loop() {
  int16_t adcdata;

  // start conversion & read ADC data
  NJU9103.get_adcdata(&adcdata);

  Serial.println(adcdata, DEC);
  delay(50);
}
