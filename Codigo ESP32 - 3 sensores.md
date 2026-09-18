#include <DHT.h>

// Definición de pines
#define DHTPIN 13       // Pin de datos para el DHT11
#define DHTTYPE DHT11   // Tipo de sensor

#define LDRPIN 4        // Pin analógico (ADC) para el módulo LDR

#define TRIGPIN 5       // Pin Trigger del HC-SR04
#define ECHOPIN 18      // Pin Echo del HC-SR04

// Inicialización del sensor DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Inicialización de la comunicación serial a 115200 baudios
  Serial.begin(115200);
  
  // Configuración del sensor DHT
  dht.begin();
  
  // Configuración de pines para el ultrasónico
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
  
  // Configuración del pin del LDR
  pinMode(LDRPIN, INPUT);
  
  Serial.println("--- Sistema de Monitoreo Multisensor Inicializado ---");
}

void loop() {
  // 1. Lectura del DHT11
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  // 2. Lectura del LDR (ADC de 12 bits: 0 - 4095)
  int luz = analogRead(LDRPIN);
  
  // 3. Lectura del HC-SR04 (Medición de tiempo y cálculo de distancia)
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  
  long duracion = pulseIn(ECHOPIN, HIGH);
  float distancia = (duracion * 0.034) / 2.0; // Fórmula: tiempo * velocidad del sonido / 2
  
  // 4. Salida formateada al Monitor Serial
  Serial.println("=== MONITOREO AULA ===");
  
  // Validación de lectura correcta del DHT11
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error al leer el sensor DHT11!");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temp, 1);
    Serial.println(" C");
    
    Serial.print("Humedad: ");
    Serial.print(hum, 1);
    Serial.println(" %");
  }
  
  Serial.print("Luz: ");
  Serial.println(luz);
  
  Serial.print("Distancia: ");
  Serial.print(distancia, 1);
  Serial.println(" cm");
  
  // Condición lógica de ocupación (umbral de 30 cm)
  if (distancia < 30.0 && distancia > 0) {
    Serial.println("Estado: Ocupado (distancia < 30 cm)");
  } else {
    Serial.println("Estado: Libre");
  }
  
  Serial.println("---------------------");
  
  // Tasa de refresco de 2 segundos entre lecturas
  delay(2000);
}
