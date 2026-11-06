#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// --- Configuración de Pines ---
const int LDR_PIN = 34; // Sensor de luminosidad LDR[cite: 20]

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 10000; // Envío cada 15 segundos para IFTTT

// Tópico MQTT para Adafruit IO
const char* topic_ldr = AIO_USERNAME "/feeds/luminosidad";

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Conectado");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT con Adafruit IO...");
    if (client.connect("ESP32_Luminosidad_UTS", AIO_USERNAME, AIO_KEY)) {
      Serial.println(" ¡Conectado!");
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  setup_wifi();
  client.setServer(AIO_SERVER, AIO_SERVERPORT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // Lectura analógica de la LDR
    int ldrValue = analogRead(LDR_PIN);

    // Publicación en el feed luminosidad[cite: 20]
    if (client.publish(topic_ldr, String(ldrValue).c_str())) {
      Serial.print("Luminosidad publicada con éxito: ");
      Serial.println(ldrValue);
    } else {
      Serial.println("Error al publicar en Adafruit IO");
    }
  }
}