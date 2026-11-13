#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

// --- CREDENCIALES DE WIFI ---
#define WLAN_SSID       "wifi name"       // Cambia por el nombre de tu red WiFi
#define WLAN_PASS       "wifi pss"   // Cambia por la contraseña de tu WiFi

// --- CREDENCIALES DE ADAFRUIT IO ---
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Duvs16"
#define AIO_KEY         "token adafruit

// --- CONFIGURACIÓN DE PINES ---
#define DHTPIN 4          // Pin digital conectado al DHT11
#define DHTTYPE DHT11     // Definir tipo de sensor DHT
#define LDR_PIN 34        // Pin analógico conectado al LDR

DHT dht(DHTPIN, DHTTYPE);

// Configuración cliente MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Configuración de los Feeds
Adafruit_MQTT_Publish temperatura = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish luminosidad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/luminosidad");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  // Inicializar sensores
  dht.begin();
  pinMode(LDR_PIN, INPUT);

  Serial.println(F("Conectando a WiFi..."));
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(F("WiFi conectado!"));
}

void loop() {
  // Mantener conexión MQTT activa
  MQTT_connect();

  // Leer temperatura del DHT11
  float temp = dht.readTemperature();
  // Leer valor analógico del LDR
  int ldrValue = analogRead(LDR_PIN);

  // Validar si la lectura del DHT11 es correcta
  if (isnan(temp)) {
    Serial.println(F("Error al leer el sensor DHT11!"));
  } else {
    Serial.print(F("Enviando temperatura: "));
    Serial.print(temp);
    Serial.print(F(" °C  |  Luminosidad: "));
    Serial.println(ldrValue);

    // Publicar en Adafruit IO
    if (!temperatura.publish(temp)) {
      Serial.println(F("Fallo al enviar temperatura"));
    }
    
    // AQUÍ ESTÁ EL CAMBIO (forzar a int32_t)
    if (!luminosidad.publish((int32_t)ldrValue)) {
      Serial.println(F("Fallo al enviar luminosidad"));
    }
  }

  // Esperar 10 segundos antes de la siguiente lectura
  delay(10000);
}

void MQTT_connect() {
  int ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Conectando a MQTT... "));
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println(F("Reintentando conexión MQTT en 5 segundos..."));
    mqtt.disconnect();
    delay(5000); 
    retries--;
    if (retries == 0) {
      while (1); 
    }
  }
  Serial.println(F("MQTT Conectado!"));
}