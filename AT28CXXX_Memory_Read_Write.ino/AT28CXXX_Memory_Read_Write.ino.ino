#define CLK 13
#define BE 12
#define RW 11
#define OE 10
#define WE 9
#define CE 8

#define CLK_MS 1

byte command = 0;
uint32_t inputHexValue = 0xe000;

void writeByteToEEPROM(uint16_t addr, byte b) {

  PORTA = (addr) & 0xff;
  PORTC = ((addr) >> 8) & 0xff;
  PORTL = b;

  digitalWrite(WE, 0);

  delayMicroseconds(1);

  digitalWrite(WE, 1);

  delay(7);

}

void writeEEPROM(uint16_t from, uint16_t to) {

  DDRA = 0xff;
  DDRC = 0xff;
  DDRL = 0xff;

  delay(5);

  digitalWrite(BE, 0);
  digitalWrite(OE, 1);
  digitalWrite(CE, 0);
  digitalWrite(WE, 1);

  for (uint32_t i = from; i <= to; i++) {

    uint16_t j = i - 0xe000;

    if (i == 0xfffc) {
      Serial.println("fffc");
      writeByteToEEPROM(j, 0x00);
    } else if (i == 0xfffd) {
      Serial.println("fffd");
      writeByteToEEPROM(j, 0xff); // GO TO START VECTOR IN EEPROM
    } else {
      writeByteToEEPROM(j, 0xea);
    }
  }
}

void readEEPROM(uint16_t from, uint16_t to) {

  DDRA = 0xff;
  DDRC = 0xff;
  DDRL = 0x00;

  char output[128];
  byte data[16];

  delay(5);

  digitalWrite(BE, 0);
  digitalWrite(OE, 0);
  digitalWrite(CE, 0);
  digitalWrite(WE, 1);

  for (uint32_t i = from; i <= to; i += 16) {

    uint16_t k = i - 0xe000;

    for (byte j = 0; j < 16; j++) {

      PORTA = k + j;
      PORTC = ((k + j) >> 8);

      data[j] = PINL;

    }

    sprintf(output, "%04x/%04x:\t%02x %02x %02x %02x %02x %02x %02x %02x    %02x %02x %02x %02x %02x %02x %02x %02x", word(i), word(k), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(output);
  }
}


void serialWait() {
  if (!command) Serial.println("Enter Command [write/read/run]");
  while (!Serial.available() && !command) {
  }
}

void serialReader() {

  String string = "";

  while (Serial.available()) {

    string = String(Serial.readStringUntil(13));

    if (string == "write") {
      Serial.println("Write Mode");
      command = 1;
    }
    if (string == "read") {
      Serial.println("Read Mode");
      command = 2;
    }
    if (string == "run") {
      Serial.println("Run Mode");
      command = 3;
    }
  }
}



void setup() {


  Serial.begin(57600);
  Serial.println("INIT");

  pinMode(CLK, 1);
  pinMode(BE, OUTPUT);
  pinMode(RW, INPUT);
  pinMode(OE, OUTPUT);
  pinMode(WE, OUTPUT);
  pinMode(CE, OUTPUT);

}


void loop() {

  serialWait();
  serialReader();

  if (command == 1) {
    pinMode(OE, OUTPUT);
    pinMode(WE, OUTPUT);
    pinMode(CE, OUTPUT);
    writeEEPROM(0xe000, 0xffff);
    delay(3000);
    command = 0;
  } else if (command == 2) {
    pinMode(OE, OUTPUT);
    pinMode(WE, OUTPUT);
    pinMode(CE, OUTPUT);
    readEEPROM(0xe000, 0xffff);
    delay(3000);
    command = 0;
  } else if (command == 3) {

    DDRA = 0x00;
    DDRC = 0x00;
    DDRL = 0x00;

    digitalWrite(BE, 1);
    pinMode(OE, INPUT);
    pinMode(CE, INPUT);
    pinMode(WE, INPUT);

    char output[15];

    digitalWrite(CLK, 0);
    delay(CLK_MS);
    digitalWrite(CLK, 1);

    sprintf(output, "   %04x  %c %c %02x", PINA + (PINC << 8), digitalRead(RW) ? 'r' : 'W', digitalRead(CE) ? 'd' : 'E', PINL);
    Serial.println(output);
    delay(CLK_MS);



  }
}
