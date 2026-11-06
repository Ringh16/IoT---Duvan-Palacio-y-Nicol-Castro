
# 🌡️ Sistema IoT de Monitoreo Ambiental con ESP32 y Adafruit IO

Este repositorio contiene la implementación de un sistema IoT funcional para el monitoreo en tiempo real de variables ambientales (temperatura, humedad y luminosidad) y el control remoto de actuadores (LED y Buzzer) mediante la plataforma en la nube **Adafruit IO** y el protocolo **MQTT**.

---

## 🏗️ 1. Diagrama de Arquitectura de Integración Cloud

El flujo de información y control bidireccional del proyecto sigue la estructura presentada a continuación:

```text
[ Sensor DHT11 ] ──┐
                   ├──> [ ESP32 DevKit ] <──(MQTT 1883)──> [ Broker Cloud ] <──> [ Dashboard Web / Mobile ]
[ Sensor LDR ]   ──┘    (Pin 25 & 27)                       (Adafruit IO)         (Widgets / Controls)
                           │  ▲
                           ▼  │
                     [ Actuadores ]
                     (LED / Buzzer)
```

---

## 📌 2. Definición de Pines y Componentes Hardware

| Componente | Tipo | Pin ESP32 | Descripción / Función |
| :--- | :--- | :--- | :--- |
| **DHT11** | Sensor Entrada | `GPIO 4` | Lectura de Temperatura (°C) y Humedad (%) |
| **LDR** | Sensor Entrada | `GPIO 34` | Lectura analógica de Nivel de Luminosidad |
| **LED Rojo** | Actuador Salida | `GPIO 25` | Alerta visual controlada remotamente |
| **Buzzer** | Actuador Salida | `GPIO 27` | Alerta sonora controlada remotamente |

---

## 📡 3. Definición de Flujos de Datos (Feeds Adafruit IO)

| Nombre del Feed | Dirección | Descripción del Dato / Acción |
| :--- | :--- | :--- |
| `temperatura` | Lectura (ESP32 ➔ Cloud) | Envío continuo de temperatura ambiental |
| `humedad` | Lectura (ESP32 ➔ Cloud) | Envío continuo de humedad relativa |
| `luminosidad` | Lectura (ESP32 ➔ Cloud) | Envío continuo del valor de luz (LDR) |
| `led-rojo` | Escritura/Control (Cloud ➔ ESP32) | Control remoto del LED Rojo (`ON`/`OFF` o `1`/`0`) |
| `buzzer` | Escritura/Control (Cloud ➔ ESP32) | Control remoto del Buzzer (`ON`/`OFF` o `1`/`0`) |

---

## 🛠️ 4. Instrucciones para Replicar el Proyecto

1. **Clonar el Repositorio:**
   ```bash
   git clone https://github.com/Ringh16/IoT---Duvan-Palacio-y-Nicol-Castro.git
   ```

2. **Configurar Credenciales Seguras:**
   - Haz una copia del archivo de plantilla `config.example.h` y renómbralo a `config.h`.
   - Abre `config.h` y completa con los datos de tu red WiFi y tus claves de Adafruit IO:
     ```cpp
     #define WIFI_SSID     "Tu_Red_WiFi"
     #define WIFI_PASS     "Tu_Contraseña_WiFi"
     #define AIO_USERNAME  "Tu_Usuario_Adafruit"
     #define AIO_KEY       "Tu_AIO_Key_Privada"
     ```

3. **Librerías Requeridas en Arduino IDE:**
   - `PubSubClient` (por Nick O'Leary)
   - `DHT sensor library` (por Adafruit)
   - `Adafruit Unified Sensor` (por Adafruit)

4. **Compilación y Carga:**
   - Selecciona la placa **ESP32 Dev Module**.
   - Compila y sube el boceto `semana12arduino.ino`.

---

## 📊 5. Evidencias del Dashboard en Adafruit IO

<img width="1361" height="830" alt="Captura de pantalla 2026-11-05 212016" src="https://github.com/user-attachments/assets/e4e68082-410f-498a-aa95-3c77f7bf027d" />

- **Gráfico de Línea:** Histórico de Temperatura.
- **Gauge / Display Numérico:** Visualización de Humedad actual, luminosidad y temperatura actual.
- **Toggle Switches:** Interruptores independientes para el LED Rojo y el Buzzer.

---

## 🔒 6. Seguridad de Credenciales
Las credenciales sensibles (SSID, contraseña de WiFi y la AIO Key de producción) se mantienen protegidas mediante la exclusión del archivo `config.h` en el control de versiones a través de `.gitignore`, cumpliendo con las buenas prácticas de seguridad.
