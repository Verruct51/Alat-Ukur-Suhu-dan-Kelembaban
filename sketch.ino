#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi dan server API
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* serverName = "http://110.239.93.223:5000/sensor";

// LCD dan DHT
LiquidCrystal_I2C lcd(0x27, 16, 2); 
#define DHTPIN 15       // Pin data sensor DHT
#define DHTTYPE DHT22   // Tipe sensor DHT22

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastTime = 0;
unsigned long timerDelay = 3000; // 3 detik

void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();      // Menyalakan backlight LCD
  lcd.begin(16, 2);     // Inisialisasi layar LCD
  lcd.print("Loading...");

  // Koneksi WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("Terhubung ke WiFi");
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= timerDelay) {
    // Membaca data dari sensor
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Menampilkan data di Serial Monitor
    Serial.print("Suhu: ");
    Serial.print(temp);
    Serial.print(" C, Kelembapan: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Menampilkan data di LCD
    lcd.setCursor(0, 0);
    lcd.print("Suhu: ");
    lcd.print(temp);
    lcd.print(" C   ");

    lcd.setCursor(0, 1);
    lcd.print("Kelembapan: ");
    lcd.print(humidity);
    lcd.print(" %  ");

    // Mengirim data ke server jika WiFi terhubung
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);

      // Buat JSON untuk dikirim
      String jsonData = "{\"suhu\": " + String(temp) + ", \"kelembapan\": " + String(humidity) + "}";
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(jsonData);

      // Menampilkan respons HTTP
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("HTTP Response code: " + String(httpResponseCode));
        Serial.println("Server Response: " + response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected. Trying to reconnect...");
      WiFi.begin(ssid, password);
    }

    lastTime = currentTime;
  }
}
