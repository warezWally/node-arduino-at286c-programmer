

char serial[128];

#define WRITE_PIN 13
#define CLOCK_PIN 2
#define READ_WRITE 3
#define WRITE_ENABLE 7
#define OUTPUT_ENABLE 4
#define BUS_ENABLE 8
#define READY 9


void readEEPROM() {

  digitalWrite(WRITE_ENABLE, 1);
  digitalWrite(BUS_ENABLE, 0);
  digitalWrite(OUTPUT_ENABLE, 0);

  DDRA = 0xff;
  DDRC = 0xff;
  DDRL = 0x00;

  char output[128];
  byte data[16];

  delay(5);

  for (uint32_t i = 0xe000; i <= 0xffff; i += 16) {

    for (int j = 0; j < 16; j++) {
      PORTA = i + j;
      PORTC = ((i + j) >> 8);

      data[j] = PINL;


    }

    sprintf(output, "%04x:\t%02x %02x %02x %02x %02x %02x %02x %02x    %02x %02x %02x %02x %02x %02x %02x %02x", word(i), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(output);

  }

}

void writeEEPROM() {

  digitalWrite(OUTPUT_ENABLE, 1);

  DDRA = 0xff;
  DDRC = 0xff;
  DDRL = 0xff;

  delay(5);

  for (uint32_t i = 0xe000; i <= 0xffff; i++) {

    PORTA = i & 0xff;
    PORTC = ((i) >> 8) & 0xff;

    if (i == 0xfffc) {
      PORTL = 0x00;
    } else if (i == 0xfffd) {
      PORTL = 0xe0;
    } else {
      PORTL = 0xea;
    }

    delay(3);

    digitalWrite(WRITE_ENABLE, 0);

    delayMicroseconds(1);

    digitalWrite(WRITE_ENABLE, 1);

    delay(3);

  }

}

void setup() {

  Serial.begin(115200);

  digitalWrite(WRITE_ENABLE, 1);
  pinMode(WRITE_ENABLE, OUTPUT);
  pinMode(WRITE_ENABLE, OUTPUT);
  pinMode(BUS_ENABLE, OUTPUT);
  pinMode(OUTPUT_ENABLE, OUTPUT);


  digitalWrite(BUS_ENABLE, 1);
  digitalWrite(OUTPUT_ENABLE, 0);

  if (!digitalRead(WRITE_PIN)) {

    readEEPROM();
    writeEEPROM();

    digitalWrite(OUTPUT_ENABLE, 0);

    PORTL = 0x00;
    PORTA = 0x00;
    PORTC = 0x00;
    Serial.println("DONE");
    // return;

    /* DDRL = 0xff;
      PORTL = 0xea;
      digitalWrite(OUTPUT_ENABLE,1);

      for(int b = 0xfffc;b<0xffff;b++){
        PORTA = b;
        PORTC = (b >> 8);
        if(b == 0xfffc) {
            PORTL = 0x00;
            } else if (b == 0xfffd)
            {
                PORTL = 0x80;
                } else
                {
                    PORTL = 0xea;
                    }
        Serial.println(PINL,HEX);
        delayMicroseconds(10);
        digitalWrite(WRITE_ENABLE,0);
        delayMicroseconds(10);
        digitalWrite(WRITE_ENABLE,1);
      } */
    Serial.println("SWITCH NOW");
    delay(2000);
    digitalWrite(BUS_ENABLE, 1);
    /* DDRA = 0xff;
      DDRC = 0xff;

      for(int i = 0x0000; i<0xffff;i++){

        delay(25);
        PORTA = i;
        PORTC = (i >> 8) ^ 0x80;

      } */


  }
  digitalWrite(BUS_ENABLE, 1);
  digitalWrite(OUTPUT_ENABLE, 0);


  DDRA = 0x00;   //Address 0-7
  DDRC = 0x00;   //Address 8-15
  DDRL = 0x00;   //Data 0-7

  pinMode(CLOCK_PIN, INPUT);
  pinMode(READ_WRITE, INPUT);
  pinMode(WRITE_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), clockPulse, RISING);

}

void clockPulse() {

  char output[32];
  unsigned address = 0;
  unsigned data = 0;
  bool bit;

  for (int i = 7; i >= 0; i--) {
    bit = PINC & (1 << i);
    Serial.print(bit ? "1" : "0");
    address |= (bit << (i + 8));
  }

  for (int i = 7; i >= 0; i--) {
    bit = PINA & (1 << i);
    Serial.print(bit ? "1" : "0");
    address |= (bit << (i));
  }

  Serial.print("\t");


  for (int i = 7; i >= 0; i--) {
    bit = PINL & (1 << i);
    Serial.print(bit ? "1" : "0");
    data |= (bit << (i));
  }


  sprintf(output, "\t%04x\t%c\t%02x", address, digitalRead(3) ? 'r' : 'W', data);
  Serial.print(output);

  /*    Serial.print(PINA,BIN);

     Serial.print("\t");
     Serial.print("0x");
     Serial.print(PINC,HEX);
     Serial.print(PINA,HEX); */

  Serial.println();

}

void loop() {



}
