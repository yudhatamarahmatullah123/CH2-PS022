#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <EEPROM.h>
#include <Servo.h>

#define trigPin1 6
#define echoPin1 5
#define trigPin2 4
#define echoPin2 3
#define ONE_WIRE_BUS 8

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 20, 4);
DS3231 rtc;
Servo myservo;

int addr1 = 0;
int addr2 = 1;
int addr3 = 2;
int addr4 = 3;
int addr5 = 4;

DateTime t;
int jampagi, jamsiang, jamsore;
float TempC;
float pHValue;
int tds;
int adctds;
int adcPH;
int TempCx;
int pHValuex;
int ntux;
float ntufix;
long duration1, distance1;
long duration2, distance2;
int btset = 2;
int btup = 10;
int btdown = 11;
int btok = 12;
int btsetx;
int btupx;
int btdownx;
int btokx;
float batasamo;
float batasph;
float amoniak;
int relayout = A3;
int relayin = A2;
int relayaerator = 7;
int tanda = 0;
int kolam;
int tinggipakan;

void setup() {
    jampagi = EEPROM.read(addr1);
    jamsiang = EEPROM.read(addr2);
    jamsore = EEPROM.read(addr3);
    batasamo = EEPROM.read(addr4);
    batasph = EEPROM.read(addr5);

    pinMode(relayout, OUTPUT);
    pinMode(relayin, OUTPUT);
    pinMode(relayaerator, OUTPUT);
    digitalWrite(relayout, LOW);
    digitalWrite(relayin, LOW);
    digitalWrite(relayaerator, LOW);

    pinMode(btset, INPUT_PULLUP);
    pinMode(btup, INPUT_PULLUP);
    pinMode(btdown, INPUT_PULLUP);
    pinMode(btok, INPUT_PULLUP);

    pinMode(trigPin1, OUTPUT);
    pinMode(echoPin1, INPUT);
    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);

    Wire.begin();
    sensors.begin();
    Serial.begin(9600);
    lcd.begin();
    lcd.clear();

    // rtc.begin();
    // rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAmy
    // rtc.setTime(13, 15, 0);     // Set the time to 12:00:00 (24hr format)
    // rtc.setDate(1, 1, 2014);   // Set the date to January 1st, 2014

    myservo.attach(9);
    myservo.write(100);
}

void loop() {
    btsetx = digitalRead(btset);
    btupx = digitalRead(btup);
    btdownx = digitalRead(btdown);
    btokx = digitalRead(btok);

    if (btsetx == 0) {
        delay(1000);
        lcd.clear();
        setjampagi();
        setjamsiang();
        setjamsore();
        setbatasamo();
        setbatasph();
    }

    t = rtc.getDate();

    // ketinggian air kolam
    digitalWrite(trigPin1, LOW); // Added this line
    delayMicroseconds(2);        // Added this line
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin1, LOW);
    duration1 = pulseIn(echoPin1, HIGH);
    distance1 = (duration1 / 2) / 29.1;

    // ketinggian pakan ikan
    digitalWrite(trigPin2, LOW); // Added this line
    delayMicroseconds(2);        // Added this line
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin2, LOW);
    duration2 = pulseIn(echoPin2, HIGH);
    distance2 = (duration2 / 2) / 29.1;

    if (distance1 > 35) {
        distance1 = 35;
    }

    if (distance2 > 30) {
        distance2 = 30;
    }

    kolam = 35 - distance1;
    tinggipakan = map(distance2, 5, 30, 100, 0);

    if ((kolam >= 29) && (kolam <= 31)) {
        digitalWrite(relayin, LOW);
        digitalWrite(relayout, LOW);
    }

    if (kolam < 29) {
        digitalWrite(relayin, HIGH);
        digitalWrite(relayout, LOW);
    }

    if (kolam > 31) {
        digitalWrite(relayin, LOW);
        digitalWrite(relayout, HIGH);
    }

    adcPH = analogRead(A0); // menggunakan pin A0 untuk membaca output sensor pH
    pHValue = (adcPH - 906.84) / -37.283;

    adctds = analogRead(A1);
    tds = map(adctds, 855, 1, 0, 100);

    pHValuex = pHValue * 100.0;

    sensors.requestTemperatures();
    TempC = sensors.getTempCByIndex(0); // Celcius
    TempCx = TempC * 100.0;

    if (pHValue < 6.5) {
        amoniak = (pHValue / TempC) * 0.202;
    }

    if (pHValue == 7.0) {
        amoniak = (pHValue / TempC) * 1.131;
    }

    if (pHValue > 7.0) {
        amoniak = (pHValue / TempC) * 3.306;
    }

    if (amoniak > batasamo) {
        digitalWrite(relayaerator, HIGH);
    }

    if (amoniak <= batasamo) {
        digitalWrite(relayaerator, LOW);
    }

    if ((t.hour() == jampagi) && (jampagi > 0) && (tanda == 0)) {
        myservo.write(150);
        delay(5000);
        myservo.write(100);
        tanda = 1;
    }

    if ((t.hour() == jamsiang) && (jamsiang > 0) && (tanda == 1)) {
        myservo.write(150);
        delay(5000);
        myservo.write(100);
        tanda = 2;
    }

    if ((t.hour() == jamsore) && (jamsore > 0) && (tanda == 2)) {
        myservo.write(150);
        delay(5000);
        myservo.write(100);
        tanda = 0;
    }

    lcd.setCursor(0, 0);
    lcd.print("TDS:");
    lcd.print(tds);
    lcd.print("% ");

    lcd.print("PH:");
    lcd.print(pHValue);
    lcd.print("  ");

    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(TempC, 1);
    lcd.print("c ");

    lcd.print(batasamo);
    lcd.print("/");
    lcd.print(batasph);
    lcd.print(" ");

    lcd.setCursor(0, 2);
    lcd.print(t.hour(), DEC);
    lcd.print(":");
    lcd.print(t.minute(), DEC);
    lcd.print(":");
    lcd.print(t.second(), DEC);
    lcd.print("  ");
    lcd.print(jampagi);
    lcd.print("/");
    lcd.print(jamsiang);
    lcd.print("/");
    lcd.print(jamsore);
    lcd.print("  ");

    lcd.setCursor(0, 3);
    lcd.print("S=");
    lcd.print(kolam);
    lcd.print("/");
    lcd.print(tinggipakan);
    lcd.print("  ");

    lcd.setCursor(10, 3);
    lcd.print("Mg/l:");
    lcd.print(amoniak);
    lcd.print(" ");

    Serial.print("*");
    Serial.print(pHValue * 100.0);
    Serial.print(",");
    Serial.print(tds);
    Serial.print(",");
    Serial.print(amoniak * 100.0);
    Serial.print(",");
    Serial.print(TempC * 100.0);
    Serial.println("#");

    delay(1000);
}

void setjampagi() {
    btsetx = digitalRead(btset);
    btupx = digitalRead(btup);
    btdownx = digitalRead(btdown);
    btokx = digitalRead(btok);

    lcd.setCursor(0, 0);
    lcd.print("Waktu Pakan Pagi");
    lcd.setCursor(0, 1);
    lcd.print("Jam: ");
    lcd.print(jampagi);
    lcd.print("   ");

    if (btupx == 0) {
        delay(200);
        jampagi++;
    }

    if (btdownx == 0) {
        delay(200);
        jampagi--;
    }

    if (jampagi > 23) {
        jampagi = 0;
    }

    if (btokx == 0) {
        lcd.clear();
        delay(2000);
        EEPROM.write(addr1, jampagi);
        return;
    }

    setjampagi();
}

void setjamsiang() {
    btsetx = digitalRead(btset);
    btupx = digitalRead(btup);
    btdownx = digitalRead(btdown);
    btokx = digitalRead(btok);

    lcd.setCursor(0, 0);
    lcd.print("Waktu Pakan Siang  ");
    lcd.setCursor(0, 1);
    lcd.print("Jam: ");
    lcd.print(jamsiang);
    lcd.print("   ");

    if (btupx == 0) {
        delay(200);
        jamsiang++;
    }

    if (btdownx == 0) {
        delay(200);
        jamsiang--;
    }

    if (jamsiang > 23) {
        jamsiang = 0;
    }

    if (btokx == 0) {
        lcd.clear();
        delay(2000);
        EEPROM.write(addr2, jamsiang);
        return;
    }

    setjamsiang();
}

void setjamsore() {
    btsetx = digitalRead(btset);
    btupx = digitalRead(btup);
    btdownx = digitalRead(btdown);
    btokx = digitalRead(btok);

    lcd.setCursor(0, 0);
    lcd.print("Waktu Pakan Sore  ");
    lcd.setCursor(0, 1);
    lcd.print("Jam: ");
    lcd.print(jamsore);
    lcd.print("   ");

    if (btupx == 0) {
        delay(200);
        jamsore++;
    }

    if (btdownx == 0) {
        delay(200);
        jamsore--;
    }

    if (jamsore > 23) {
        jamsore = 0;
    }

    if (btokx == 0) {
        lcd.clear();
        delay(2000);
        EEPROM.write(addr3, jamsore);
        return;
    }

    setjamsore();
}

void setbatasamo() {
    btsetx = digitalRead(btset);
    btupx = digitalRead(btup);
    btdownx = digitalRead(btdown);
    btokx = digitalRead(btok);

    lcd.setCursor(0, 0);
    lcd.print("BATAS AMONIAK  ");
    lcd.setCursor(0, 1);
    lcd.print("mg/l: ");
    lcd.print(batasamo);
    lcd.print("   ");

    if (btupx == 0) {
        delay(200);
        batasamo = batasamo + 0.1;
    }

    if (btdownx == 0) {
        delay(200);
        batasamo = batasamo - 0.1;
    }

    if (batasamo > 100.0) {
        batasamo = 0;
    }

    if (btokx == 0) {
        lcd.clear();
        delay(2000);
        EEPROM.write(addr4, batasamo);
        return;
    }

    setbatasamo();
}

void setbatasph() {
    btsetx = digitalRead(btset);
    btupx = digitalRead(btup);
    btdownx = digitalRead(btdown);
    btokx = digitalRead(btok);

    lcd.setCursor(0, 0);
    lcd.print("BATAS PH  ");
    lcd.setCursor(0, 1);
    lcd.print("PH: ");
    lcd.print(batasph);
    lcd.print("   ");

    if (btupx == 0) {
        delay(200);
        batasph = batasph + 0.1;
    }

    if (btdownx == 0) {
        delay(200);
        batasph = batasph - 0.1;
    }

    if (batasph > 100.0) {
        batasph = 0;
    }

    if (btokx == 0) {
        lcd.clear();
        delay(2000);
        EEPROM.write(addr5, batasph);
        return;
    }

    setbatasph();
}