# Proyecto IoT: Alerta de Luminosidad en Tiempo Real (Semana 13)

Este proyecto implementa un sistema de monitoreo de iluminación ambiental en tiempo real diseñado para aulas de clase y entornos académicos. Utiliza una placa **ESP32** para leer los datos de un sensor analógico de luz (**LDR**), transmite las lecturas a la plataforma **Adafruit IO** a través de protocolo MQTT/REST y activa alertas móviles push en tiempo real mediante **IFTTT** cuando la luminosidad cae por debajo de los umbrales configurados.

---

## 🏗️ Arquitectura del Sistema

```
[ Sensor LDR ] ---> (Lectura Analógica ADC) 
                       │
                       ▼
                 [ ESP32 ] 
                       │
                       ▼ (Conexión Wi-Fi / REST API)
               [ Adafruit IO ] 
                       │
                       ▼ (Trigger de Acción / Evento > 3000)
                 [ IFTTT ] 
                       │
                       ▼ (Webhook GET ?value1=)
         [ Notificación Push Móvil ]
```

---

## 🛠️ Requisitos del Sistema y Componentes

### Hardware
- Microcontrolador ESP32-WROOM-32.
- Fotorresistencia LDR.
- Resistencia de $10\text{ k}\Omega$ (configuración en divisor de tensión).
- Protoboard y cables de conexión DuPont.

### Software y Servicios Cloud
- **Arduino IDE** (con soporte para tarjetas ESP32).
- **Adafruit IO**: Feed de datos (`luminosidad`) y Action Triggers.
- **IFTTT**: Applet Webhook integrado con servicio de notificaciones push.

---

## 📐 Diagrama de Conexión del Sensor

El sensor LDR está conectado en configuración de divisor de voltaje al pin analógico del ESP32:

* **VCC ($3.3\text{ V}$):** Un extremo del sensor LDR.
* **GND:** Un extremo de la resistencia de $10\text{ k}\Omega$.
* **Pin Analógico (GPIO 34 / ADC):** Punto medio entre el LDR y la resistencia de $10\text{ k}\Omega$.

---

## 📂 Estructura del Repositorio

```text
proyecto-iot-semana13/
├── README.md
├── informe_tecnico.md
├── codigo/
│   └── esp32_luminosidad.ino
└── evidencias/
    ├── applet_ifttt.png
    ├── notificacion_push.png
    ├── grafico_historico.png
    └── dashboard_adafruit.png
```

---

## 🔄 Explicación del Flujo de Datos

1. **Captura:** El ESP32 lee la entrada analógica mediante el ADC (valores entre $0$ y $4095$).
2. **Publicación:** El ESP32 transmite el valor mediante HTTP GET/POST a la API de Adafruit IO en la variable `luminosidad`.
3. **Evaluación de Alerta:** La acción en Adafruit IO evalúa si la lectura de la fotorresistencia supera el umbral estipulado ($> 3000$, correspondiente a baja iluminación).
4. **Disparo de Webhook:** Al cumplirse la condición, Adafruit IO invoca la URL del Webhook en IFTTT pasando el parámetro dinámico por URL:
   `https://maker.ifttt.com/trigger/alerta_luz/with/key/SU_KEY?value1=`
5. **Notificación:** IFTTT recibe el parámetro `value1` y despliega la notificación push en el dispositivo móvil registrado.

---
## 📸 Capturas de pantalla requeridas:
 - Creacion del feed
  <img width="600" height="800" alt="creacion_feed" src="https://github.com/user-attachments/assets/b49a0f7e-2a4e-4378-93af-77ec56a98c87" />
  
 - Grafico histórico generado
   <img width="600" height="800" alt="grafico_historico" src="https://github.com/user-attachments/assets/12f5555b-834a-464b-8e3a-d3c7cfa67226" />
   
 - Applet configurado
   <img width="600" height="800" alt="applet_configurado" src="https://github.com/user-attachments/assets/e5288274-302e-4ec0-bdc3-91516368395c" />
   
 - Notificación push
   <img width="600" height="800" alt="Notificacion_push" src="https://github.com/user-attachments/assets/000c16f4-7ae6-4a2c-97d6-6b89b4f7a01d" />





---

## 📸 Evidencias de Funcionamiento

Las capturas de pantalla de la ejecución y validación del sistema se encuentran organizadas en la carpeta `/evidencias`:

- `dashboard_adafruit.png`: Vista general del panel de control de datos en Adafruit IO.
- `grafico_historico.png`: Gráfico con el comportamiento temporal de las lecturas.
- `applet_ifttt.png`: Configuración del webhook y del disparador en IFTTT.
- `notificacion_push.png`: Notificación recibida en el celular con el dato exacto del sensor.

---

## 💻 Código Fuente Comentado (`esp32_luminosidad.ino`)

```cpp
#include <WiFi.h>
#include <HTTPClient.h>

// Configuración de red Wi-Fi
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_PASSWORD";

// Configuración Adafruit IO
const char* aio_username = "TU_USUARIO_ADAFRUIT";
const char* aio_key = "TU_AIO_KEY";
const char* feed_key = "luminosidad";

// Pin asignado al sensor LDR
const int LDR_PIN = 34;

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);

  // Conexión a la red inalámbrica
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConexión establecida.");
}

void loop() {
  // Lectura del valor del sensor LDR (ADC de 12 bits: 0 a 4095)
  int valLDR = analogRead(LDR_PIN);
  Serial.print("Lectura LDR: ");
  Serial.println(valLDR);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // URL REST API de Adafruit IO para actualizar el feed
    String url = "https://io.adafruit.com/api/v2/" + String(aio_username) + "/feeds/" + String(feed_key) + "/data";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-AIO-Key", aio_key);

    // Formato de payload JSON requerido por Adafruit IO
    String payload = "{\"value\":" + String(valLDR) + "}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Dato enviado exitosamente. Código respuesta: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error al enviar dato: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  // Intervalo de transmisión (cada 5 segundos)
  delay(5000);
}
```

---

## 🎓 Conclusiones sobre la Automatización en IoT

1. **Desacoplamiento e Integración Cloud:** La combinación de Adafruit IO e IFTTT permite separar la lógica de lectura del dispositivo físico de la lógica de notificación, reduciendo la carga de procesamiento en el ESP32.
2. **Monitoreo Eficiente y Alertas Proactivas:** El uso de webhooks basados en disparadores por eventos evita la necesidad de hacer solicitudes persistentes (polling) desde el teléfono móvil, optimizando el consumo energético e infraestructura.
3. **Escalabilidad:** Esta arquitectura modular sienta las bases para integrar actuadores automáticos (como relés de iluminación) activados de forma remota ante las mismas alertas.
