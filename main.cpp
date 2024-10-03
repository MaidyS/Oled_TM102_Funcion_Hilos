/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h" // Librería base de Mbed OS
#include "I2C.h" // Controlador para comunicación I2C
#include "ThisThread.h" // Para hilos y temporización
#include "Adafruit_GFX.h" // Librería para gráficos
#include "Adafruit_SSD1306.h" // Controlador para pantallas OLED SSD1306
#include <cstdio> // Para manejo de cadenas (sprintf)
#include <cstring> // Para funciones de manejo de strings (strlen)

#define TMP102_ADDRESS 0x90 // Dirección del TMP102
#define tiempo_muestreo 1s // Asegúrate de declarar esta constante

// Pines y puertos
BufferedSerial serial(USBTX, USBRX);
I2C i2c(D14, D15);
Adafruit_SSD1306_I2c oled(i2c, D0, 0x78, 32, 128);  // Dirección I2C para OLED

AnalogIn ain(A0);

// Variables globales
float Vin = 0.0;
int ent = 0;
int dec = 0;
char men[40];
char comando[3] = {0x01, 0x60, 0xA0};
char data[2];
const char *mensaje_inicio = "Arranque del programa\n\r";

// Función para inicializar la pantalla OLED
void inicializarOLED() {
    oled.begin();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.display();
    ThisThread::sleep_for(3000ms);
    oled.clearDisplay();
    oled.display();
    oled.printf("Test\r\n");
    oled.display();
}

// Función para leer voltaje analógico y mostrarlo en OLED
void leerVoltaje() {
    while (true) {
        Vin = ain * 3.3;
        ent = int(Vin);
        dec = int((Vin - ent) * 10000);

        oled.clearDisplay();
        oled.setTextCursor(0, 2);
        sprintf(men, "El voltaje es:\n\r %01u.%04u volts \n\r", ent, dec);
        oled.printf("%s", men);  // Se asegura un formato seguro
        oled.display();

        // Impresión por el puerto serie
        serial.write(men, strlen(men));

        ThisThread::sleep_for(tiempo_muestreo);  // Usa la constante correctamente declarada
    }
}

// Función para leer la temperatura por I2C
void leerTemperatura() {
    while (true) {
        comando[0] = 0;  // Registro de temperatura
        i2c.write(TMP102_ADDRESS, comando, 1);  // Enviar el comando para leer
        i2c.read(TMP102_ADDRESS, data, 2);      // Leer 2 bytes

        int16_t temp = (data[0] << 4) | (data[1] >> 4);
        float Temperatura = temp * 0.0625;
        ent = int(Temperatura);
        dec = int((Temperatura - ent) * 10000);

        oled.clearDisplay();
        oled.setTextCursor(0, 2);
        sprintf(men, "La Temperatura es:\n\r %01u.%04u Celsius\n\r", ent, dec);
        oled.printf("%s", men);  // Se asegura un formato seguro
        oled.display();

        // Impresión por el puerto serie
        serial.write(men, strlen(men));

        ThisThread::sleep_for(2000ms);
    }
}

int main() {
    // Inicialización
    inicializarOLED();
    serial.write(mensaje_inicio, strlen(mensaje_inicio));

    // Arranque del sensor de temperatura
    i2c.write(TMP102_ADDRESS, comando, 3);

    // Creación de hilos
    Thread hiloVoltaje;
    Thread hiloTemperatura;

    // Asignar funciones a los hilos
    hiloVoltaje.start(leerVoltaje);
    hiloTemperatura.start(leerTemperatura);

    // Mantener el programa corriendo
    while (true) {
        ThisThread::sleep_for(1s);
    }
}


