/*
ROM Reader. Quick Arduino program to read a parallel-accessed ROM and dump it to the serial
port in hex.

Oddbloke. 16th Feb 2014.
 */

 //const char ADDR_PINS[] = {44, 42, 40, 38, 36, 34, 32, 30, 33, 35, 41, 37, 28};
 //const char DATA_PINS[] = {46, 48, 50, 53, 51, 49, 47, 45}; //same for 6264 and 6116
//    GND (pin 14), /CE (20), /OE (22) should be wired to ground.
//    Vpp (pin 1), /PGM (pin 27), Vcc (pin 28) should be wired to +5 volts.
static const int OE_PIN = 39;  //same for 6264 and 6116
static const int CS_PIN = 43;  //same for 6264 and 6116
static const int WE_PIN = 29;
static const int vccpin = 27;  //jumpered from vcc
static const int gndpin = 52;  //jumpered from gnd

// How I've wired the digital pins on my Arduino to the address and data pins on
// the ROM.
static const int kPin_A0  = 44;
static const int kPin_A1  = 42;
static const int kPin_A2  = 40;
static const int kPin_A3  = 38;
static const int kPin_A4  = 36;
static const int kPin_A5  = 34;
static const int kPin_A6  = 32;
static const int kPin_A7  = 30;
static const int kPin_A8  = 33;
static const int kPin_A9  = 35;
static const int kPin_A10 = 41;
static const int kPin_A11 = 37;
static const int kPin_A12 = 28;
static const int kPin_A13 = 31;

static const int kPin_D0 = 46;
static const int kPin_D1 = 48;
static const int kPin_D2 = 50;
static const int kPin_D3 = 53;
static const int kPin_D4 = 51;
static const int kPin_D5 = 49;
static const int kPin_D6 = 47;
static const int kPin_D7 = 45;

const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
              '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void setup()
{
  pinMode(CS_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  pinMode(WE_PIN, OUTPUT);
  pinMode(vccpin,INPUT);
  pinMode(gndpin,INPUT);
  
  // set the address lines as outputs ...
  pinMode(kPin_A0, OUTPUT);     
  pinMode(kPin_A1, OUTPUT);     
  pinMode(kPin_A2, OUTPUT);     
  pinMode(kPin_A3, OUTPUT);     
  pinMode(kPin_A4, OUTPUT);     
  pinMode(kPin_A5, OUTPUT);     
  pinMode(kPin_A6, OUTPUT);     
  pinMode(kPin_A7, OUTPUT);     
  pinMode(kPin_A8, OUTPUT);     
  pinMode(kPin_A9, OUTPUT);     
  pinMode(kPin_A10, OUTPUT);     
  pinMode(kPin_A11, OUTPUT);     
  pinMode(kPin_A12, OUTPUT);     
  pinMode(kPin_A13, OUTPUT);
 
  Serial.begin(9600);
}

void setpinsread(){
  // set the data lines as inputs ...
  pinMode(kPin_D0, INPUT); 
  pinMode(kPin_D1, INPUT); 
  pinMode(kPin_D2, INPUT); 
  pinMode(kPin_D3, INPUT); 
  pinMode(kPin_D4, INPUT); 
  pinMode(kPin_D5, INPUT); 
  pinMode(kPin_D6, INPUT); 
  pinMode(kPin_D7, INPUT); 
}
void setpinswrite(){
  // set the data lines as inputs ...
  pinMode(kPin_D0, OUTPUT); 
  pinMode(kPin_D1, OUTPUT); 
  pinMode(kPin_D2, OUTPUT); 
  pinMode(kPin_D3, OUTPUT); 
  pinMode(kPin_D4, OUTPUT); 
  pinMode(kPin_D5, OUTPUT); 
  pinMode(kPin_D6, OUTPUT); 
  pinMode(kPin_D7, OUTPUT); 
  digitalWrite(kPin_D0, LOW);
  digitalWrite(kPin_D1, LOW);
  digitalWrite(kPin_D2, LOW);
  digitalWrite(kPin_D3, LOW);
  digitalWrite(kPin_D4, LOW);
  digitalWrite(kPin_D5, LOW);
  digitalWrite(kPin_D6, LOW);
  digitalWrite(kPin_D7, LOW);
}

void SetAddress(int addr)
{
  // update the address lines to reflect the address we want ...
  digitalWrite(kPin_A0, (addr & 1)?HIGH:LOW);
  digitalWrite(kPin_A1, (addr & 2)?HIGH:LOW);
  digitalWrite(kPin_A2, (addr & 4)?HIGH:LOW);
  digitalWrite(kPin_A3, (addr & 8)?HIGH:LOW);
  digitalWrite(kPin_A4, (addr & 16)?HIGH:LOW);
  digitalWrite(kPin_A5, (addr & 32)?HIGH:LOW);
  digitalWrite(kPin_A6, (addr & 64)?HIGH:LOW);
  digitalWrite(kPin_A7, (addr & 128)?HIGH:LOW);
  digitalWrite(kPin_A8, (addr & 256)?HIGH:LOW);
  digitalWrite(kPin_A9, (addr & 512)?HIGH:LOW);
  digitalWrite(kPin_A10, (addr & 1024)?HIGH:LOW);
  digitalWrite(kPin_A11, (addr & 2048)?HIGH:LOW);
  digitalWrite(kPin_A12, (addr & 4096)?HIGH:LOW);
  digitalWrite(kPin_A13, (addr & 8192)?HIGH:LOW);
}

void writebyte(int val){
  digitalWrite(OE_PIN,LOW);
  digitalWrite(WE_PIN,LOW);  
  digitalWrite(CS_PIN,HIGH);
  digitalWrite(kPin_D0, (val & 1)?HIGH:LOW);
  digitalWrite(kPin_D1, (val & 2)?HIGH:LOW);
  digitalWrite(kPin_D2, (val & 4)?HIGH:LOW);
  digitalWrite(kPin_D3, (val & 8)?HIGH:LOW);
  digitalWrite(kPin_D4, (val & 16)?HIGH:LOW);
  digitalWrite(kPin_D5, (val & 32)?HIGH:LOW);
  digitalWrite(kPin_D6, (val & 64)?HIGH:LOW);
  digitalWrite(kPin_D7, (val & 128)?HIGH:LOW);
  digitalWrite(CS_PIN,LOW);
  }

byte ReadByte()
{
  digitalWrite(CS_PIN,LOW);
  digitalWrite(OE_PIN,LOW);
  digitalWrite(WE_PIN,HIGH);  
  // read the current eight-bit byte being output by the ROM ...
  byte b = 0;
  if (digitalRead(kPin_D0)) b |= 1;  
  if (digitalRead(kPin_D1)) b |= 2;  
  if (digitalRead(kPin_D2)) b |= 4;  
  if (digitalRead(kPin_D3)) b |= 8;  
  if (digitalRead(kPin_D4)) b |= 16; 
  if (digitalRead(kPin_D5)) b |= 32; 
  if (digitalRead(kPin_D6)) b |= 64; 
  if (digitalRead(kPin_D7)) b |= 128;
  return(b);
}

void loop()
{
  byte subcommand;
//  int patterns[]={0xAA,0x55,0x00,0xFF};
Serial.println("Choose Test:");
Serial.println("T = Test ");
Serial.println("R = Read ");
Serial.println("Enter Test (T/R):");
 do
      {
      subcommand = toupper (Serial.read ());
      } while (subcommand != 'T' && subcommand != 'R' && subcommand != ' ' );    
    if (subcommand == 'T') {testwrite();}
    if (subcommand == 'R') {readic();}    
 }

void testwrite(){
  byte d[16];
  int x, y, addr,val; 

  val=170; //aa
  Serial.println("Writing 0xAA to 0x100 addresses");
  setpinswrite();
  for (addr = 0; addr < 256; addr ++)
    {
     SetAddress(addr);
     writebyte(val);    
    }
    
  Serial.println("Reading 0x100 addresses");
  setpinsread();
  for (addr = 0; addr < 256; addr += 16)
  {
    // read 16 bytes of data from the ROM ...
    for (x = 0; x < 16; x++)
    {
      SetAddress(addr + x); // tells the ROM the byte we want ...
      d[x] = ReadByte(); // reads the byte back from the ROM
    }
  
    // now we'll print each byte in hex ...
    for (y = 0; y < 16; y++)
    {
      Serial.print(hex[ (d[y] & 0xF0) >> 4  ]);
      Serial.print(hex[ (d[y] & 0x0F)       ]);
    }
          
    // and print an ASCII dump too ...
    
    Serial.print(" ");
    for (y = 0; y < 16; y++)
    {
      char c = '.';
      if (d[y] > 32 && d[y]<127) c = d[y];
      Serial.print(c);
    }
      
    Serial.println("");
  }
  
  }

void readic(){
  byte d[16];
  int x, y, addr; 
  // The only reason I'm choosing to read in blocks of 16 bytes
  // is to keep the hex-dump code simple. You could just as easily
  // read a single byte at a time if that's all you needed.
  
  Serial.println("Reading ROM ...\n");
  setpinsread();

//  for (addr = 0; addr < 16384; addr += 16)
  for (addr = 0; addr < 512; addr += 16)
  {
    // read 16 bytes of data from the ROM ...
    for (x = 0; x < 16; x++)
    {
      SetAddress(addr + x); // tells the ROM the byte we want ...
      d[x] = ReadByte(); // reads the byte back from the ROM
    }
  
    // now we'll print each byte in hex ...
    for (y = 0; y < 16; y++)
    {
      Serial.print(hex[ (d[y] & 0xF0) >> 4  ]);
      Serial.print(hex[ (d[y] & 0x0F)       ]);
    }
          
    // and print an ASCII dump too ...
    
    Serial.print(" ");
    for (y = 0; y < 16; y++)
    {
      char c = '.';
      if (d[y] > 32 && d[y]<127) c = d[y];
      Serial.print(c);
    }
      
    Serial.println("");
  }
  
  // All done, so lockup ...
  //while (true) {delay(10000);}

}
