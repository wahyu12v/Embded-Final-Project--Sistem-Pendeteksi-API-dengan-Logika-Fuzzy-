
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR 0x27

#define LCD_COLS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

const int buzzerPin = 26;       
const int sensor_suhu = 27;
const int sensor_waktu = 14;
const float beta = 3950;
const float resistor = 10000.0;
const float kelvinBase = 273.15;
const int suhu_minimal = 20;
const int suhu_maksimal = 45;
const int waktu_minimal = 180;
const int waktu_maksimal = 285;
const int buzzerVolume = 200;   

void setup() {
  Wire.begin();
  Serial.begin(115200);

  lcd.init();
  lcd.backlight(); 
  lcd.clear();    

  pinMode(buzzerPin, OUTPUT); 
  digitalWrite(buzzerPin, LOW); 
}

void loop() {
  int nilai_suhu_adc = analogRead(sensor_suhu);
  int nilai_waktu_adc = analogRead(sensor_waktu);

  float resistance = resistor / ((4095.0 / nilai_suhu_adc) - 1);

  float steinhart;
  steinhart = resistance / resistor;
  steinhart = log(steinhart);
  steinhart /= beta;
  steinhart += 1.0 / (25 + kelvinBase);
  steinhart = 1.0 / steinhart;
  steinhart -= kelvinBase;

  float x = steinhart;
  if (steinhart < suhu_minimal) {
    x = suhu_minimal;
  } else if (steinhart > suhu_maksimal) {
    x = suhu_maksimal;
  }

  float rendah = 20;
  float tinggi = 45;

  Serial.print("Nilai Sensor Suhu (20 - 45 ): ");
  Serial.println(x);
  
  float Suhu_Rendah;
  if (x > tinggi) {
    Suhu_Rendah = 0;
  } else if (rendah < x && x < tinggi) {
    Suhu_Rendah = (tinggi - x) / (tinggi - rendah);
  } else if (x < rendah) {
    Suhu_Rendah = 1;
  }

  float Suhu_Tinggi;
  if (x < rendah) {
    Suhu_Tinggi = 0;
  } else if (x >= rendah && x <= tinggi) {
    Suhu_Tinggi = (x - rendah) / (tinggi - rendah);
  } else if (x > tinggi) {
    Suhu_Tinggi = 1;
  }

  Serial.println("Suhu_Rendah [X]: " + String(Suhu_Rendah, 4));
  Serial.println("Suhu_Tinggi [X]: " + String(Suhu_Tinggi, 4));

  float y = map(nilai_waktu_adc, 0, 4095, waktu_minimal, waktu_maksimal);
  float singkat = 180;
  float lama = 285;

  Serial.print("Nilai sensor waktu (180 - 285 detik): ");
  Serial.println(y);

  float waktu_singkat;
  if (y >= lama) {
    waktu_singkat = 0;
  } else if (singkat < y && y < lama) {
    waktu_singkat = (lama - y) / (lama - singkat);
  } else if (y <= singkat) {
    waktu_singkat = 1;
  }

  float waktu_lama;
  if (y <= singkat) {
    waktu_lama = 0;
  } else if (y >= singkat && y <= lama) {
    waktu_lama = (y - singkat) / (lama - singkat);
  } else if (y >= lama) {
    waktu_lama = 1;
  }

  Serial.println("Waktu_Singkat [Y]: " + String(waktu_singkat, 4));
  Serial.println("Waktu_Lama [Y]: " + String(waktu_lama, 4));

  float a1 = min(Suhu_Rendah, waktu_singkat); // α-predikat1
  float Z1 = a1 * (170 - 50) + 50; // Perhitungan Z1

  Serial.print("A1 : ");
  Serial.println(a1, 4);
  Serial.print("Z1 : ");
  Serial.println(Z1);

  float a2 = min(Suhu_Rendah, waktu_lama); // α-predikat2
  float Z2 = 170 - a2 * (170 - 50); // Perhitungan Z2

  Serial.print("A2 : ");
  Serial.println(a2, 4);
  Serial.print("Z2 : ");
  Serial.println(Z2);

  float a3 = min(Suhu_Tinggi, waktu_singkat); // α-predikat3
  float Z3 = 170 - a3 * (170 - 50); // Perhitungan Z3

  Serial.print("A3 : ");
  Serial.println(a3, 4);
  Serial.print("Z3 : ");
  Serial.println(Z3);

  float a4 = min(Suhu_Tinggi, waktu_lama); // α-predikat4
  float Z4 = a4 * (170 - 50) + 50; // Perhitungan Z4

  Serial.print("A4 : ");
  Serial.println(a4, 4);
  Serial.print("Z4 : ");
  Serial.println(Z4);

  float z = (a1 * Z1 + a2 * Z2 + a3 * Z3 + a4 * Z4) / (a1 + a2 + a3 + a4);

  float mu_aman = (170 - z) / 120.0;
  float mu_bahaya = (z - 50) / 120.0;

  lcd.setCursor(0, 0);
  lcd.print("Nilai Z:");
  lcd.setCursor(0, 1);
  lcd.print("                "); // Menghapus pesan sebelumnya
  lcd.setCursor(0, 1);
  lcd.print(z);

  Serial.print("Nilai Z: ");
  Serial.println(z);
  Serial.print("µ AMAN[Z]: ");
  Serial.println(mu_aman, 4);
  Serial.print("µ BAHAYA[Z]: ");
  Serial.println(mu_bahaya, 4);

  if (z > 110) {
    lcd.setCursor(0, 0);
    lcd.print("Indikasi");
    lcd.setCursor(0, 1);
    lcd.print("Kebakaran!!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Kebakaran");
    lcd.setCursor(0, 1);
    lcd.print("Tidak Ada");
  }

  if (z > 110) {
    analogWrite(buzzerPin, buzzerVolume); 
    analogWrite(buzzerPin, 0); 
  }

  delay(1000);
}
