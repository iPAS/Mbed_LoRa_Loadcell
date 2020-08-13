/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include "Hx711.h"
#include "ADS1231.h"


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
I2C                  gI2C(I2C_SDA, I2C_SCL);
Adafruit_SSD1306_I2c gOled2(gI2C, P_5, SSD_I2C_ADDRESS, 64, 128);


// https://os.mbed.com/handbook/Serial
// https://os.mbed.com/docs/mbed-os/v5.15/apis/serial.html
Serial uart(UART_TX, UART_RX, NULL, 115200);


/******************************************************************************
 * HX711
 * 
 * https://os.mbed.com/users/megrootens/code/HX711/
 * https://os.mbed.com/users/jmiller322/code/SmartCrutches//file/d5e36ee82984/main.cpp/
 * https://learn.sparkfun.com/tutorials/load-cell-amplifier-hx711-breakout-hookup-guide
 * https://www.thaieasyelec.com/article-wiki/review-product-article/how-to-use-load-cell-and-hx711-amplifier-module.html
 * https://www.thaieasyelec.com/media/wysiwyg/blog/30.png
 ******************************************************************************/
Hx711 loadcell_hx711(P_8, P_9, 0, 0.005, 128);  // Hx711(PinName pin_sck, PinName pin_dt, int offset, float scale, uint8_t gain = 128)
Ticker hx711_ticker;
struct 
{
    uint32_t data;
    float mass;
} hx711_sample;

void hx711_read(void)
{
    hx711_sample.data = loadcell_hx711.readRaw();
    hx711_sample.mass = loadcell_hx711.read();
}

void hx711_init(void)
{
    // loadcell_hx711.set_offset(124);
    hx711_ticker.attach(&hx711_read, 1);
}


/******************************************************************************
 * ADS1232
 *
 * https://os.mbed.com/users/mcm/code/ADS1231/
 ******************************************************************************/
ADS1231  loadcell_ads1232(P_25, P_29);
Ticker ads1232_ticker;
struct 
{
    ADS1231::ADS1231_status_t status;
    ADS1231::Vector_count_t   data;
    ADS1231::Vector_mass_t    calculated_mass;
    ADS1231::Vector_voltage_t calculated_volt;
} ads1232_sample;

void ads1232_read(void) {
    ads1232_sample.status          = loadcell_ads1232.ADS1231_ReadRawData(&ads1232_sample.data, 4);
    ads1232_sample.calculated_mass = loadcell_ads1232.ADS1231_CalculateMass(&ads1232_sample.data, 1.0, ADS1231::ADS1231_SCALE_kg);
    ads1232_sample.calculated_volt = loadcell_ads1232.ADS1231_CalculateVoltage(&ads1232_sample.data, 5.0);
}

void ads1232_init(void)
{
    ADS1231::ADS1231_status_t sts;

    // Reset and wake the ADS1232 up
    sts = loadcell_ads1232.ADS1231_PowerDown();
    if (sts == ADS1231::ADS1231_status_t::ADS1231_FAILURE)
    {
        uart.printf("ADS1232 fail on power-down\r\n");
    }
    
    sts = loadcell_ads1232.ADS1231_Reset();
    if (sts == ADS1231::ADS1231_status_t::ADS1231_FAILURE)
    {
        uart.printf("ADS1232 fail on reset\r\n");
    }
    wait(1);
 
    /** CALIBRATION time start!  **/
    // 1. REMOVE THE MASS ON THE LOAD CELL ( ALL LEDs OFF ). Read data without any mass on the load cell
    // aux = myWeightSensor.ADS1231_ReadData_WithoutMass ( &myData, 4 );
    // 2. PUT A KNOWN MASS ON THE LOAD CELL ( JUST LED1 ON ). Read data with an user-specified calibration mass
    // aux = myWeightSensor.ADS1231_ReadData_WithCalibratedMass ( &myData, 4 );

    // Calculating the tare weight ( JUST LED3 ON )
    // aux = myWeightSensor.ADS1231_SetAutoTare ( ADS1231::ADS1231_SCALE_kg, &myData, 5 );

    ads1232_ticker.attach(&ads1232_read, 1);  // the address of the function to be attached ( readDATA ) and the interval ( 0.5s ) ( JUST LED4 BLINKING )
}


/******************************************************************************
 * Main 
 ******************************************************************************/
int main()
{
    uint16_t x = 0;

    gOled2.clearDisplay();
    gOled2.printf("%ux%u OLED Display\r\n", gOled2.width(), gOled2.height());
    gOled2.display();

    hx711_init();
    ads1232_init();

    // wait(3);
    while (true) {
        led = !led;
        // led = 1;
        ThisThread::sleep_for(BLINKING_RATE_MS);


        gOled2.clearDisplay();
        gOled2.printf("%u\r", x);
        gOled2.display();


        uart.printf("HX711: raw=%ld mass=%.2f\r\n", hx711_sample.data, hx711_sample.mass);


        if (ads1232_sample.status == ADS1231::ADS1231_status_t::ADS1231_FAILURE)
            uart.printf("ADS1232 fail on readRaw()\r\n");
        else
            uart.printf("ADS1232: raw=%ld  mass=%0.5fkg voltage=%0.5fmV\r\n",
                (uint32_t)ads1232_sample.data.myRawValue,
                ads1232_sample.calculated_mass.myMass, 
                ads1232_sample.calculated_volt.myVoltage * 1000);


        


        x++;
    }
}
