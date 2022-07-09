// Konfigurasi dan Otorisasi ke Blynk
#define BLYNK_TEMPLATE_ID "XXXXX"
#define BLYNK_DEVICE_NAME "XXXXX"
#define BLYNK_AUTH_TOKEN "XXXXX"

// Konfigurasi Host dan Otorisasi ke Firebase
#define FIREBASE_HOST "XXXXX"
#define FIREBASE_AUTH "XXXXX"

// Konfigurasi Wi-Fi SSID dan Password
#define WIFI_SSID "XXXXX"
#define WIFI_PASSWORD "XXXXX"

// Konfigurasi pin DHT11 dan LED
#define DHTPIN D6
#define redLed D5

// Konfigurasi pin Buzzer
#define buzzPin D7

// Konfigurasi tipe DHT ke DHT11
#define DHTTYPE DHT11

// Menyertakan library Firebase untuk ESP8266
#include "FirebaseESP8266.h"

// Menyertakan Library Wi-Fi ESP8266 dan DHT 
#include <ESP8266WiFi.h>
#include <DHT.h>

// Menyertakan library Blynk untuk ESP8266
#include <BlynkSimpleEsp8266.h>

// Inisialisasi FirebaseData dan Parse menggunakan JSON
FirebaseData firebaseData;
FirebaseData ledAuto;
FirebaseData buzzAuto;
FirebaseJson json;

// Inisialisasi DHT pin dan tipe
DHT dht(DHTPIN, DHTTYPE);

// Variabel untuk menyimpan data sensor, status, dan waktu
float h, t;
float hTemp, tTemp, ledAutoFloat, buzzAutoFloat, ledAutoTemp, buzAutoTemp;
bool dhtStatus, ledStatus = false;
int secSS, minMM, hrsHH = 0;

unsigned long theTime;
long beforeTime;

// Konfigurasi koneksi ke Blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

void setup() {
  // Membaca kecepatan serial (baud) sebesar 9600
  Serial.begin(9600);

  // Memulai DHT
  dht.begin();

  // Menginisialisasi pin LED sebagai keluaran
  pinMode(redLed, OUTPUT);

  // Memulai konektivitas Blynk
  Blynk.begin(auth, ssid, pass);

  // Memulai konektivitas Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

// Prosedur untuk membaca sensor DHT11
void sensorUpdate() { 
  // Menyimpan kelembaban dan suhu pada masing-masing variabel
  h = dht.readHumidity();
  t = dht.readTemperature();

  // Mencetak kedua data ke serial monitor
  Serial.printf("\nHum  = %d%%", h);
  Serial.printf("\nTem  = %.1f°C", t);

  // Tidak menggunakan fahrenheit
  float f = dht.readTemperature(false);
  // Kondisi ketika sensor tidak dapat membaca data
  if (isnan(h) || isnan(t) || isnan(f))
  {
    // Konsisi dibawah hanya dijalankan satu kali
    if (dhtStatus == true) {
      dhtStatus = false;
      Blynk.virtualWrite(V0, 0);
      Serial.printf("\nBlynk dhtStatus failed.");
    }
    return;
  } else {
    // Konsisi dibawah hanya dijalankan satu kali
    if (dhtStatus == false) {
      dhtStatus = true;
      Blynk.virtualWrite(V0, 1);
      Serial.printf("\nBlynk dhtStatus reading.");
    }
  }

  // Hanya jika terjadi perubahan data maka akan dikirimkan ke Blynk
  if (hTemp != h) {
    Blynk.virtualWrite(V5, h);
    Serial.printf("\nBlynk Hum updated.");
  }
  if (tTemp != t) {
    Blynk.virtualWrite(V4, t);
    Serial.printf("\nBlynk Tem updated.");
  }
  hTemp = h;
  tTemp = t;
}

// Prosedur untuk membaca aktuator LED dan Buzzer
void actuatorUpdate() {
  Firebase.getInt(ledAuto, "/dht11/status/led");
  Firebase.getInt(buzzAuto, "/dht11/status/buzzer");

  // Ubah data FirebaseData ke Float
  ledAutoFloat = ledAuto.floatData();
  buzzAutoFloat = buzzAuto.floatData();

  // Hanya jika terjadi perubahan data maka akan dikirimkan ke Blynk
  if (ledAutoTemp != ledAutoFloat) {
    Blynk.virtualWrite(V1, ledAutoFloat);
    Serial.printf("\nBlynk LED updated.");
  }
  ledAutoTemp = ledAutoFloat;

  if (buzAutoTemp != buzzAutoFloat) {
    Blynk.virtualWrite(V2, buzzAutoFloat);
    Serial.printf("\nBlynk Buzzer updated.");
  }
  buzAutoTemp = buzzAutoFloat;
}

// Prosedur untuk menyimpan data ke Firebase
void updateDataToFirebase() {
  Firebase.setFloat(firebaseData, "/dht11/humidity", dht.readHumidity());
  Firebase.setFloat(firebaseData, "/dht11/temperature", dht.readTemperature());
}

void automationLedBuzzer() {
  Firebase.getInt(ledAuto, "/dht11/status/led");
  Firebase.getInt(buzzAuto, "/dht11/status/buzzer");

  Serial.printf("\nledAuto  = %d", ledAuto.intData());
  Serial.printf("\nbuzzAuto = %d", buzzAuto.intData());

  if (ledAuto.intData() == 1) {
    digitalWrite(redLed, HIGH);
  } else {
    digitalWrite(redLed, LOW);
  }
  if (buzzAuto.intData() == 1) {
    tone(buzzPin, 700, 500);
  } else {
    noTone(buzzPin);
  }
}

void loop() {
  // Jalankan sensor update setiap iterasi
  sensorUpdate();
  // Jalankan Blynk  
  Blynk.run();

  // Jalankan aktuator update setiap iterasi
  actuatorUpdate();

  // Untuk membuat penghitung waktu
  theTime = millis() / 1000;
  if (theTime > beforeTime) {
    secSS++;
    if (secSS >= 60) {
      secSS = 0;
      minMM++;
      if (minMM >= 60) {
        minMM = 0;
        hrsHH++;
        if (hrsHH >= 99) {
          secSS = 0;
          minMM = 0;
          hrsHH = 0;
        }
      }
    }
    beforeTime = theTime;
  }
  Serial.printf("\nTime = %02d:%02d:%02d\n", hrsHH, minMM, secSS);

  // Kondisi ketika program telah berjalan 10 detik, maka akan memperbarui data ke Firebase  
  if (theTime % 10 == 0) { updateDataToFirebase(); }

  // Otomasi
  automationLedBuzzer();
  
  delay(1000);
}
