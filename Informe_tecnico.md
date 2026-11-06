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
## 5. Preguntas de reflexión
### 5.1 ¿Qué ventaja ofrece IFTTT frente a programar el envío de correos directamente desde el ESP32?

Enviar un correo desde el ESP32 es técnicamente posible (SMTP), pero implica cargar al microcontrolador con responsabilidades que no le corresponden. IFTTT ofrece ventajas en varios frentes:

*a) Complejidad y recursos del firmware.*
SMTP seguro requiere TLS, manejo de certificados, autenticación y gestión de errores. Todo eso consume memoria flash y RAM, y suele ejecutarse de forma bloqueante, con el riesgo de que el ESP32 deje de atender el bucle MQTT y se pierda la conexión. Con IFTTT, el ESP32 solo hace lo que sabe hacer bien: *medir y publicar*.

*b) Seguridad de las credenciales.*
Un envío directo exige guardar en el firmware el usuario y la contraseña (o contraseña de aplicación) de una cuenta de correo. Si el código se sube a GitHub o se extrae del dispositivo, esas credenciales quedan expuestas. Además, proveedores como Gmail restringen el acceso de aplicaciones menos seguras y obligan a usar contraseñas de aplicación o OAuth. Con IFTTT, la autorización se gestiona en la nube mediante los flujos de cada servicio.

*c) Desacople y mantenibilidad ("no reprogramar para cambiar la regla").*
Con la alerta en el firmware, cambiar el umbral (100 → 150 lux), el destinatario o el texto exige *recompilar y volver a flashear* el dispositivo. Con IFTTT, se edita el Applet en segundos sin tocar el hardware, lo que es crucial cuando el dispositivo ya está instalado en un aula.

*d) Multicanal y ecosistema.*
Cambiar de "correo" a "push", "hoja de cálculo" o "mensaje a un chat" implica en el ESP32 escribir e integrar una librería distinta para cada servicio. En IFTTT es cambiar la acción That.

*e) Escalabilidad.*
Con decenas de sensores, replicar la lógica de alertas en cada firmware multiplicaría el mantenimiento. En la nube, las reglas se administran de forma centralizada.

*Contrapartida (análisis honesto).* IFTTT introduce *dependencia de un tercero* (disponibilidad, cambios de política, límites del plan gratuito: actualmente solo *2 Applets, cada uno con una acción), **latencia adicional* y requiere Internet. Si la alerta fuese crítica para la seguridad, convendría complementar con lógica local en el ESP32 (por ejemplo, un LED o buzzer que no dependa de la nube). Para el caso de uso de este proyecto (aviso al coordinador), el equilibrio favorece claramente a IFTTT.

---

### 5.2 ¿Por qué es importante que el ESP32 publique datos con una frecuencia adecuada (10–15 s) para que IFTTT funcione correctamente?

IFTTT *no mide nada por sí mismo: solo evalúa los datos que Adafruit IO recibe. La frecuencia de publicación determina, por tanto, **qué tan bien "ve" IFTTT la realidad*. Hay dos riesgos opuestos:

*Si se publica muy lento (por ejemplo, cada varios minutos):*
- *Latencia de detección.* En el peor caso, la alerta se retrasa aproximadamente un periodo de publicación completo más la latencia de la nube. Con 10–15 s el retraso es aceptable; con 5 min no lo sería para un aviso "inmediato".
- *Eventos perdidos.* Si la luminosidad baja de 100 lux durante un lapso menor que el periodo de muestreo (por ejemplo, un apagón de 20 s), *ninguna muestra lo registra* y IFTTT nunca recibe el dato que dispara la condición. El periodo de muestreo debe ser menor que la duración mínima del evento que se quiere detectar.
- *Percepción de "feed inactivo".* Datos muy espaciados dificultan validar el funcionamiento y reducen la resolución del Line Chart.

*Si se publica muy rápido (por ejemplo, cada segundo o más):*
- *Límite de tasa de Adafruit IO.* La cuenta gratuita admite *30 datos por minuto* en total (todos los feeds). Superarlo provoca errores de límite y, si se insiste por MQTT, *bloqueos temporales* del dispositivo, con lo que también se perderían los datos críticos.
- *Ruido y notificaciones repetidas.* Con muestreo muy denso, una condición mantenida (sensor tapado) genera muchas evaluaciones del trigger y puede saturar al usuario con alertas repetidas.
- *Desperdicio de recursos.* Más consumo de energía y de ancho de banda, y consumo acelerado del almacenamiento (ver 5.4).

*¿Por qué 10–15 s?* Publicar cada 10–15 s equivale a *4–6 datos por minuto*, muy por debajo del tope de 30/min, con un retraso máximo de detección de pocos segundos y un volumen de datos manejable. Otros dos puntos de diseño son relevantes para que IFTTT reciba datos continuos:

- *No usar delay() largos.* Un delay(15000) bloquea el loop(); el cliente MQTT no puede enviar sus paquetes keep-alive y el broker cierra la conexión. Se debe usar *temporización con millis()* y llamar a client.loop() constantemente.
- *Reconexión automática.* Si el ESP32 pierde WiFi o MQTT, reconnect() debe restaurar la sesión; de lo contrario IFTTT no recibe datos y la alerta *nunca se dispara*, sin ningún aviso de error.

En resumen: la frecuencia es un *compromiso entre latencia, cobertura de eventos, límites de la plataforma y consumo*, y 10–15 s es un punto de equilibrio razonable para esta aplicación.

---

### 5.3 ¿Qué otras acciones (That) podrían configurarse en IFTTT además de la notificación push, y cómo beneficiarían al proyecto integrador?

El valor de IFTTT es que el This (la anomalía detectada) puede disparar *cualquier servicio compatible*. Algunas acciones relevantes:

| Acción (That) | Cómo se usaría | Beneficio para el proyecto integrador |
|---|---|---|
| *Enviar un correo electrónico* | Aviso con el valor y la hora a coordinación y a mantenimiento | Deja *registro formal* y permite avisar a varios responsables |
| *Agregar una fila a Google Sheets* | Registrar fecha, hora y valor cada vez que se dispara la alerta | Crea un *histórico propio y duradero*, que compensa la retención limitada de Adafruit IO y sirve como base de análisis |
| *Mensaje a Telegram / Slack / Discord* (según disponibilidad del servicio en IFTTT) | Alerta a un grupo de trabajo | *Respuesta colectiva* y trazabilidad de quién atendió la alerta |
| *Controlar un enchufe o bombilla inteligente* (Smart Life, TP-Link, Philips Hue, etc.) | Encender la iluminación cuando la luz sea baja | *Cierra el lazo de control: el sistema no solo avisa, también **actúa* (de monitoreo a automatización) |
| *Crear un evento en Google Calendar* | Agendar una tarea de revisión de la instalación | Integra la alerta con la *gestión operativa* del mantenimiento |
| *Llamada o SMS* (sujeto a disponibilidad por región y plan) | Escalar alertas críticas | *Mayor garantía de atención* cuando la notificación push pasa desapercibida |
| *Webhooks* | Llamar a una API propia, por ejemplo un backend del proyecto integrador | *Integración con sistemas propios* (base de datos, panel institucional) |

*Beneficio global.* Se pasa de "avisar" a "avisar, registrar y actuar": múltiples canales, trazabilidad y reacción automática sin intervención humana. Además, en un proyecto integrador con varios sensores (temperatura, humedad, CO₂, presencia), el mismo patrón se replica cambiando solo el trigger y la acción.

*Restricción práctica (versión gratuita).* El plan gratuito de IFTTT permite únicamente *2 Applets, cada uno con **una sola acción, y los *Webhooks y los Applets multiacción están reservados a planes de pago. Por eso, un diseño realista con la cuenta gratuita sería: *Applet 1* = notificación push; *Applet 2* = registro en Google Sheets. Las combinaciones más ricas requerirían el plan Pro o una alternativa.

---

### 5.4 ¿Qué limitaciones encontró al usar la versión gratuita de Adafruit IO?

| Límite del plan gratuito | Valor | Impacto en este proyecto |
|---|---|---|
| *Retención de datos* | *30 días* | Los datos más antiguos se eliminan: no es posible el análisis de tendencias estacionales o de largo plazo ni comparar periodos académicos (semestres). |
| *Tasa de datos* | *30 datos/min* (en total) | Condiciona la frecuencia de publicación y el número de sensores por cuenta. Superarlo produce errores y bloqueos temporales por MQTT. |
| *Feeds* | *10* | Limita la escalabilidad del proyecto integrador (un sensor suele ocupar uno o más feeds). |
| *Dashboards* | *5* | Restringe la organización de visualizaciones por aula, sensor o usuario. |
| *Dependencia de la nube* | — | Sin Internet no hay almacenamiento ni alertas. |

*Cálculo del volumen de datos de este proyecto.*
A 15 s por muestra, un feed recibe 5 760 datos/día, es decir, *≈ 172 800 datos en 30 días. A 10 s por muestra son 8 640 datos/día, es decir, *≈ 259 200 en 30 días**. La tasa de 4–6 datos/min se mantiene muy por debajo del tope de 30/min, pero cualquier ampliación (más sensores o publicación más rápida) consumiría ese margen con rapidez.

*Otras limitaciones del ecosistema gratuito*
- IFTTT gratuito: *2 Applets* con una acción cada uno (ver 5.3).
- La *latencia y la frecuencia de evaluación* de los Applets en el plan gratuito son de velocidad estándar, de modo que la notificación no es estrictamente instantánea.
- Posibles *notificaciones repetidas* mientras la condición se mantenga, ya que el trigger evalúa cada dato nuevo que cumple la regla.

*Estrategias de mitigación propuestas*
1. *Exportar periódicamente* los datos del feed (descarga en CSV desde Adafruit IO) antes de que superen los 30 días.
2. *Registrar en Google Sheets* mediante un segundo Applet de IFTTT para conservar el historial más allá de 30 días.
3. *Reducir la resolución* (publicar solo si el valor cambia de forma significativa, o promediar por bloques) para ahorrar datos sin perder información relevante.
4. *Almacenamiento local* (tarjeta SD) como respaldo del ESP32.
5. *Plan IO+* o una plataforma propia (por ejemplo, broker MQTT local + base de datos de series temporales) si el proyecto integrador exige mayor retención y escala.


---

## 6. Conclusiones y Recomendaciones

1. **Robustez en la Integración Cloud:** El uso de parámetros `GET` (`?value1=`) demostró ser un método de integración altamente confiable entre Adafruit IO e IFTTT para evitar fallos de formato JSON en notificaciones móviles directas.
2. **Eficiencia Energética y de Red:** El sistema utiliza notificaciones basadas en eventos (*event-driven architecture*), lo que evita saturar la red o mantener conexiones de polling abiertas constantemente desde el cliente final.
3. **Trabajo Futuro:** Se recomienda implementar una histéresis en la lógica de disparo (Adafruit IO o firmware) para evitar el envío de múltiples notificaciones en caso de que la lectura oscile repetidamente alrededor del valor de 3000.
