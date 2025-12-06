# CHIP-8 Emulator / Interpreter - ESP32-S3 (Arduino)

## Descripción

Este proyecto es un **emulador de CHIP-8** para el **ESP32-S3** usando Arduino.
Permite ejecutar juegos y programas diseñados para CHIP-8, mostrando la salida en una pantalla TFT y controlándolos con un teclado matricial.

---

## Requisitos

* ESP32-S3
* Pantalla TFT compatible con Arduino
* Teclado matricial 4x4
* Arduino IDE con soporte para ESP32

---

## Instalación

1. Instala [Arduino IDE](https://www.arduino.cc/en/software) y configura el soporte para ESP32.
2. Clona el repositorio:

```bash
git clone git@github.com:Hyzoor/ESP32S3-CHIP8EMU.git
cd ESP32S3-CHIP8EMU
```

3. Abre el proyecto en Arduino IDE.
4. Selecciona la placa **ESP32-S3** y el puerto correcto.
5. Compila y sube el proyecto al microcontrolador.

---

## Uso

1. Conecta la pantalla TFT y el teclado matricial. Si usa pines distintos modifique el main.ino.
2. Enciende el dispositivo.
3. El emulador cargará los ROMs y podrás jugar usando el teclado.

---

## Añadir más ROMs

1. Coloca el archivo `.ch8` de la ROM que quieras en la carpeta `/data` del proyecto.
2. Sube los archivos a la flash de ESP32s3. Se puede realizar con "Upload LittleFS to ESP32s3" 
3. ¡Ya podrás jugar tu nuevo juego en el emulador!

---

## Notas

* Es importante que los roms sean de chip8 y no de sus predecesores SUPERCHIP o XO CHIP
* La pantalla TFT y el teclado deben estar correctamente conectados para que el emulador funcione.
* En caso de querer usar otros pines modificarlos en el main.ino