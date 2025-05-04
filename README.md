# Proyecto de Simulación VANET - TrAD Protocol en Quito

Este repositorio contiene la implementación del protocolo **TrAD** en un entorno urbano real (Av. Simón Bolívar – Puengasí, Quito), usando **OMNeT++ 5.6.2**, **SUMO 1.18.0** y **Veins 5.1**.

---

## 🔧 Requisitos del entorno

- Windows 10
- [OMNeT++ 5.6.2](https://github.com/omnetpp/omnetpp/releases/tag/omnetpp-5.6.2)
- SUMO 1.18.0
- Veins 5.1
- Git y MSYS2 (se instala con OMNeT++)

---

## 🚨 Restaurar entorno en caso de daño

Si el entorno de OMNeT++ o los scripts de entorno se corrompen o eliminan accidentalmente:

### ✅ PASO 1. Descargar backup oficial

Descarga OMNeT++ 5.6.2 desde:

👉 https://github.com/omnetpp/omnetpp/releases/tag/omnetpp-5.6.2  
(Opcionalmente desde: https://omnetpp.org/download/old.html)

Extrae en una carpeta como:

```
D:\omnetpp-5.6.2-backup\
```

### ✅ PASO 2. Copiar archivos críticos al entorno actual

Desde el backup copia los siguientes archivos y carpetas:

- `setenv`
- `configure`
- `mingwenv.cmd`
- Carpeta completa: `tools/win64/`

A tu carpeta actual del proyecto:

```
D:\omnetpp-5.6.2\
```

⚠️ **Acepta reemplazar archivos existentes** si lo solicita.

---

## ▶️ Ejecutar entorno

Abre el entorno correctamente con:

```bash
cd D:\omnetpp-5.6.2
./mingwenv.cmd
```

Luego compila con:

```bash
. setenv
make
```

---

## 📁 Estructura del repositorio

```
omnetpp-5.6.2/
├── samples/
│   └── veins/
│       └── src/
│           └── veins/
│               └── modules/
│                   └── application/
│                       └── traci/
│                           ├── TrADApp.cc
│                           ├── TrADApp.h
│                           └── MyRSUApp.cc
├── tools/
├── setenv
├── mingwenv.cmd
├── configure
└── .gitignore
```

---

## 📦 ¿Qué se sube al repositorio?

Sube todo lo que necesites para restaurar simulaciones, **excepto archivos binarios, temporales o resultados**. Este repositorio ya incluye:

- Archivos clave del entorno (`setenv`, `mingwenv.cmd`, etc.).
- Código fuente personalizado.
- Estructura para reproducir simulaciones.

El archivo `.gitignore` está optimizado para evitar pérdidas y facilitar recuperación.

---

## 📬 Contacto

Desarrollado por: **Brandon Andrés Jiménez Nieto**  
Universidad: **Yachay Tech**  
Escuela: **Matemáticas y Ciencias Computacionales**
