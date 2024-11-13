#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define DHTPIN 4
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

// Inisialisasi LED dan Aktuator
#define LED_HIJAU 5
#define LED_KUNING 17
#define LED_MERAH 16
#define RELAY_POMPA 14  
#define BUZZER 15

const char* ssid = "King";
const char* password = "devit123";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(RELAY_POMPA, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  dht.begin();

  // Koneksi WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("Terhubung ke WiFi");

  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Menghubungkan ke MQTT...");
    if (client.connect("ESP32_Client")) {
      Serial.println("Terhubung ke MQTT broker");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (suhu > 35) {
    digitalWrite(LED_MERAH, HIGH);
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_HIJAU, LOW);
  } else if (suhu >= 30 && suhu <= 32) {
    digitalWrite(LED_KUNING, HIGH);
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(BUZZER, LOW);
  } else if (suhu < 30) {
    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(BUZZER, LOW);
  }

  digitalWrite(RELAY_POMPA, HIGH);

  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C, Kelembapan: ");
  Serial.print(kelembapan);
  Serial.println(" %");

  if (client.connected()) {
    String suhuStr = String(suhu);
    String kelembapanStr = String(kelembapan);
    client.publish("hidroponik/suhu", suhuStr.c_str());
    client.publish("hidroponik/kelembapan", kelembapanStr.c_str());
  }

  delay(5000); 
}
