/*
  SU DEPOSU SEVIYE TAKIP SISTEMI - ALICI (EV / KONTROL NOKTASI TARAFI)
  ========================================================================
  Kullanilan malzemeler:
    - Arduino Uno / Nano (WiFi'ye baglamak isterseniz ileride ESP32 ile
      degistirebilirsiniz)
    - LoRa modulu (SX1278 / Ra-02, 433MHz) - VERICI ILE AYNI FREKANS

  Bu temel surum, gelen veriyi Seri Port'a yazdirir. Isterseniz bir
  OLED/LCD ekran ekleyip orada gosterebilir, ya da ESP32 kullanip WiFi
  uzerinden telefonunuza/buluta aktarabilirsiniz.

  Kutuphane: "LoRa" by Sandeep Mistry (Arduino Kutuphane Yoneticisi'nden kurun)
  ========================================================================
*/

#include <SPI.h>
#include <LoRa.h>

#define LORA_SS   10
#define LORA_RST  9
#define LORA_DIO0 2

void setup() {
  Serial.begin(9600);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa baslatilamadi, baglantilari kontrol edin.");
    while (1);
  }
  Serial.println("LoRa alici hazir, veri bekleniyor...");
}

void loop() {
  int paketBoyutu = LoRa.parsePacket();
  if (paketBoyutu) {
    String veri = "";
    while (LoRa.available()) {
      veri += (char)LoRa.read();
    }

    // Veri formati: "LITRE;DOLULUK"
    int ayracIndex = veri.indexOf(';');
    if (ayracIndex > 0) {
      String litreStr = veri.substring(0, ayracIndex);
      String dolulukStr = veri.substring(ayracIndex + 1);

      Serial.print("Su seviyesi: ");
      Serial.print(litreStr);
      Serial.print(" litre  (doluluk: %");
      Serial.print(dolulukStr);
      Serial.println(")");

      // Sinyal gucu (RSSI) - menzil test/ayarlamasi icin faydalidir
      Serial.print("Sinyal gucu (RSSI): ");
      Serial.println(LoRa.packetRssi());
    }
  }
}
