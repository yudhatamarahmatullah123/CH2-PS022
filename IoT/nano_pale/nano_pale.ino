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
  const double referenceResistance = 7107.02; // Nilai resistansi referensi pada suhu 25°C
  const double referenceTemperature = 25.0; // Suhu referensi (°C) untuk resistansi referensi
  const double betaCoefficient = 3950.0; // Koefisien Beta NTC Thermistor
  const double seriesResistor = 10000.0; // Nilai resistor seri pada rangkaian

  // Baca nilai analog dari NTC Thermistor
  int ntcValue = analogRead(pin);

  // Konversi nilai analog menjadi resistansi NTC Thermistor
  double resistance = seriesResistor / (1023.0 / ntcValue - 1.0);

  // Hitung suhu menggunakan persamaan Steinhart-Hart
  double steinhart;
  steinhart = log(resistance / referenceResistance);
  steinhart /= betaCoefficient;
  steinhart += 1.0 / (referenceTemperature + 273.15);
  steinhart = 1.0 / steinhart - 273.15;

  // Kalibrasi suhu jika diperlukan
  double calibratedTemperature = kalibrasiSuhu(steinhart);

  // Tampilkan hasil di Serial Monitor
//  Serial.print("ADC Value: ");
//  Serial.print(ntcValue);
//  Serial.print(", Resistance: ");
//  Serial.print(resistance);
//  Serial.print(" Ohms, Temperature: ");
//  Serial.print(calibratedTemperature);
//  Serial.println(" °C");
  
  return calibratedTemperature;
}

// Fungsi untuk kalibrasi suhu sesuai kebutuhan Anda
double kalibrasiSuhu(double suhuAwal) {
  // Tambahkan logika kalibrasi di sini
  // Contoh: return suhuAwal + offset;
  // Tambahkan logika kalibrasi di sini
  // Contoh: return suhuAwal + offset;
  double offset = 0.0; // Sesuaikan dengan nilai offset yang diperlukan
  return suhuAwal + offset;
}

float bacaAmmonia(int pin) {
  float VRL = analogRead(pin) * (3.3 / 10230);
  float RS = (3.3 / VRL - 1) * 10;
  float Ro = 28; // Ganti dengan nilai RO yang sesuai
  float ratio = RS / Ro;
  float ppm = pow(10, ((log10(ratio) - 0.858) / -0.417));
  return ppm;
}

float bacaPh(int pin) {
  // Baca nilai analog dari sensor pH
  int adcPH = analogRead(pin);

  // Konversi nilai analog menjadi tegangan
  float voltage = adcPH * (5.0 / 1023.0);

  // Konversi tegangan menjadi nilai pH
  // Sesuaikan kemiringan (slope) dan offset sesuai dengan karakteristik sensor Anda
  float pHValue = 7.0 - (voltage - 2.5); // Contoh sederhana, sesuaikan sesuai karakteristik sensor Anda

  // Pastikan nilai pH berada dalam rentang 0-14
  pHValue = constrain(pHValue, 0.0, 14.0);

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
