#include <LiquidCrystal_I2C.h>

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
 * The LCD version can be used without a PC.  An I2C LCD is connected to 
 * the SCL,SDL lines and 2 buttons (gobutton,nxtbutton) are used to select
 * and run the tests.  
 * 
 * The pin arrangement is organized so a ZIF socket can be mounted  
 * on a stripboard and directly plugged in to the MEGA 36 pin connector. 
 * 6264 SRAM (ATMega 2560 pinout)
 * IC Pin 14 (Vss) is jumpered to GND (MEGA pin is set to INPUT)
 * IC Pin 20 (CS) set on MEGA to OUTPUT LOW
 * IC Pin 22 (OE) set on MEGA to OUTPUT LOW
 * IC Pin 28 (Vcc) is jumpered to VCC (MEGA pin is set to INPUT)
*/

LiquidCrystal_I2C lcd(0x3f,16,2);

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

int ledpin=13;
int boardledpin[]={ 22, 24 }; //2 color LED on board
unsigned long previousMillis =0;
const long interval = 500;
int chipstatus = 0;             //0 no errors;1 errors;2 testing
boolean buttonpressed=false;
int pressdelay=700;                 //button press delay (ms)
unsigned long lastDebounceTime = 0;
int gobutton=8;
int nxtbutton=9;
int testno=1;
boolean doserial=false;
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
pinMode(ledpin,OUTPUT);
pinMode(boardledpin[0],OUTPUT);
pinMode(boardledpin[1],OUTPUT);
pinMode(nxtbutton,INPUT_PULLUP);
pinMode(gobutton,INPUT_PULLUP);
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

  // init lcd
  lcd.init();
  lcd.backlight();
  
  // Initialize all pins
  setupAddressPins();
  setDataPinsOutput();
  setupWritePins();
  
  // Initialize Serial Port
  if (doserial){
  Serial.begin(9600);
  Serial.println("Universal Static RAM Tester");
  Serial.print(NA);
  Serial.print('/');
  Serial.print(ND);
  Serial.print('/');
  Serial.print(NW);
  Serial.println(" address/data/write pin(s) configured");
  }
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
void lcdprintBinary(size_t data) {
  for (size_t b = ND; b > 0; b--) {
    lcd.print(bitRead(data, b - 1));
  }
}

// Output binary value from the ND bit data
void printBinary(size_t data) {
  for (size_t b = ND; b > 0; b--) {
    if (doserial){Serial.print(bitRead(data, b - 1));}
  }
}

// Print an unsigned 64-bit integer
void printU64(uint64_t value) {
  if (value == 0) {
      if (doserial){Serial.print('0');}
      return;
  }
  
  unsigned char buf[20];
  uint8_t i = 0;
  
  while (value > 0) {
      uint64_t q = value/10;
      buf[i++] = value - q * 10;
      value = q;
  }
  if (doserial){
    for (; i > 0; i--) {
    Serial.print((char) ('0' + buf[i - 1]));
    }
  }
}

void blink() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (chipstatus==0) { //no errors
      if (digitalRead(boardledpin[0])!=HIGH) {digitalWrite(boardledpin[0],HIGH);}
      if (digitalRead(boardledpin[1])!=LOW) {digitalWrite(boardledpin[1],LOW);}
      if (digitalRead(ledpin)!=HIGH) {digitalWrite(ledpin,HIGH);}
    }
    if (chipstatus==1) { //errors
      if (digitalRead(boardledpin[0])!=LOW) {digitalWrite(boardledpin[0],LOW);}
      digitalWrite(boardledpin[1],!digitalRead(boardledpin[1]));
      digitalWrite(ledpin,!digitalRead(ledpin));
    }
    if (chipstatus==2) { //testing
      if (digitalRead(boardledpin[0])==digitalRead(boardledpin[1])) {
        digitalWrite(boardledpin[0],!digitalRead(boardledpin[0]));
      }
      digitalWrite(boardledpin[0],!digitalRead(boardledpin[0]));
      digitalWrite(boardledpin[1],!digitalRead(boardledpin[1]));
      if (digitalRead(ledpin)!=LOW) {digitalWrite(ledpin,LOW);}
    }
    
  }
}

void loop() {
  byte subcommand;
  #define ELS(x)   (sizeof(x) / sizeof(x[0]))
  int patterns[]={0xAA,0x55,0x00,0xFF};
  int ans;
  printtest(testno);
  do{
    ans = checkButtons();
    blink();    
    if (ans==1){
      testno+=1;if (testno>4){testno=1;};
      printtest(testno);
      }
  } while (ans<2);
  
    if (testno==1) {fulltest();}
    if (testno==2) {testpattern(patterns,0,4);}
    if (testno==3) {testpattern(patterns,3,4);}
    if (testno==4) {testpattern(patterns,2,3);}

/*  
Serial.println("Choose Test Pattern:");
Serial.println("F = Full Test (SLOW)");
Serial.println("A = Alt  Test (FAST)");
Serial.println("1 = 1's (FAST)");
Serial.println("0 = 0's (FAST)");
Serial.println("Enter Test Pattern (F,A,1 or 0):");

 do
      {
      subcommand = toupper (Serial.read ());blink();
      } while (subcommand != 'F' && subcommand != 'A' && subcommand != '1'  && subcommand != '0' && subcommand != ' ' );          
    if (subcommand == 'F') {fulltest();}
    if (subcommand == 'A') {testpattern(patterns,0,4);}
    if (subcommand == '1') {testpattern(patterns,3,4);}
    if (subcommand == '0') {testpattern(patterns,2,3);}
*/
}

void printtest(int x){
  if (x==1){
  lcd.setCursor(0,0);
  lcd.print("Full Test     ->");
  lcd.setCursor(0,1);
  lcd.print("Next          ->");
  }
  if (x==2){
  lcd.setCursor(0,0);
  lcd.print("Alt Test      ->");
  lcd.setCursor(0,1);
  lcd.print("Next          ->");
  }
  if (x==3){
  lcd.setCursor(0,0);
  lcd.print("1's Test      ->");
  lcd.setCursor(0,1);
  lcd.print("Next          ->");
  }
  if (x==4){
  lcd.setCursor(0,0);
  lcd.print("0's Test      ->");
  lcd.setCursor(0,1);
  lcd.print("Next          ->");
  }
}

void fulltest(){
  chipstatus=2;
  lcd.clear();
  uint32_t firstError = 0;
  uint32_t lastError = 0;
  uint64_t errorCount = 0;
  
  // Use all possible values for data patterns
  for (size_t pattern = 0; pattern < bit(ND); pattern++) {
    if (doserial){
      Serial.print("Running test pattern ");
      printBinary(pattern);
      Serial.println();
    }    
    lcd.setCursor(0,0);
    lcd.print("Testing    End->");
    lcd.setCursor(0,1);
    lcdprintBinary(pattern);
    
    // Loop through all addresses in the SRAM
    for (uint32_t addr = 0; addr < addressCount; addr++) {
      blink();
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
        if (doserial){  
          Serial.print("Error at address 0x");
          Serial.print(addr, HEX);
          Serial.print(" - Got: ");
          printBinary(data);
          Serial.print(", expected: ");
          printBinary(pattern);
          Serial.println();
        }
        if (errorCount==1){lcd.setCursor(0, 0);lcd.print("                ");}
        lcd.setCursor(0, 0);
        lcd.print("Err 0x");
        lcd.setCursor(6,0);
        lcd.print(addr,HEX);
        lcd.setCursor(11,0);
        lcd.print("End->");
        if (checkButtons()>0){
          chipstatus=1;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(u64tostring(errorCount)+" Errors");
          lcd.setCursor(0,1);
          lcd.print("RAM FAILED    ->");
          do {blink();} while (checkButtons()==0);
          do {} while (checkButtons()!=0);
          return;
        }
      }
      if (checkButtons()!=0) {
          if (errorCount>0){
            chipstatus=1;
          }else{
            chipstatus=0;
          }
          return;
      }
    }
  }
  lcd.clear();
  if (doserial){
    Serial.println("Test complete*****************");
    printU64(errorCount);  
    Serial.print(" errors found (");
    Serial.print((100.f * errorCount) / ((float)bit(ND) * addressCount));
    Serial.println("% failed)");
  }
  if (errorCount > 0) {
    chipstatus=1;
    lcd.setCursor(0,0);
    lcd.print(u64tostring(errorCount)+" Errors");
    lcd.setCursor(0,1);
    lcd.print("RAM FAILED    ->");
    if (doserial){
      Serial.print("Errors: 0x");
      Serial.print(firstError, HEX);
      Serial.print(" to 0x");
      Serial.println(lastError, HEX);
      Serial.println("----------RAM FAILED----------");
    }
  }else{
    if (doserial){Serial.println("----------RAM PASSED----------");}
    lcd.setCursor(0,0);
    lcd.print("0 Errors");
    lcd.setCursor(0,1);
    lcd.print("RAM PASSED    ->");
    chipstatus=0;
  }
  if (doserial){Serial.println("******************************");}
  do {blink();} while (checkButtons()==0);
  do {} while (checkButtons()!=0);
}

void testpattern(int patterns[],int a, int b){
  chipstatus=2;
  lcd.clear();
  uint32_t firstError = 0;
  uint32_t lastError = 0;
  uint64_t errorCount = 0;
  // Use values for patterns array
   for (size_t i = a; i < b; i++) {
     if (doserial){  
       Serial.print("Running test pattern ");
       printBinary(patterns[i]);
       Serial.println();
     }
    lcd.setCursor(0,0);
    lcd.print("Testing         ");
    lcd.setCursor(0,1);
    lcdprintBinary(patterns[i]);
    
    // Loop through all addresses in the SRAM
    for (uint32_t addr = 0; addr < addressCount; addr++) {
      blink();
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
        if (doserial){  
        Serial.print("Error at address 0x");
        Serial.print(addr, HEX);
        Serial.print(" - Got: ");
        printBinary(data);
        Serial.print(", expected: ");
        printBinary(patterns[i]);
        Serial.println();
        }
        if (errorCount==1){lcd.setCursor(0, 0);lcd.print("                ");}
        lcd.setCursor(0, 0);
        lcd.print("Err 0x");
        lcd.setCursor(6,0);
        lcd.print(addr,HEX);
        lcd.setCursor(11,0);
        lcd.print("End->");
        if (checkButtons()>0){
          chipstatus=1;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(u64tostring(errorCount)+" Errors");
          lcd.setCursor(0,1);
          lcd.print("RAM FAILED    ->");
          do {blink();} while (checkButtons()==0);
          do {} while (checkButtons()!=0);
          return;
        }
      }
    }
  }
lcd.clear();  
    if (doserial){
      Serial.println("Test complete*****************");  
      printU64(errorCount);
      Serial.println(" errors found.");
      //Serial.print((100.f * errorCount) / (100.f * addressCount));
    }  
  if (errorCount > 0) {
    chipstatus=1;
    lcd.setCursor(0,0);
    lcd.print(u64tostring(errorCount)+" Errors");
    lcd.setCursor(0,1);
    lcd.print("RAM FAILED    ->");
    if (doserial){
      Serial.print("Error span: 0x");
      Serial.print(firstError, HEX);
      Serial.print(" to 0x");
      Serial.println(lastError, HEX);
      Serial.println("----------RAM FAILED----------");
    }    
  }else{
      if (doserial){Serial.println("----------RAM PASSED----------");}
    chipstatus=0;
    lcd.setCursor(0,0);
    lcd.print("0 Errors");
    lcd.setCursor(0,1);
    lcd.print("RAM PASSED    ->");
  }
  if (doserial){Serial.println("******************************");}
  do {blink();} while (checkButtons()==0);
  do {} while (checkButtons()!=0);
}

int checkButtons(){
  if (!buttonpressed){                                //if button enabled, check status
    lastDebounceTime = millis();
    int gobuttonState;
    int nxtbuttonState;
    gobuttonState = digitalRead(gobutton);
    nxtbuttonState = digitalRead(nxtbutton);
    if (gobuttonState==LOW){
      buttonpressed=true;                         //disables button checking for debounce  
      return 2;
    } //end if buttonstate LOW
    if (nxtbuttonState==LOW){
      buttonpressed=true;                         //disables button checking for debounce
      return 1;
    } //end if nxtbutton    
  } else {
    if ((millis() - lastDebounceTime) > pressdelay) {buttonpressed=false;}    
  }//end if buttonpressed false
  return 0;
} //end func

String u64tostring(uint64_t input) {
  String result = "";
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}
