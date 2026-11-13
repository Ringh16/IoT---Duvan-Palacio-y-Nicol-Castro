# Práctica de IoT: Almacenamiento Avanzado y Visualización con ESP32, InfluxDB y Grafana

Este repositorio contiene la documentación, scripts de conexión y configuraciones de dashboards para la persistencia de datos de sensores ambientales utilizando un ESP32, un puente en Python, InfluxDB OSS local y Grafana en un entorno de **Windows 11**.

---

## Requisitos Previos y Entorno

* Sistema operativo: Windows 11.
* Python instalado (versión 3.8 o superior).
* NodeMCU / ESP32 configurado enviando datos (temperatura y luminosidad) a **Adafruit IO**.

---

## 1. Instalación y Configuración de InfluxDB (OSS Local)

Para mantener un histórico persistente de los datos de las aulas (`AULA-101` y `AULA-102`):

1. **Descargar InfluxDB**
   * Dirígete al sitio oficial de InfluxData y descarga la versión Open Source (OSS) para **Windows**.
   * Descomprime el archivo ZIP en una ruta accesible de tu equipo (por ejemplo, `C:\influxdb`).

2. **Ejecutar el servidor**
   * Abre una terminal (PowerShell o CMD), navega hasta la carpeta donde descomprimiste InfluxDB y ejecuta el servicio:

     ```bash
     influxd
     ```

   * Deja esta ventana abierta para mantener el servidor corriendo.

3. **Configuración inicial en el navegador**
   * Abre tu navegador y entra a: `http://localhost:8086`
   * Configura tu usuario, contraseña, la organización (`uts`) y el bucket inicial (`aulas`).
   * Genera y copia un **API Token** con permisos de acceso total, el cual utilizarás en los scripts de Python y en Grafana.

---

## 2. Instalación de Dependencias de Python y Scripts

El puente de datos se encarga de leer los valores desde los feeds de Adafruit IO y almacenarlos en InfluxDB local de manera automatizada.

1. **Instalación de librerías**

   Abre una terminal en VS Code y ejecuta el siguiente comando para instalar los paquetes necesarios:

   ```bash
   pip install requests influxdb-client numpy
   ```
