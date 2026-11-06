#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "config.h"

// --- Configuración de Pines y Sensores ---
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int LDR_PIN = 34;
const int BUZZER_PIN = 27;
const int LED_ROJO = 25;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 10000; // Enviar datos cada 10 segundos

// Declaración de tópicos (Estructura usuario/feeds/nombre_feed)
const char* topic_temp = AIO_USERNAME "/feeds/temperatura";
const char* topic_hum  = AIO_USERNAME "/feeds/humedad";
const char* topic_ldr  = AIO_USERNAME "/feeds/luminosidad";
const char* topic_led  = AIO_USERNAME "/feeds/led-rojo";
const char* topic_buz  = AIO_USERNAME "/feeds/buzzer";

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim(); // Elimina espacios extras o saltos de línea

  Serial.print("Mensaje recibido en [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Control remoto del LED Rojo
  String strTopicLed = String(AIO_USERNAME) + "/feeds/led-rojo";
  if (String(topic) == strTopicLed) {
    if (message == "1" || message == "ON" || message == "on") {
      digitalWrite(LED_ROJO, HIGH);
      Serial.println("-> LED Rojo ENCENDIDO");
    } else if (message == "0" || message == "OFF" || message == "off") {
      digitalWrite(LED_ROJO, LOW);
      Serial.println("-> LED Rojo APAGADO");
    }
  }

  // Control remoto del Buzzer (soporta activo y pasivo)
  String strTopicBuz = String(AIO_USERNAME) + "/feeds/buzzer";
  if (String(topic) == strTopicBuz) {
    if (message == "1" || message == "ON" || message == "on") {
      digitalWrite(BUZZER_PIN, HIGH);
      tone(BUZZER_PIN, 1000); // Emite un tono a 1kHz por si es un buzzer pasivo
      Serial.println("-> Buzzer ACTIVADO");
    } else if (message == "0" || message == "OFF" || message == "off") {
      digitalWrite(BUZZER_PIN, LOW);
      noTone(BUZZER_PIN); // Apaga el tono del buzzer
      Serial.println("-> Buzzer DESACTIVADO");
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32_AulaUTS", AIO_USERNAME, AIO_KEY)) {
      Serial.println("conectado");
      // Suscribirse a los tópicos de control (actuadores)
      client.subscribe(topic_led);
      client.subscribe(topic_buz);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 2 segundos");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  dht.begin();
  setup_wifi();
  
  client.setServer(AIO_SERVER, AIO_SERVERPORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int ldrValue = analogRead(LDR_PIN);

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Error leyendo el sensor DHT11!");
      return;
    }

    // Publicar lecturas hacia Adafruit IO
    client.publish(topic_temp, String(temp).c_str());
    client.publish(topic_hum, String(hum).c_str());
    client.publish(topic_ldr, String(ldrValue).c_str());

    Serial.println("Datos publicados en Adafruit IO:");
    Serial.print("Temp: "); Serial.print(temp); Serial.println(" °C");
    Serial.print("Humedad: "); Serial.print(hum); Serial.println(" %");
    Serial.print("Luminosidad: "); Serial.println(ldrValue);
  }
}