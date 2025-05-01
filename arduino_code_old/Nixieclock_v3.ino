#include <Wire.h>
#include <RTClib.h>

#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by Unix time_t as ten ASCII digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message

#define PIN_SHDN 13

RTC_DS3231 rtc;

const byte pinsBus [8] = { 5, 4, 3, 2, 6, 7, 8, 9 };
const byte pinsCs [6] = {10, A0, 11, A1, 12, A2};

const unsigned int digits [4][10] = {
//   0   1   2   3   4   5   6   7   8   9
  {  4,  1,  0,  8,  9, 10, 11, 12, 13,  5 },
  { 18,  7,  6, 14, 15, 24, 25, 26, 27, 19 },
  { 32, 21, 20, 28, 29, 30, 31, 40, 41, 33 },
  { 38, 35, 34, 42, 43, 44, 45, 46, 47, 39 }
};

const unsigned int points [4][2] = {
//   L   R
  {  2,  3 },
  { 16, 17 },
  { 22, 23 },
  { 36, 37 }
};

byte displayNumbers [2];
boolean displayPoints [4][2];

void setup() {
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  
  noInterrupts();
  Serial.begin(9600);
  interrupts();
  
  for (int i = 0; i < sizeof(pinsBus); i++) {
    pinMode(pinsBus[i], OUTPUT);
    digitalWrite(pinsBus[i], LOW);
  }
  for (int i = 0; i < sizeof(pinsCs); i++) {
    pinMode(pinsCs[i], OUTPUT);
    digitalWrite(pinsCs[i], LOW);
  }
  
  // Reset the display
  writeDisplay();

  pinMode(PIN_SHDN, OUTPUT);
  digitalWrite(PIN_SHDN, LOW);
}

void loop() {
  if (Serial.available()) {
    processSyncMessage();
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (true);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, resetting time");
    rtc.adjust(DateTime((uint32_t) 0));
  }

  DateTime now = rtc.now();
  Serial.println(now.unixtime(), DEC);

  now = (now + TimeSpan(0, 1, 0, 0)); //UTC+1
  
  boolean dst = summertime(now.year(), now.month(), now.day(), now.hour(), 1);
  if (dst) now = (now + TimeSpan(0, 1, 0, 0));
  // Something seems to adjust the timezone and dst before rtc.now

  displayNumbers[0] = now.hour();
  displayNumbers[1] = now.minute();
  writeDisplay();

  delay(1000);
}

boolean summertime(int year, byte month, byte day, byte hour, byte tzHours) {
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
  if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7)) || month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7))) {
    return true;
  } else {
    return false;
  }
}

void processSyncMessage() {
  // if time sync available from serial port, update time and return true
  while (Serial.available() >=  TIME_MSG_LEN ) {  // time message consists of header & 10 ASCII digits
    char c = Serial.read();
    Serial.print(c);
    if (c == TIME_HEADER) {
      uint32_t pctime = 0;
      for (int i=0; i < TIME_MSG_LEN -1; i++) {
        c = Serial.read();
        if( c >= '0' && c <= '9') {
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number
        }
      }
      Serial.print("Setting time to");
      Serial.println(pctime);
      rtc.adjust(pctime);
    }
  }
}

void writeDisplay() {
  byte display [6] = {0, 0, 0, 0, 0, 0};
  for (int d = 0; d < sizeof(displayNumbers); d++) {
    // first digit of number
    int value = displayNumbers[d] / 10;
    int digitBit = digits[d*2][value];
    display[digitBit / 8] |= 1 << (digitBit % 8);

    // second digit of number
    value = displayNumbers[d] % 10;
    digitBit = digits[d*2+1][value];
    display[digitBit / 8] |= 1 << (digitBit % 8);
  }
  // set points
  for (int t = 0; t < sizeof(displayPoints); t++) {
    for (int p = 0; p < sizeof(displayPoints[t]); p++) {
      if (displayPoints[t][p]) {
        int digitBit = points[t][p];
        display[digitBit / 8] |= 1 << (digitBit % 8);
      }
    }
  }

  writeBus(display);
  Serial.println();
}

void writeBus(byte value[]) {
  for (int c = 0; c < sizeof(pinsCs); c++) {
    byte chip = value[c];
    setBus(chip);
    delay(1);
    digitalWrite(pinsCs[c], HIGH);
    delay(1);
    digitalWrite(pinsCs[c], LOW);
  }
}
void setBus(byte value) {
  for (int i = 0; i < 8; i++) {
    int bit = value & 1;
    if (bit == 1) {
      digitalWrite(pinsBus[i], HIGH);
    } else {
      digitalWrite(pinsBus[i], LOW);
    }
    value >>= 1;
  }
}
