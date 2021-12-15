//#define DEBUG_PRINT_ROM_END

#define SHIFT_DATA 2
#define SHIFT_CLK 4
#define SHIFT_LATCH 3

#define DATA0 5
#define DATA7 12
#define EEPROM_WE 13
#define RAM_WE A5

#define CLK_RATE A4
#define RUN_CLK A3
#define PROG_EEPROM A2
#define STEP_CLK A1
#define CLK A0

const float MAX_CLK_RATE = 2000.0;
const unsigned long DEBOUNCE_DELAY = 50;
const byte SHIFT_DELAY = 1;

const int DATA_PINS[] = {5, 6, 7, 8, 9, 10, 11, 12};

// EEPROM //////////////////////////////////////////////////////////////////////

bool prog_eeprom = false;
enum prog_state_enum {
  PS_NONE,
  PS_WAIT_SIZE,
  PS_WAIT_ORG,
  PS_WAIT_DATA,
  PS_DONE
};
prog_state_enum prog_state;

void switch_prog_eeprom() {
  Serial.println("Switching to EEPROM Programming Mode");
  Serial.println("");

  digitalWrite(SHIFT_DATA, LOW);
  pinMode(SHIFT_DATA, OUTPUT);
  digitalWrite(SHIFT_CLK, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(EEPROM_WE, HIGH);
  digitalWrite(CLK, HIGH);
  setDataBusMode(OUTPUT);

  prog_state = PS_NONE;
}

void enableWrite()      { digitalWrite(EEPROM_WE, LOW);}
void disableWrite()     { digitalWrite(EEPROM_WE, HIGH);}
void waitWriteCycle()   { delay(10); }

// Output the address bits and outputEnable signal using shift registers.
void setAddress(int addr, bool outputEnable) {
    // Set the highest bit as the output enable bit (active low)
    if (outputEnable) {
        addr &= ~0x8000;
    } else {
        addr |= 0x8000;
    }
    byte dataMask = 0x04;
    byte clkMask = 0x10;
    byte latchMask = 0x08;

    // Make sure the clock is low to start.
    PORTD &= ~clkMask;

    // Shift 16 bits in, starting with the MSB.
    for (uint16_t ix = 0; (ix < 16); ix++)
    {
        // Set the data bit
        if (addr & 0x8000)
        {
            PORTD |= dataMask;
        }
        else
        {
            PORTD &= ~dataMask;
        }

        // Toggle the clock high then low
        PORTD |= clkMask;
        delayMicroseconds(3);
        PORTD &= ~clkMask;
        addr <<= 1;
    }

    // Latch the shift register contents into the output register.
    PORTD &= ~latchMask;
    delayMicroseconds(1);
    PORTD |= latchMask;
    delayMicroseconds(1);
    PORTD &= ~latchMask;
}

// Set the I/O state of the data bus.
// The 8 bits data bus are is on pins D5..D12.
void setDataBusMode(uint8_t mode) {
    // On the Uno and Nano, D5..D12 maps to the upper 3 bits of port D and the
    // lower 5 bits of port B.
    if (mode == OUTPUT) {
        DDRB |= 0x1f;
        DDRD |= 0xe0;
    } else {
        DDRB &= 0xe0;
        DDRD &= 0x1f;
    }
}

// Read a byte from the data bus.  The caller must set the bus to input_mode
// before calling this or no useful data will be returned.
byte readDataBus() {
    return (PINB << 3) | (PIND >> 5);
}

// Write a byte to the data bus.  The caller must set the bus to output_mode
// before calling this or no data will be written.
void writeDataBus(byte data) {
     PORTB = (PORTB & 0xe0) | (data >> 3);
     PORTD = (PORTD & 0x1f) | (data << 5);
}

// Read a byte from the EEPROM at the specified address.
byte readEEPROM(int address) {
    setDataBusMode(INPUT);
    setAddress(address, /*outputEnable*/ true);
    return readDataBus();
}

// Write a byte to the EEPROM at the specified address.
void writeEEPROM(int address, byte data) {
    setAddress(address, /*outputEnable*/ false);
    setDataBusMode(OUTPUT);
    writeDataBus(data);
    enableWrite();
    delayMicroseconds(1);
    disableWrite();
    delayMicroseconds(1);
}

// Set an address and data value and toggle the write control.  This is used
// to write control sequences, like the software write protect.  This is not a
// complete byte write function because it does not set the chip enable or the
// mode of the data bus.
void setByte(byte value, word address) {
    setAddress(address, false);
    writeDataBus(value);

    delayMicroseconds(1);
    enableWrite();
    delayMicroseconds(1);
    disableWrite();
}

// Write the special six-byte code to turn off Software Data Protection.
void disableSoftwareWriteProtect() {
    disableWrite();
    setDataBusMode(OUTPUT);

    setByte(0xaa, 0x5555);
    setByte(0x55, 0x2aaa);
    setByte(0x80, 0x5555);
    setByte(0xaa, 0x5555);
    setByte(0x55, 0x2aaa);
    setByte(0x20, 0x5555);

    setDataBusMode(INPUT);
    waitWriteCycle();
}

// Write the special three-byte code to turn on Software Data Protection.
void enableSoftwareWriteProtect() {
    disableWrite();
    setDataBusMode(OUTPUT);

    setByte(0xaa, 0x5555);
    setByte(0x55, 0x2aaa);
    setByte(0xa0, 0x5555);

    setDataBusMode(INPUT);
    waitWriteCycle();
}

word rom_size;
word rom_org;

#ifdef DEBUG_PRINT_ROM_END
byte rom_debug[600];
#endif

bool waitSerial() {
  long timeout = millis() + 1000;
  while(Serial.available() <= 0 && millis() <  timeout) {}
  return Serial.available();
}

void program() {
  
  disableSoftwareWriteProtect();

  word pos = rom_org;
  // Watch out for overflow 0xff00 + 0x0100 == 0 so
  //   pos < rom_org + rom_size
  //   pos < 0xff00 + 0x100
  //   pos < 0
  // will fail to execute, hence we have to do
  //   pos <= rom_org + (rom_size - 1)
  //   pos <= 0xff00 + 0xff
  //   pos <= 0xffff
  while(pos <= rom_org + (rom_size - 1)) {
    if (!waitSerial()) { prog_state = PS_NONE; return; }
    byte b = Serial.read();

    if (pos % 0x40 == 0) {
      waitWriteCycle(); // Crossing 28c256 page boundary, must wait 10ms for write cycle to complete
    }
    
    #ifdef DEBUG_PRINT_ROM_END
      rom_debug[pos % 600] = b;
    #endif
    
    writeEEPROM(pos, b);
    pos += 1;
  }
  waitWriteCycle(); // Allow final write cycle to complete
  setAddress(pos, false);
  enableSoftwareWriteProtect();
  prog_state = PS_DONE;
}

void prog_eeprom_loop() {
  switch (prog_state) {
    case PS_NONE: {
      Serial.println("Waiting for binary image beginning with length WORD - little-endian please");
      prog_state = PS_WAIT_SIZE;
      break;
    }
    case PS_WAIT_SIZE: {
      if (!waitSerial()) { prog_state = PS_NONE; return; }
      byte low = Serial.read();
      if (!waitSerial()) { prog_state = PS_NONE; return; }
      byte high = Serial.read();
      rom_size = (high << 8) | low;
      prog_state = PS_WAIT_ORG;
      break;
    }
    case PS_WAIT_ORG: {
      if (!waitSerial()) { prog_state = PS_NONE; return; }
      byte low = Serial.read();
      if (!waitSerial()) { prog_state = PS_NONE; return; }
      byte high = Serial.read();
      rom_org = (high << 8) | low;
      prog_state = PS_WAIT_DATA;
      break;
    }
    case PS_WAIT_DATA:{
      program();
      break;
    }
  }
}

// CLOCK ///////////////////////////////////////////////////////////////////////

unsigned long lastClk = millis();
unsigned long clk_rate = 1000;
bool run_clk = true;
unsigned long run_clk_debounce = 0;
bool step_pressed = false;
bool clk_rising = false;

// void set_clk(int value) {
//   clk_rising = (value && !clk_state);
//   clk_state = value;
//   digitalWrite(CLK, clk_state ? HIGH : LOW);
// }

void clk_pulse() {
  digitalWrite(CLK, LOW);
  delayMicroseconds(1);
  digitalWrite(CLK, HIGH);
  clk_rising = true;
}

char debugstr[80];
void manual_clk() {
  bool next_pressed = (digitalRead(STEP_CLK) == HIGH);
  unsigned long next_debounce = millis();
  if (next_debounce - run_clk_debounce < DEBOUNCE_DELAY) {
     return;
  }
  
  if (next_pressed != step_pressed) {
    run_clk_debounce = next_debounce;
    if (next_pressed && !step_pressed) {
      clk_pulse();
    }
    step_pressed = next_pressed;
  
    #ifdef DEBUG_PRINT_ROM_END
    for(int i=0; i<600;i++) {
      Serial.print(rom_debug[i], HEX); Serial.print(" ");
      if (i % 20 == 0) {
        Serial.println("");
      }
    }
    #endif
  }
}

void auto_clk() {
  clk_rate = (unsigned long)(analogRead(CLK_RATE) / 1024.0 * MAX_CLK_RATE);
  
  unsigned long now = millis();
  if (now - lastClk > clk_rate) {
    lastClk = now;
    clk_pulse();
  }
}

void clk_loop() {
  clk_rising = false;
  
  run_clk = (digitalRead(RUN_CLK) == HIGH);
  
  if (!run_clk) {
    manual_clk();
  } else {
    auto_clk();
  }
}

// INIT ////////////////////////////////////////////////////////////////////////

//TODO This function is for testing purposes only
// Read the first 256 byte block of the EEPROM and dump it to the serial monitor.
void printContents() {
    //for (int base = 0xff00; (base < 0xffff); base += 16) {
    word start = 0xff00;
    for (int i = 0; i < 16; i++) {
        word base = start+i*16;
        byte data[16];
        for (int offset = 0; offset <= 15; offset += 1) {
            data[offset] = readEEPROM(base + offset);
        }

        char buf[80];
        sprintf(buf, "%04x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        Serial.println(buf);
    }
}

void setup() {
  digitalWrite(SHIFT_DATA, LOW);
  digitalWrite(SHIFT_CLK, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(EEPROM_WE, HIGH);
  digitalWrite(RAM_WE, HIGH);
  digitalWrite(CLK, HIGH);
  
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(EEPROM_WE, OUTPUT);
  pinMode(RAM_WE, INPUT);
  pinMode(CLK, OUTPUT);
    
  pinMode(CLK_RATE, INPUT);
  pinMode(RUN_CLK, INPUT);
  pinMode(PROG_EEPROM, INPUT);
  pinMode(STEP_CLK, INPUT);

  setAddress(0x0000, true);

  Serial.begin(115200);
  Serial.println("Gareth's 6502 utility woop woop!");

  printContents();
}

// DEBUGGER ////////////////////////////////////////////////////////////////////

void switch_debugger() {
  Serial.println("Switching to Debugger Mode");
  Serial.println("");

  digitalWrite(SHIFT_DATA, LOW);
  pinMode(SHIFT_DATA, INPUT);
  digitalWrite(SHIFT_CLK, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(EEPROM_WE, HIGH);
  digitalWrite(CLK, HIGH);
  setDataBusMode(INPUT);
}

byte readData() {
  byte data = 0;
  for (int i = DATA7; i >= DATA0; i--) {
    data = data << 1;
    data |= (digitalRead(i) == HIGH ? 1 : 0);
  }
  return data;
}

byte readByte() {
  byte result = 0;
  for (int i = 0; i < 8; i++) {
    word in = ((digitalRead(SHIFT_DATA) == HIGH) ? 1 : 0);
    result = result << 1;
    result |= in;

    digitalWrite(SHIFT_CLK, HIGH);
    delayMicroseconds(SHIFT_DELAY);
    digitalWrite(SHIFT_CLK, LOW);
  }
  return result;
}

word readWord() {
  digitalWrite(SHIFT_LATCH, LOW);
  delayMicroseconds(SHIFT_DELAY);
  digitalWrite(SHIFT_LATCH, HIGH);
  delayMicroseconds(SHIFT_DELAY);

  // I wired the 74HC165 backwards so high comes out before low
  byte high = readByte();
  byte low = readByte();

  return (high << 8) | low;
}

word readAddress() {
  return readWord();
}

char buffer[80];
void debugger_loop() {
  if (!clk_rising) {
    return;
  }
  
  byte data = readData();
  word address = readAddress();
  byte we = (digitalRead(RAM_WE) == LOW);

  sprintf(buffer, "%04x\t%02x\t%s", address, data, we ? "W" : "r");
  Serial.println(buffer);
}

// LOOP ////////////////////////////////////////////////////////////////////////

bool firstTime = true;
void mode() {
  bool next_prog_eeprom = (digitalRead(PROG_EEPROM) == LOW);
  if (next_prog_eeprom != prog_eeprom || firstTime) {
    firstTime = false;
    prog_eeprom = next_prog_eeprom;
    if (prog_eeprom) {
      switch_prog_eeprom();
    } else {
      switch_debugger();
    }
  }
}

void loop() {
  mode();
  if (prog_eeprom) {
    prog_eeprom_loop();
  } else {
    clk_loop();
    debugger_loop();
  }
}
