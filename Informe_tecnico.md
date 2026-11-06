# Informe Técnico: Sistema de Monitoreo e Integración IoT para Control de Luminosidad

**Asignatura:** Internet de las Cosas (IoT)  
**Proyecto:** Evaluación y Notificación de Niveles de Iluminación Ambiental  
**Rama del Repositorio:** `semana-13`  

---

## 1. Introducción e Requerimientos del Sistema

El presente informe documenta el diseño, la implementación y la validación de un sistema de monitoreo en tiempo real basado en tecnología IoT. El objetivo principal consiste en detectar variaciones críticas en la intensidad lumínica dentro de un entorno controlado (aula de clases / laboratorio) y notificar al usuario final de manera automática en su dispositivo móvil.

### Objetivos Específicos:
1. Capturar datos analógicos de un sensor de luz LDR utilizando el convertidor analógico-digital (ADC) de 12 bits de la tarjeta ESP32.
2. Publicar los datos leídos hacia la plataforma en la nube **Adafruit IO** utilizando el protocolo HTTP/REST.
3. Evaluar reglas de acción en la nube (Action Triggers) basadas en un umbral numérico específico ($> 3000$).
4. Desencadenar eventos Webhook vía solicitud `GET` hacia la plataforma **IFTTT** con parámetros dinámicos (`?value1=`).
5. Transmitir notificaciones Push en tiempo real hacia la aplicación móvil de IFTTT.

---

## 2. Descripción de la Arquitectura de Hardware y Conexión

### Componentes Utilizados:
* **Tarjeta de desarrollo:** ESP32-WROOM-32.
* **Sensor:** Fotorresistencia LDR (Light Dependent Resistor).
* **Resistencia fija:** $10\text{ k}\Omega$ (para acondicionamiento de la señal).
* **Fuente de alimentación:** $3.3\text{ V}$ provenientes del regulador embebido en la tarjeta ESP32.

### Circuito Acondicionador (Divisor de Voltaje):
La fotorresistencia LDR varía su resistencia inversamente a la cantidad de luz recibida. Para transformar esta variación de resistencia en un nivel de voltaje medible por el microcontrolador, se configuró un circuito en divisor de voltaje:

$$V_{out} = V_{CC} \cdot \left( \frac{R_{fija}}{R_{LDR} + R_{fija}} \right)$$

Donde:
* $V_{CC} = 3.3\text{ V}$
* $R_{fija} = 10\text{ k}\Omega$
* $V_{out}$ se conecta directamente al puerto **GPIO 34 (ADC)** de la ESP32.

---

## 3. Flujo de Datos y Protocolos de Comunicación

```
┌─────────────┐      Lectura ADC       ┌─────────────┐
│ Sensor LDR  │ ─────────────────────> │    ESP32    │
└─────────────┘                        └──────┬──────┘
                                              │
                                              │ HTTP POST / JSON
                                              ▼
                                       ┌─────────────┐
                                       │ Adafruit IO │ (Feed: luminosidad)
                                       └──────┬──────┘
                                              │
                                              │ Action Trigger (Valor > 3000)
                                              │ HTTP GET ?value1={value}
                                              ▼
                                       ┌─────────────┐
                                       │    IFTTT    │ (Maker Webhook)
                                       └──────┬──────┘
                                              │
                                              │ Notificación Push
                                              ▼
                                       ┌─────────────┐
                                       │ Dispositivo │
                                       │    Móvil    │
                                       └─────────────┘
```

1. **Lectura Local:** El ADC de la ESP32 muestrea el voltaje del sensor generando una lectura digital en un rango de $0$ a $4095$.
2. **Transmisión Cloud:** La tarjeta envía la información estructurada en JSON (`{"value": lectura}`) mediante una petición HTTP POST a la REST API de Adafruit IO.
3. **Procesamiento de Reglas (Adafruit IO Action):** Se define un disparador de acción de tipo Webhook con la siguiente lógica:
   * **Condición:** Si `luminosidad` es mayor a `3000` (indicador de baja iluminación).
   * **Acción:** Petición `GET` a `https://maker.ifttt.com/trigger/alerta_luz/with/key/SU_KEY?value1=` concatenando dinámicamente el valor del sensor (`Get Default: luminosidad`).
4. **Despliegue Móvil (IFTTT):** IFTTT captura el evento a través del parámetro de consulta `value1` e inyecta la variable directamente en la plantilla del mensaje push enviada al smartphone.

---

## 4. Pruebas y Validación del Sistema

Durante las pruebas de integración se validó la cadena completa de comunicación:

1. **Prueba de Umbral:** Se cubrió la fotorresistencia para incrementar el valor analógico por encima de los 3000 luxes simulados.
2. **Respuesta de Adafruit IO:** El feed registró las lecturas elevadas y ejecutó la acción inmediatamente al cruzar el umbral definido.
3. **Recepción en IFTTT:** La URL configurada con parámetro de consulta formateó el valor sin caracteres ambiguos ni marcas de posición no procesadas (`{{JsonPayload}}`).
4. **Notificación en Pantalla:** La notificación llegó al celular del usuario con el siguiente formato exitoso:
   > *ALERTA: Luminosidad baja en el aula. Valor actual: 3254 lux. Verificar iluminación.*

---

## 5. Conclusiones y Recomendaciones

1. **Robustez en la Integración Cloud:** El uso de parámetros `GET` (`?value1=`) demostró ser un método de integración altamente confiable entre Adafruit IO e IFTTT para evitar fallos de formato JSON en notificaciones móviles directas.
2. **Eficiencia Energética y de Red:** El sistema utiliza notificaciones basadas en eventos (*event-driven architecture*), lo que evita saturar la red o mantener conexiones de polling abiertas constantemente desde el cliente final.
3. **Trabajo Futuro:** Se recomienda implementar una histéresis en la lógica de disparo (Adafruit IO o firmware) para evitar el envío de múltiples notificaciones en caso de que la lectura oscile repetidamente alrededor del valor de 3000.