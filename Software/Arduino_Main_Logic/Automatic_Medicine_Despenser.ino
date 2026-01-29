//https://raw.githubusercontent.com/SpacehuhnTech/arduino/main/package_spacehuhn_index.json

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS1307.h>
#include <Servo.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(2, 3);  // RX, TX

String number = "+8801623924703";

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- RTC ----------------
DS1307 rtc(SDA, SCL);

// ---------------- Servos ----------------
Servo selectorServo;
Servo boxServo;

// ---------------- Pins ----------------
#define SELECTOR_SERVO_PIN 11
#define BOX_SERVO_PIN 6
#define BUZZER_PIN A0
#define IR_SENSOR_PIN 4
#define BOX_BUTTON_PIN A2

// ---------------- EEPROM ----------------
#define EEPROM_SERVO_POS 0
#define EEPROM_FLAG 5

#define ADDR_MORNING_H 10
#define ADDR_MORNING_M 11
#define ADDR_NOON_H 12
#define ADDR_NOON_M 13
#define ADDR_NIGHT_H 14
#define ADDR_NIGHT_M 15

// ---------------- Pill Times ----------------
int MORNING_HOUR, MORNING_MIN;
int NOON_HOUR, NOON_MIN;
int NIGHT_HOUR, NIGHT_MIN;

// ---------------- Flags ----------------
bool morningDone = false, noonDone = false, nightDone = false;
bool morningTaken = false, noonTaken = false, nightTaken = false;
bool morningCallSent = false, noonCallSent = false, nightCallSent = false;

// ---------------- Box State ----------------
bool boxOpen = false;
unsigned long boxOpenTime = 0;
String currentDose = "";

// ---------------- Servo ----------------
int lastServoAngle = 0;

// ---------------- Display ----------------
unsigned long lastSwitch = 0;
bool showRemaining = false;

// ---------------- Nano -> ESP8266 ----------------
bool lastMorningTaken = false;
bool lastNoonTaken = false;
bool lastNightTaken = false;

// ---------------- Pill detection ----------------
bool pillDetected = false;
unsigned long pillDetectTime = 0;
#define PILL_TAKEN_DELAY 5000
#define PILL_TIMEOUT 15000

// =================================================
// SLOW SERVO MOVEMENT
void moveServoSlow(Servo &s, int fromAngle, int toAngle, int delayMs){
  if(fromAngle<toAngle){
    for(int p=fromAngle;p<=toAngle;p++){ s.write(p); delay(delayMs); }
  }else{
    for(int p=fromAngle;p>=toAngle;p--){ s.write(p); delay(delayMs); }
  }
}

// =================================================
void makeCall(){
  Serial.println("Call Alert!"); // replace with GSM AT commands if available
  gsmSerial.print(F("ATD"));
  gsmSerial.print(number);
  gsmSerial.print(F(";\r\n"));
}

// =================================================
void setup(){
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  rtc.begin();
  rtc.halt(false);

    // ---- SET RTC TIME ONLY ONCE ----
  
 // rtc.setDOW(MONDAY);
// rtc.setTime(18, 56, 0);
  // rtc.setDate(19, 1, 2026);

  selectorServo.attach(SELECTOR_SERVO_PIN);
  boxServo.attach(BOX_SERVO_PIN);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(BOX_BUTTON_PIN, INPUT_PULLUP);

  if(EEPROM.read(EEPROM_FLAG)!=123){
    EEPROM.update(EEPROM_FLAG,123);
    EEPROM.update(EEPROM_SERVO_POS,0);
  }

  lastServoAngle = EEPROM.read(EEPROM_SERVO_POS);
  selectorServo.write(lastServoAngle); // go directly to last position
  boxServo.write(90);

  // Load pill times from EEPROM
  MORNING_HOUR = EEPROM.read(ADDR_MORNING_H);
  MORNING_MIN = EEPROM.read(ADDR_MORNING_M);
  NOON_HOUR = EEPROM.read(ADDR_NOON_H);
  NOON_MIN = EEPROM.read(ADDR_NOON_M);
  NIGHT_HOUR = EEPROM.read(ADDR_NIGHT_H);
  NIGHT_MIN = EEPROM.read(ADDR_NIGHT_M);

  gsmSerial.begin(9600);

  delay(500);
  gsmSerial.println("AT+CMGF=1");  //Set SMS to Text Mode
                                   // updateSerial();
  delay(500);
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  //updateSerial();
  delay(500);
  gsmSerial.println("AT+CMGL=\"REC UNREAD\"");  // Read Unread Messages
                                                // updateSerial();
  delay(500);

  lcd.print("Medicine Ready");
  delay(2000);
  lcd.clear();
}

// =================================================
void loop(){
  // ---------- Receive pill times from ESP8266 ----------
  while(Serial.available()){
    String line = Serial.readStringUntil('\n');
    line.trim();
    if(line.startsWith("TIME:")){
      int mi = line.indexOf("M=");
      int ni = line.indexOf("N=");
      int ei = line.indexOf("E=");

      if(mi!=-1){
        MORNING_HOUR = line.substring(mi+2, mi+4).toInt();
        MORNING_MIN  = line.substring(mi+5, mi+7).toInt();
        EEPROM.update(ADDR_MORNING_H, MORNING_HOUR);
        EEPROM.update(ADDR_MORNING_M, MORNING_MIN);
      }
      if(ni!=-1){
        NOON_HOUR = line.substring(ni+2, ni+4).toInt();
        NOON_MIN  = line.substring(ni+5, ni+7).toInt();
        EEPROM.update(ADDR_NOON_H, NOON_HOUR);
        EEPROM.update(ADDR_NOON_M, NOON_MIN);
      }
      if(ei!=-1){
        NIGHT_HOUR = line.substring(ei+2, ei+4).toInt();
        NIGHT_MIN  = line.substring(ei+5, ei+7).toInt();
        EEPROM.update(ADDR_NIGHT_H, NIGHT_HOUR);
        EEPROM.update(ADDR_NIGHT_M, NIGHT_MIN);
      }
    }
  }

  Time t = rtc.getTime();
  int h = t.hour;
  int m = t.min;
  int s = t.sec;

  // ---------- DISPLAY ----------
  if (millis() - lastSwitch > 15000) {
    showRemaining = true;
    lastSwitch = millis();
    lcd.clear();
  }

  if (showRemaining){
    showRemainingTime(h, m, s);
    if (millis() - lastSwitch > 2500){
      showRemaining = false;
      lcd.clear();
    }
  }else{
    showDateTime();
  }

  // ---------- DISPENSE ----------
  if(h==MORNING_HOUR && m==MORNING_MIN && !morningDone){
    currentDose="MORNING"; dispense("MORNING DOSE",0); morningDone=true;
  }
  if(h==NOON_HOUR && m==NOON_MIN && !noonDone){
    currentDose="NOON"; dispense("NOON DOSE",90); noonDone=true;
  }
  if(h==NIGHT_HOUR && m==NIGHT_MIN && !nightDone){
    currentDose="NIGHT"; dispense("NIGHT DOSE",180); nightDone=true;
  }

  // ---------- BOX BUTTON ----------
  if(digitalRead(BOX_BUTTON_PIN)==LOW){
    delay(200);
    boxServo.write(boxOpen?90:0);
    boxOpen = !boxOpen;
    boxOpenTime = millis();
    pillDetected=false;
    while(digitalRead(BOX_BUTTON_PIN)==LOW);
  }

  // ---------- CHECK MEDICINE TAKEN ----------
  checkMedicineTaken();

  // ---------- DAILY RESET ----------
  if(h==0 && m==0 && s==1){
    morningDone=noonDone=nightDone=false;
    morningTaken=noonTaken=nightTaken=false;
    morningCallSent=noonCallSent=nightCallSent=false;
  }

  delay(200);
}

// =================================================
void dispense(String msg,int angle){
  lcd.clear();
  lcd.print("MEDICINE TIME");
  lcd.setCursor(0,1); lcd.print(msg);

  moveServoSlow(selectorServo,lastServoAngle,angle,20);
  lastServoAngle=angle;
  EEPROM.update(EEPROM_SERVO_POS,angle); // Save last position

  if(currentDose=="MORNING") morningTaken=false;
  if(currentDose=="NOON") noonTaken=false;
  if(currentDose=="NIGHT") nightTaken=false;

  boxServo.write(0);
  boxOpen=true;
  boxOpenTime=millis();
  pillDetected=false;

  lcd.clear();
  lcd.print("Take Medicine");
}

// =================================================
void checkMedicineTaken(){
  if(!boxOpen) return;

  // ---------- IR detect ----------
  if(!pillDetected && digitalRead(IR_SENSOR_PIN)==LOW){
    pillDetected=true;
    pillDetectTime=millis(); // Start 5s timer

    if(currentDose=="MORNING") morningTaken=true;
    if(currentDose=="NOON") noonTaken=true;
    if(currentDose=="NIGHT") nightTaken=true;

    // Send status to ESP immediately
    if(morningTaken!=lastMorningTaken){ Serial.println("STATUS:MORNING="+String(morningTaken)); lastMorningTaken=morningTaken;}
    if(noonTaken!=lastNoonTaken){ Serial.println("STATUS:NOON="+String(noonTaken)); lastNoonTaken=noonTaken;}
    if(nightTaken!=lastNightTaken){ Serial.println("STATUS:NIGHT="+String(nightTaken)); lastNightTaken=nightTaken;}
  }

  // ---------- Beep while box open ----------
  if(boxOpen && !pillDetected){
    digitalWrite(BUZZER_PIN,HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN,LOW);
    delay(300);
  }

  // ---------- Case 1: Pill Taken ----------
  if(pillDetected && millis()-pillDetectTime>=5000){
    boxServo.write(90);
    boxOpen=false;
    digitalWrite(BUZZER_PIN,LOW); // Stop beep
  }

  // ---------- Case 2: Pill NOT Taken ----------
  if(!pillDetected && millis()-boxOpenTime>=15000){
    boxServo.write(90);
    boxOpen=false;
    makeCall();
    digitalWrite(BUZZER_PIN,LOW); // Stop beep
    // Send status
    if(morningTaken!=lastMorningTaken){ Serial.println("STATUS:MORNING="+String(morningTaken)); lastMorningTaken=morningTaken;}
    if(noonTaken!=lastNoonTaken){ Serial.println("STATUS:NOON="+String(noonTaken)); lastNoonTaken=noonTaken;}
    if(nightTaken!=lastNightTaken){ Serial.println("STATUS:NIGHT="+String(nightTaken)); lastNightTaken=nightTaken;}
    
    delay(5000);
  }
}

// =================================================
void showDateTime(){
  lcd.setCursor(0,0); lcd.print("Time: "); lcd.print(rtc.getTimeStr());
  lcd.setCursor(0,1); lcd.print(rtc.getDateStr());
}

// =================================================
void showRemainingTime(int h,int m,int s){
  int nh,nm;
  if(h<MORNING_HOUR || (h==MORNING_HOUR && m<MORNING_MIN)){ nh=MORNING_HOUR; nm=MORNING_MIN;}
  else if(h<NOON_HOUR || (h==NOON_HOUR && m<NOON_MIN)){ nh=NOON_HOUR; nm=NOON_MIN;}
  else if(h<NIGHT_HOUR || (h==NIGHT_HOUR && m<NIGHT_MIN)){ nh=NIGHT_HOUR; nm=NIGHT_MIN;}
  else{ nh=MORNING_HOUR; nm=MORNING_MIN;}

  long now=h*3600L + m*60L + s;
  long next=nh*3600L + nm*60L;
  if(next<=now) next+=86400;
  long diff=next-now;

  lcd.setCursor(0,0); lcd.print("Next Pill In");
  lcd.setCursor(0,1);
  lcd.print(diff/3600); lcd.print("h ");
  lcd.print((diff%3600)/60); lcd.print("m ");
  lcd.print(diff%60); lcd.print("s");
}
