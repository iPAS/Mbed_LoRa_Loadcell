/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include "Hx711.h"


// Blinking rate in milliseconds
#define BLINKING_RATE_MS     1000
DigitalOut led(LED3);  // Initialise the digital pin LED1 as an output


// SSD1306 adapted from Adafruit's library
// https://os.mbed.com/users/nkhorman/code/Adafruit_GFX/
class I2CPreInit : public I2C  // an I2C sub-class that provides a constructed default
{
  public:
    I2CPreInit(PinName sda, PinName scl) : I2C(sda, scl) {
        frequency(400000);
        start();
    };
};

//I2CPreInit
I2C           gI2C(I2C_SDA, I2C_SCL);
Adafruit_SSD1306_I2c gOled2(gI2C, P_5, SSD_I2C_ADDRESS, 64, 128);


// https://os.mbed.com/handbook/Serial
// https://os.mbed.com/docs/mbed-os/v5.15/apis/serial.html
Serial uart(UART_TX, UART_RX, NULL, 115200);


// https://os.mbed.com/users/megrootens/code/HX711/
// https://os.mbed.com/users/jmiller322/code/SmartCrutches//file/d5e36ee82984/main.cpp/
Hx711 loadcell(P_5, P_6, 0, 0.005, 128);


int main()
{
    uint16_t x = 0;

    gOled2.clearDisplay();
    gOled2.printf("%ux%u OLED Display\r\n", gOled2.width(), gOled2.height());
    gOled2.display();
    ThisThread::sleep_for(5000);

    while (true) {
        led = !led;
        // led = 1;
        ThisThread::sleep_for(BLINKING_RATE_MS);

        float f = loadcell.read();

        gOled2.clearDisplay();
        gOled2.printf("%u: %.2f\r", x, f);
        gOled2.display();

        uart.printf("%u: %.2f\r\n", x, f);

        x++;
    }
}
