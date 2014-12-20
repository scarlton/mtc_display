#include <LiquidCrystal.h>

int statusled = 13;
byte statusbyte = 0;
byte data1 = 0;
byte data2 = 0;

boolean playing = false;
unsigned int clocks = 0;
int clocktmp = 0;
byte hour = 0;
byte minute = 0;
byte second = 0;
byte frame = 0;
byte fps = 0x3; //30fps default
byte nibble = 0;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  pinMode(statusled, OUTPUT);

  lcd.begin(16,2);
  lcd.print("   MIDI Clock");
  delay(1000);
  lcd.clear();

  lcd.setCursor(7,0);
  lcd.print('|');
  lcd.setCursor(10,0);
  lcd.print('|');
  lcd.setCursor(4,1);
  lcd.print(':');
  lcd.setCursor(7,1);
  lcd.print(':');
  lcd.setCursor(10,1);
  lcd.print('.');
  Serial.begin(31250);
}

void loop() {
  if (Serial.available()) {
    statusbyte = Serial.read();
    if (statusbyte > 0x80) {
      switch (statusbyte) {
        case 0xF0: //Sys Ex
          while (!Serial.available()) {}
          if (Serial.read() == 0x7F) { //Real Time Sys Ex
            while (Serial.available() < 2) {}
            Serial.read();
            if (Serial.read() == 0x01) { // MTC Full Frame
              while (Serial.available() < 6) {}
              Serial.read();
              data1 = Serial.read();
              hour = data1 & 0x1F;
              fps = (data1 & 0x60) >> 5;
              minute = Serial.read();
              second = Serial.read();
              frame = Serial.read();
              Serial.read();
              displayMTC(1);
            }
          }
          break;

        case 0xF1: //MTC quarter frame
          while (!Serial.available()) {}
          data1 = Serial.read();
          nibble = data1 & 0xF;
          switch (int(data1 >> 4)) {
            case 0:
              //frames low
              frame = (frame & 0xF0) | nibble;
              displayMTC_frame(1);
              break;
            case 1:
              //frames high
              frame = (frame & 0x0F) | (nibble << 4);
              displayMTC_frame(1);
              break;
            case 2:
              //seconds low
              second = (second & 0xF0) | nibble;
              displayMTC_second(1);
              break;
            case 3:
              //seconds high
              second = (second & 0x0F) | (nibble << 4);
              displayMTC_second(1);
              break;
            case 4:
              //minutes low
              minute = (minute & 0xF0) | nibble;
              displayMTC_minute(1);
              break;
            case 5:
              //minutes high
              minute = (minute & 0x0F) | (nibble << 4);
              displayMTC_minute(1);
              break;
            case 6:
              //hours low
              hour = (hour & 0xF0) | nibble;
              displayMTC_hour(1);
              break;
            case 7:
              //hours high
              hour = (hour & 0x0F) | ((nibble & 0x01) << 4);
              fps = (nibble & 0x06) >> 1;
              displayMTC_hour(1);
              break;
            default:
              //error
              break;
          }
          break;

        case 0xF2: //song position pointer
          while (!Serial.available()) {}
          data1 = Serial.read();
          while (!Serial.available()) {}
          data2 = Serial.read();
          clocks = int((data2 << 7) | data1) * 6;
          displayBars(0);
          break;

        case 0xF8: //midi clock
          clocks++;
          if (clocks % 24 == 0) { //blink on each quarter note
            blink();
          }
          displayBars(0);
          break;

        case 0xFA: //midi start
          playing = true;
          clocks = 0;
          displayBars(0);
          break;

        case 0xFB: //midi continue
          playing = true;
          break;

        case 0xFC: //midi stop
          playing = false;
          break;

        default:
          break;
      }
    }
  }
}

void displayMTC_hour(int row) {
  lcd.setCursor(1, row);
  if (hour < 100) {
    lcd.print(" ");
    lcd.setCursor(2, row);
  }
  if (hour < 10) {
    lcd.print(" ");
    lcd.setCursor(3, row);
  }
  lcd.print(hour, DEC);
}

void displayMTC_minute(int row) {
  lcd.setCursor(5, row);
  if (minute < 10) {
    lcd.print("0");
    lcd.setCursor(6, row);
  }
  lcd.print(minute, DEC);
}

void displayMTC_second(int row) {
  lcd.setCursor(8, row);
  if (second < 10) {
    lcd.print("0");
    lcd.setCursor(9, row);
  }
  lcd.print(second, DEC);
}

void displayMTC_frame(int row) {
  lcd.setCursor(11, row);
  if (frame < 10) {
    lcd.print("0");
    lcd.setCursor(12, row);
  }
  lcd.print(frame, DEC);
}

void displayMTC(int row) {
  displayMTC_hour(row);
  displayMTC_minute(row);
  displayMTC_second(row);
  displayMTC_frame(row);
}

void displayBars(int row) {
  lcd.setCursor(4, row);
  if ((clocks / 96) + 1 < 100) {
    lcd.print(" ");
    lcd.setCursor(5, row);
  }
  if ((clocks / 96) + 1 < 10) {
    lcd.print(" ");
    lcd.setCursor(6, row);
  }
  lcd.print((clocks / 96) + 1);

  lcd.setCursor(9, row);
  lcd.print(((clocks % 96) / 24) + 1);

  lcd.setCursor(11, row);
  if ((clocks % 24) < 10) {
    lcd.print("0");
    lcd.setCursor(12, row);
  }
  lcd.print(clocks%24);
}

void blink() {
  digitalWrite(statusled, HIGH);
  delay(1);
  digitalWrite(statusled, LOW);
}