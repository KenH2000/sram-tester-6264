/***************************************************************************
 * Universal Static RAM Tester - Tests SRAM by writing and reading patterns
 * This is a modified version of the original testers:
 * derived from the 2114 SRAM tester by Carsten Skjerk
 * https://github.com/skjerk/Arduino-2114-SRAM-tester
 * ***AND****
 * (c) Dennis Marttinen 2022
 * Licensed under the MIT license
 * https://github.com/twelho/sram-tester
 ***************************************************************************
 * This version tests the 6264 SRAM
 * and adds options to test the full pattern and other (FASTER) patterns.
 * 
 * The pin arrangement is organized so a ZIF socket can be mounted  
 * on a stripboard and directly plugged in to the MEGA 36 pin connector. 
 * 6264 SRAM (ATMega 2560 pinout)
 * IC Pin 14 (Vss) is jumpered to GND (MEGA pin is set to INPUT)
 * IC Pin 20 (CS) set on MEGA to OUTPUT LOW
 * IC Pin 22 (OE) set on MEGA to OUTPUT LOW
 * IC Pin 28 (Vcc) is jumpered to VCC (MEGA pin is set to INPUT)
*/

struct WritePin {
  uint8_t pin;
  bool inverted;
};

// ------------------------------
//CS and OE configure to OUTPUT, LOW
const uint8_t gpins[] = { 39, 43 };

//direct wired to VCC and GND, configure as INPUT
const uint8_t ipins[] = { 52, 27 };

// Address Pins
const uint8_t addressPins[] = {
  44, 42, 40, 38, 36, 34, 32, 30, 33, 35, 41, 37, 28
};

// Data Pins
const uint8_t dataPins[] = {
  46, 48, 50, 53, 51, 49, 47, 45
};

// Write Pins
const WritePin writePins[] = {
  {29, true}, // (LOW = WRITE)
};

// The number of addressable memory locations
const uint32_t addressCount = 8192;
// ------------------------------

#define NX(x) (sizeof(x) / sizeof((x)[0]))

const size_t NA = NX(addressPins);
const size_t ND = NX(dataPins);
const size_t NW = NX(writePins);
const size_t NS = NX(ipins);
//int j;

// Set up special pins
void specialpins() {
  for (size_t i=0;i<NS;i++) {
    pinMode(gpins[i], OUTPUT);
    digitalWrite(gpins[i],LOW);
    pinMode(ipins[i],INPUT);
  }
}

// Set Address pins to output
void setupAddressPins() {
  for (size_t i = 0; i < NA; i++) {
    pinMode(addressPins[i], OUTPUT);
  }
}

// Set Data pins to output
void setDataPinsOutput() {
  for (size_t i = 0; i < ND; i++) {
    pinMode(dataPins[i], OUTPUT);
  }
}

// Set Data pins to input
void setDataPinsInput() {
  for (size_t i = 0; i < ND; i++) {
    // Explicitly set them LOW or else this reads its own output
    digitalWrite(dataPins[i], LOW);
    pinMode(dataPins[i], INPUT);
  }
}

// Set Write pins to output
void setupWritePins() {
  for (size_t i = 0; i < NW; i++) {
    pinMode(writePins[i].pin, OUTPUT);
  }
}

// Initial setup of pins and serial monitor
void setup() {
  // Initialize special pins
  specialpins();
  
  // Initialize all pins
  setupAddressPins();
  setDataPinsOutput();
  setupWritePins();
  
  // Initialize Serial Port
  Serial.begin(115200);
  Serial.println("Universal Static RAM Tester");
  Serial.print(NA);
  Serial.print('/');
  Serial.print(ND);
  Serial.print('/');
  Serial.print(NW);
  Serial.println(" address/data/write pin(s) configured");
}

// Set the address pins to match the specified address
void setAddressBits(size_t address) {
  for (size_t i = 0; i < NA; i++) {
    digitalWrite(addressPins[i], bitRead(address, i));
  }
}

// Set the data pins to match the specified value
void setDataBits(size_t value) {
  for (size_t i = 0; i < ND; i++) {
    digitalWrite(dataPins[i], bitRead(value, i));
  }
}

void enableWritePins() {
  for (size_t i = 0; i < NW; i++) {
    const WritePin* p = &writePins[i];
    digitalWrite(p->pin, !p->inverted);
  }
}

void disableWritePins() {
  for (size_t i = 0; i < NW; i++) {
    const WritePin* p = &writePins[i];
    digitalWrite(p->pin, p->inverted);
  }
}

// Write data to the specified memory address
void writeData(size_t address, size_t data) {
  // Set data pins to output
  setDataPinsOutput();
  
  // Set address bits
  setAddressBits(address);

  // Set data bits
  setDataBits(data);
  
  // Enable Write pins
  enableWritePins();

  // Wait for the logic to be stabilized
  //delay(1);
  
  // Disable Write pins
  disableWritePins();

  // Wait a bit for the write to commit
  //delay(1);
}

// Read data from the specified memory address
// Note that the Write pins must already be in READ mode
size_t readData(size_t address) {
  // Set data pins to input
  setDataPinsInput();
  
  // Set address bits
  setAddressBits(address);
  
  // Wait for the logic to be stabilized
  //delay(1);

  // Read each data bit one by one
  size_t result = 0;
  for (size_t i = 0; i < ND; i++) {
    bitWrite(result, i, digitalRead(dataPins[i]));
  }
  return result;
}

// Output binary value from the ND bit data
void printBinary(size_t data) {
  for (size_t b = ND; b > 0; b--) {
    Serial.print(bitRead(data, b - 1));
  }
}

// Print an unsigned 64-bit integer
void printU64(uint64_t value) {
  if (value == 0) {
      Serial.print('0');
      return;
  }
  
  unsigned char buf[20];
  uint8_t i = 0;
  
  while (value > 0) {
      uint64_t q = value/10;
      buf[i++] = value - q * 10;
      value = q;
  }
  
  for (; i > 0; i--) {
    Serial.print((char) ('0' + buf[i - 1]));
  }
}

void loop() {
  byte subcommand;
Serial.println("Choose Test Pattern:");
Serial.println("F = Full Test (SLOW)");
Serial.println("A = Alt  Test (FAST)");
Serial.println("1 = 1's (FAST)");
Serial.println("0 = 0's (FAST)");
Serial.println("Enter Test Pattern (F,A,1 or 0):");
 do
      {
      subcommand = toupper (Serial.read ());
      } while (subcommand != 'F' && subcommand != 'A' && subcommand != '1'  && subcommand != '0' );
    
    if (subcommand == 'F') {fulltest();}
    if (subcommand == 'A') {
      int patterns[]={0xAA,0x55,0x00,0xFF};
      int arrsz = sizeof(patterns) / sizeof((patterns)[0]);
      testpattern(patterns,arrsz);
    }
    if (subcommand == '1') {
      int patterns[]={0xFF};
      int arrsz = sizeof(patterns) / sizeof((patterns)[0]);
      testpattern(patterns,arrsz);
    }
    if (subcommand == '0') {
      int patterns[]={0x00};
      int arrsz = sizeof(patterns) / sizeof((patterns)[0]);
      testpattern(patterns,arrsz);
    }
}

void fulltest(){
  uint32_t firstError = 0;
  uint32_t lastError = 0;
  uint64_t errorCount = 0;
  
  // Use all possible values for data patterns
  for (size_t pattern = 0; pattern < bit(ND); pattern++) {
    Serial.print("Running test pattern ");
    printBinary(pattern);
    Serial.println();
    
    // Loop through all addresses in the SRAM
    for (uint32_t addr = 0; addr < addressCount; addr++) {
      // Write test pattern to the SRAM
      writeData(addr, pattern);
      
      // Read data from the SRAM
      size_t data = readData(addr);

      // Verify
      if (data != pattern) {
        lastError = addr;
        if (errorCount++ == 0) {
          firstError = addr;
        }
        
        Serial.print("Error at address 0x");
        Serial.print(addr, HEX);
        Serial.print(" - Got: ");
        printBinary(data);
        Serial.print(", expected: ");
        printBinary(pattern);
        Serial.println();
      }
    }
  }
  
  Serial.println("Test complete*****************");
  printU64(errorCount);
  Serial.print(" errors found (");
  Serial.print((100.f * errorCount) / ((float)bit(ND) * addressCount));
  Serial.println("% failed)");
  if (errorCount > 0) {
    Serial.print("Error span: 0x");
    Serial.print(firstError, HEX);
    Serial.print(" to 0x");
    Serial.println(lastError, HEX);
  }
  Serial.println("******************************");
}


void testpattern(int patterns[],int arrsz){
  uint32_t firstError = 0;
  uint32_t lastError = 0;
  uint64_t errorCount = 0;
  // Use values for patterns array
   for (size_t i = 0; i < arrsz; i++) {
    Serial.print("Running test pattern ");
    printBinary(patterns[i]);
    Serial.println();
    
    // Loop through all addresses in the SRAM
    for (uint32_t addr = 0; addr < addressCount; addr++) {
      // Write test pattern to the SRAM
      writeData(addr, patterns[i]);
      
      // Read data from the SRAM
      size_t data = readData(addr);

      // Verify
      if (data != patterns[i]) {
        lastError = addr;
        if (errorCount++ == 0) {
          firstError = addr;
        }
        
        Serial.print("Error at address 0x");
        Serial.print(addr, HEX);
        Serial.print(" - Got: ");
        printBinary(data);
        Serial.print(", expected: ");
        printBinary(patterns[i]);
        Serial.println();
      }
    }
  }
  
  Serial.println("Test complete*****************");
  printU64(errorCount);
  Serial.print(" errors found (");
  Serial.print((100.f * errorCount) / ((float)bit(ND) * addressCount));
  Serial.println("% failed)");
  if (errorCount > 0) {
    Serial.print("Error span: 0x");
    Serial.print(firstError, HEX);
    Serial.print(" to 0x");
    Serial.println(lastError, HEX);
  }
  Serial.println("******************************");
}

