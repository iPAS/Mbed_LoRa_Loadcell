/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "Adafruit_SSD1306.h"


// Blinking rate in milliseconds
#define BLINKING_RATE_MS     1000


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

I2CPreInit           gI2C(I2C_SDA, I2C_SCL);
Adafruit_SSD1306_I2c gOled2(gI2C, P_9, SSD_I2C_ADDRESS, 64, 128);


int main()
{
    DigitalOut led(LED3);  // Initialise the digital pin LED1 as an output

    gOled2.printf("%ux%u OLED Display\r\n", gOled2.width(), gOled2.height());
    // gOled2.display();
    // gOled2.clearDisplay();

    while (true) {
        led = !led;
        // led = 1;
        ThisThread::sleep_for(BLINKING_RATE_MS);
    }
}
