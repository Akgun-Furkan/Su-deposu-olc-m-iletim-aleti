/*
  SU DEPOSU SEVIYE TAKIP SISTEMI - VERICI (TANK TARAFI)
  =========================================================
  Kullanilan malzemeler:
    - Arduino Uno / Nano
    - JSN-SR04T su gecirmez ultrasonik mesafe sensoru
    - LoRa modulu (SX1278 / Ra-02, 433MHz)

  KURULUM: Sensor, deponun EN UST noktasina, tam asagi (su yuzeyine)
  bakacak sekilde monte edilir.

  Kutuphane: "LoRa" by Sandeep Mistry (Arduino Kutuphane Yoneticisi'nden kurun)
  =========================================================
*/

#include <SPI.h>
#include <LoRa.h>

// ---- LORA PIN BAGLANTILARI (Arduino Uno/Nano) ----
#define LORA_SS   10
#define LORA_RST  9
#define LORA_DIO0 2

// ---- ULTRASONIK SENSOR PINLERI ----
#define TRIG_PIN  4
#define ECHO_PIN  5

// ==========================================================
// DEPO OLCULERI - KENDI DEPONUZA GORE BU 3 DEGERI DUZENLEYIN
// ==========================================================
const float TANK_UZUNLUK = 100.0;          // deponun ic taban uzunlugu (cm)
const float TANK_GENISLIK = 80.0;          // deponun ic taban genisligi (cm)
const float SENSOR_TABAN_MESAFESI = 150.0; // sensorden BOS depo tabanina mesafe (cm)
                                            // -> Sensoru taktiktan sonra, depo
                                            //    bosken bir kez olcup buraya girin.
// ==========================================================

const int OLCUM_SAYISI = 5;                    // gurultuyu azaltmak icin art arda olcum
const unsigned long GONDERIM_ARALIGI = 60000;  // her 60 saniyede bir veri gonder

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa baslatilamadi, baglantilari kontrol edin.");
    while (1);
  }
  Serial.println("LoRa verici hazir.");
}

float mesafeOlcCM() {
  float toplam = 0;
  int gecerliOlcum = 0;

  for (int i = 0; i < OLCUM_SAYISI; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long sure = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (~5m menzil)
    if (sure > 0) {
      toplam += sure * 0.0343 / 2.0;  // cm cinsinden mesafe
      gecerliOlcum++;
    }
    delay(60);
  }

  if (gecerliOlcum == 0) return -1;  // olcum basarisiz
  return toplam / gecerliOlcum;
}

void loop() {
  float mesafe = mesafeOlcCM();

  if (mesafe > 0) {
    float suYuksekligi = SENSOR_TABAN_MESAFESI - mesafe;
    if (suYuksekligi < 0) suYuksekligi = 0;
    if (suYuksekligi > SENSOR_TABAN_MESAFESI) suYuksekligi = SENSOR_TABAN_MESAFESI;

    float hacimCM3 = TANK_UZUNLUK * TANK_GENISLIK * suYuksekligi;
    float litre = hacimCM3 / 1000.0;
    float doluluk = (suYuksekligi / SENSOR_TABAN_MESAFESI) * 100.0;

    String veri = String(litre, 1) + ";" + String(doluluk, 0);

    LoRa.beginPacket();
    LoRa.print(veri);
    LoRa.endPacket();

    Serial.print("Gonderildi -> ");
    Serial.print(litre, 1);
    Serial.print(" litre  (doluluk: %");
    Serial.print(doluluk, 0);
    Serial.println(")");
  } else {
    Serial.println("Sensor okuma hatasi - baglantilari kontrol edin.");
  }

  delay(GONDERIM_ARALIGI);
}
