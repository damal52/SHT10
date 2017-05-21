uint8_t getByte1(bool ack) {
  uint8_t i;
  uint16_t result = 0;
  for (i = 14; i > 0; i--) {
    result <<= 1;                   // Shift received bits towards MSB
    digitalWrite(_pinClock0, HIGH);  // Generate clock pulse
    PULSE_SHORT;
    result |= digitalRead(_pinData1);  // Merge next bit into LSB position
    digitalWrite(_pinClock0, LOW);
    PULSE_SHORT;
  }
  pinMode(_pinData1, OUTPUT);
  digitalWrite(_pinData1, !ack);     // Assert ACK ('0') if ack == 1
  PULSE_SHORT;
  digitalWrite(_pinClock0, HIGH);    // Clock #9 for ACK / noACK
  PULSE_LONG;
  digitalWrite(_pinClock0, LOW);     // Finish with clock in low state
  PULSE_SHORT;
  pinMode(_pinData1, INPUT_PULLUP);         // Return data line to input mode
#ifdef DATA_PU
  digitalWrite(_pinData1, DATA_PU);  // Restore internal pullup state
#endif
  Serial.println(result); // 
  return result;
}

// Write byte to sensor and check for acknowledge
uint8_t putByte1(uint8_t value) {
  uint8_t mask, i;
  uint8_t error = 0;
  pinMode(_pinData1, OUTPUT);        // Set data line to output mode
  mask = 0x80;                      // Bit mask to transmit MSB first
  for (i = 8; i > 0; i--) {
    digitalWrite(_pinData1, value & mask);
    PULSE_SHORT;
    digitalWrite(_pinClock0, HIGH);  // Generate clock pulse
    PULSE_LONG;
    digitalWrite(_pinClock0, LOW);
    PULSE_SHORT;
    mask >>= 1;                     // Shift mask for next data bit
  }
  pinMode(_pinData1, INPUT_PULLUP);         // Return data line to input mode
#ifdef DATA_PU
  digitalWrite(_pinData1, DATA_PU);  // Restore internal pullup state
#endif
  digitalWrite(_pinClock0, HIGH);    // Clock #9 for ACK
  PULSE_LONG;
  if (digitalRead(_pinData1))        // Verify ACK ('0') received from sensor
    error = S_Err_NoACK;
  PULSE_SHORT;
  digitalWrite(_pinClock0, LOW);     // Finish with clock in low state
  return error;
}

// Communication link reset
// At least 9 SCK cycles with DATA=1, followed by transmission start sequence
//      ______________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
void resetConnection1(void) {
  uint8_t i;
  digitalWrite(_pinData1, HIGH);  // Set data register high before turning on
  pinMode(_pinData1, OUTPUT);     // output driver (avoid possible low pulse)
  PULSE_LONG;
  for (i = 0; i < 9; i++) {
    digitalWrite(_pinClock0, HIGH);
    PULSE_LONG;
    digitalWrite(_pinClock0, LOW);
    PULSE_LONG;
  }
  startTransmission1();
}

// Generate Sensirion-specific transmission start sequence
// This is where Sensirion does not conform to the I2C standard and is
// the main reason why the AVR TWI hardware support can not be used.
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
void startTransmission1(void) {
  digitalWrite(_pinData1, HIGH);  // Set data register high before turning on
  pinMode(_pinData1, OUTPUT);     // output driver (avoid possible low pulse)
  PULSE_SHORT;
  digitalWrite(_pinClock0, HIGH);
  PULSE_SHORT;
  digitalWrite(_pinData1, LOW);
  PULSE_SHORT;
  digitalWrite(_pinClock0, LOW);
  PULSE_LONG;
  digitalWrite(_pinClock0, HIGH);
  PULSE_SHORT;
  digitalWrite(_pinData1, HIGH);
  PULSE_SHORT;
  digitalWrite(_pinClock0, LOW);
  PULSE_SHORT;
  // Unnecessary here since putByte always follows startTransmission
//  pinMode(_pinData1, INPUT);
}
