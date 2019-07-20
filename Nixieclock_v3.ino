#include <Wire.h>
#include <RTClib.h>

#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by Unix time_t as ten ASCII digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

RTC_DS3231 rtc;

const byte pinsBus [8] = { 2, 3, 4, 5, 6, 7, 8, 9 };
const byte pinsCs [6] = {12, 11, 10, A0, A1, A2};

const unsigned long long digits [4][10] = {
  // 0       1       2       3       4       5       6       7       8       9
  {1LL<<44, 1LL<<41, 1LL<<40, 1LL<< 0, 1LL<< 1, 1LL<< 2, 1LL<< 3, 1LL<< 4, 1LL<< 5, 1LL<<45},
  {1LL<<34, 1LL<<47, 1LL<<46, 1LL<< 6, 1LL<< 7, 1LL<< 8, 1LL<< 9, 1LL<<10, 1LL<<11, 1LL<<35},
  {1LL<<24, 1LL<<37, 1LL<<36, 1LL<<12, 1LL<<13, 1LL<<14, 1LL<<15, 1LL<<16, 1LL<<17, 1LL<<25},
  {1LL<<30, 1LL<<27, 1LL<<26, 1LL<<18, 1LL<<19, 1LL<<20, 1LL<<21, 1LL<<22, 1LL<<23, 1LL<<31}
};

const unsigned long long points [4][2] = {
  // Left Right
  {1LL<<42, 1LL<<43},
  {1LL<<32, 1LL<<33},
  {1LL<<38, 1LL<<39},
  {1LL<<28, 1LL<<29}
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
    rtc.adjust(DateTime(0));
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
  unsigned long long display = 0LL;
  for (int d = 0; d < sizeof(displayNumbers); d++) {
    // first digit of number
    int value = displayNumbers[d] / 10;
    display |= digits[d*2][value];

    value = displayNumbers[d] % 10;
    display |= digits[d*2+1][value];
    
    // set points
    for (int t = 0; t < sizeof(displayPoints); t++) {
      for (int p = 0; p < sizeof(displayPoints[t]); p++) {
        if (displayPoints[t][p]) {
          display |= points[t][p];
        }
      }
    }
  }

  writeBus(display);
  Serial.println();
}

void writeBus(unsigned long long value) {
  for (int c = 0; c < sizeof(pinsCs); c++) {
    byte chip = value >> c * 8;
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
