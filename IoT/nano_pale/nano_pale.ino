#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int ntcPin    = A0;
const int mq135Pin  = A1;
const int phPin     = A2;

float temperature, ammonia, phValue;
unsigned long previousMillis = 0;
const long interval = 5000; // interval pembacaan sensor setiap 5 detik

void setup() {
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
}

void loop() {
  unsigned long currentMillis = millis();

  // Baca nilai sensor setiap 5 detik
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Baca nilai sensor
    temperature = bacaSuhu(ntcPin);
    ammonia = bacaAmmonia(mq135Pin);
    phValue = bacaPh(phPin);

    // Kirim data ke NodeMCU melalui Serial
    Serial.print("T");
    Serial.println(temperature);
    Serial.print("A");
    Serial.println(ammonia);
    Serial.print("P");
    Serial.println(phValue);

    // Update tampilan LCD
    updateLCD();
  }
}

float bacaSuhu(int pin) {
  float Vo = analogRead(pin);
  float R2 = 10000 * (1023.0 / Vo - 1.0);
  float logR2 = log(R2);
  float T = 1.0 / (0.001009249522 + 0.0002378405444*logR2 + 2.019202697e-07*logR2*logR2*logR2);
  float temperature = T - 273.15;
  return temperature;
}

float bacaAmmonia(int pin) {
  float VRL = analogRead(pin) * (3.3 / 1023.0);
  float RS = (3.3 / VRL - 1) * 10;
  float Ro = 28; // Ganti dengan nilai RO yang sesuai
  float ratio = RS / Ro;
  float ppm = pow(10, ((log10(ratio) - 0.858) / -0.417));
  return ppm;
}

float bacaPh(int pin) {
  int adcPH = analogRead(pin);
  float voltage = adcPH * (5.0 / 1023.0);
  float pHValue = (6.4516 * voltage) - 5.7742;
  return pHValue;
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pale App - CH2-PS022");

  lcd.setCursor(0, 1);
  lcd.print("Temp    : ");
  lcd.print(temperature);
  lcd.setCursor(15, 1);
  lcd.print(" *C");

  lcd.setCursor(0, 2);
  lcd.print("Ammonia : ");
  lcd.print(ammonia);
  lcd.setCursor(15, 2);
  lcd.print(" mg/L");

  lcd.setCursor(0, 3);
  lcd.print("pH      : ");
  lcd.print(phValue);
  lcd.setCursor(15, 3);
  lcd.print(" pH");
}
