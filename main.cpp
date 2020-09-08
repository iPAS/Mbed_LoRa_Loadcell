/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include "Hx711.h"
#include "ADS1231.h"
#include "ADS1220.h"


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
 * Definitions
 ******************************************************************************/
// #define __HX711__
// #define __ADS1232__
#define __ADS1220__
#define LSB_SIZE(PGA, VREF) ((VREF/PGA) / (((long int)1<<23)))


/******************************************************************************
 * HX711
 * 
 * https://os.mbed.com/users/megrootens/code/HX711/
 * https://os.mbed.com/users/jmiller322/code/SmartCrutches//file/d5e36ee82984/main.cpp/
 * https://learn.sparkfun.com/tutorials/load-cell-amplifier-hx711-breakout-hookup-guide
 * https://www.thaieasyelec.com/article-wiki/review-product-article/how-to-use-load-cell-and-hx711-amplifier-module.html
 * https://www.thaieasyelec.com/media/wysiwyg/blog/30.png
 ******************************************************************************/
#ifdef __HX711__

#define HX711_PGA 64
#define HX711_VREF 5.
#define HX711_CAL_OFFSET    -7800  // raw, without weight
#define HX711_CAL_WEIGHT    100.  // 100g
#define HX711_CAL_RAW       326800  // of the weight 100g
#define HX711_CAL_SCALE     (HX711_CAL_WEIGHT / (float)(HX711_CAL_RAW - HX711_CAL_OFFSET))
Hx711 loadcell_hx711(P_8, P_9, HX711_CAL_OFFSET, HX711_CAL_SCALE, HX711_PGA);  // Hx711(PinName pin_sck, PinName pin_dt, int offset, float scale, uint8_t gain = 128)
// Hx711 loadcell_hx711(P_8, P_9, 25950, -0.0046522447, HX711_PGA);  // Hx711(PinName pin_sck, PinName pin_dt, int offset, float scale, uint8_t gain = 128)
Ticker hx711_ticker;
struct
{
    int32_t raw;
    float volt;
    float mass;
} hx711_sample;

void hx711_read(void)
{
    hx711_sample.raw  = loadcell_hx711.readRaw();
    hx711_sample.volt = hx711_sample.raw * LSB_SIZE(HX711_PGA, HX711_VREF);
    hx711_sample.mass = loadcell_hx711.read();
}

void hx711_init(void)
{
    // loadcell_hx711.set_scale();
    // loadcell_hx711.set_offset(124);
    hx711_ticker.attach(&hx711_read, 1);
}

#endif


/******************************************************************************
 * ADS1232
 *
 * https://os.mbed.com/users/mcm/code/ADS1231/
 ******************************************************************************/
#ifdef __ADS1232__

#define ADS1232_PGA 128
#define ADS1232_VREF 5.
#define ADS1232_CAL_MASS 0.100  // 100g
ADS1231  loadcell_ads1232(P_25, P_29);  // ADS1231::ADS1231 ( PinName SCLK, PinName DOUT )
Ticker ads1232_ticker;
struct 
{
    ADS1231::ADS1231_status_t status;
    ADS1231::Vector_count_t   count;
    ADS1231::Vector_mass_t    calculated_mass;
    ADS1231::Vector_voltage_t calculated_volt;
} ads1232_sample;

void ads1232_read(void) {
    ads1232_sample.status          = loadcell_ads1232.ADS1231_ReadRawData(&ads1232_sample.count, 4);
    ads1232_sample.calculated_volt = loadcell_ads1232.ADS1231_CalculateVoltage(&ads1232_sample.count, ADS1232_VREF);
    ads1232_sample.calculated_mass = loadcell_ads1232.ADS1231_CalculateMass(&ads1232_sample.count, ADS1232_CAL_MASS, ADS1231::ADS1231_SCALE_g);
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
    ThisThread::sleep_for(1000);

    /** CALIBRATION time start!  **/
    // 1. REMOVE THE MASS ON THE LOAD CELL ( ALL LEDs OFF ). Read data without any mass on the load cell
    // aux = myWeightSensor.ADS1231_ReadData_WithoutMass ( &myData, 4 );
    // 2. PUT A KNOWN MASS ON THE LOAD CELL ( JUST LED1 ON ). Read data with an user-specified calibration mass
    // aux = myWeightSensor.ADS1231_ReadData_WithCalibratedMass ( &myData, 4 );

    // Calculating the tare weight ( JUST LED3 ON )
    // aux = myWeightSensor.ADS1231_SetAutoTare ( ADS1231::ADS1231_SCALE_kg, &myData, 5 );

    int i;
    uart.printf("ADS1232: please remove the calibrated mass before calibration ...");
    for (i = 5; i > 0; i--)
    {
        wait(1);
        uart.printf(" %d ", i);
    }
    wait(1);
    uart.printf("[in progress].\r\n");

    loadcell_ads1232.ADS1231_ReadData_WithoutMass(&ads1232_sample.count, 4);
    uart.printf("ADS1232: .myRawValue_WithoutCalibratedMass > %f\r\n", ads1232_sample.count.myRawValue_WithoutCalibratedMass);

    uart.printf("ADS1232: please put a calibrated mass %.1fg on the scale ...", ADS1232_CAL_MASS * 1000);
    for (i = 10; i > 0; i--)
    {
        wait(1);
        uart.printf(" %d ", i);
    }
    wait(1);
    uart.printf("[in progress].\r\n");

    loadcell_ads1232.ADS1231_ReadData_WithCalibratedMass(&ads1232_sample.count, 4);
    uart.printf("ADS1232: .myRawValue_WithCalibratedMass > %f\r\n", ads1232_sample.count.myRawValue_WithCalibratedMass);

    uart.printf("ADS1232: please remove the calibrated mass before taring ...");
    for (i = 5; i > 0; i--)
    {
        wait(1);
        uart.printf(" %d ", i);
    }
    wait(1);
    uart.printf("[in progress].\r\n");

    loadcell_ads1232.ADS1231_SetAutoTare(ADS1232_CAL_MASS, ADS1231::ADS1231_SCALE_g, &ads1232_sample.count, 4);
    uart.printf("ADS1232: .myRawValue_TareWeight > %f\r\n", ads1232_sample.count.myRawValue_TareWeight);

    // ads1232_sample.count.myRawValue_WithoutCalibratedMass = 8385827;
    // ads1232_sample.count.myRawValue_WithCalibratedMass = 8590153;  // @31g calibrated mass
    // ads1232_sample.count.myRawValue_TareWeight = -0.025879;

    ads1232_ticker.attach(&ads1232_read, 1);  // the address of the function to be attached ( readDATA ) and the interval ( 0.5s ) ( JUST LED4 BLINKING )
}

#endif


/******************************************************************************
 * ADS1220
 *
 * https://os.mbed.com/users/sandeepmalladi/code/ADS1220/
 * https://os.mbed.com/users/sandeepmalladi/code/WeightScale//file/58df937cd05d/main.cpp/
 ******************************************************************************/
#ifdef __ADS1220__

#define ADS1220_PGA 128
#define ADS1220_VREF 5. 
#define ADS1220_CAL_OFFSET    -5500  // raw, without weight
#define ADS1220_CAL_WEIGHT    100.  // 100g
#define ADS1220_CAL_RAW       320500  // of the weight 100g
#define ADS1220_CAL_SCALE     (ADS1220_CAL_WEIGHT / (float)(ADS1220_CAL_RAW - ADS1220_CAL_OFFSET))
ADS1220 loadcell_ads1220(P_13, P_12, P_14, P_15);  //(PinName mosi, PinName miso, PinName sclk, PinName cs)
InterruptIn pin_drdy(P_20);
Ticker ads1220_ticker;
struct
{
    volatile bool available;
    int32_t offset;
    float scale;
    int32_t raw;
    float volt;
    float mass;
} ads1220_sample = { false, ADS1220_CAL_OFFSET, ADS1220_CAL_SCALE, };

void ads1220_data_ready(void)
{
    // ads1220_sample.raw       = loadcell_ads1220.ReadData();
    // ads1220_sample.volt      = ads1220_sample.raw * LSB_SIZE(ADS1220_PGA, ADS1220_VREF);
    ads1220_sample.available = true;
}

void ads1220_init(void)
{
    // pin_drdy.rise(&ads1220_read);
    pin_drdy.fall(&ads1220_data_ready);  // Interrupt routine of End-of-conversion acknowledgement

    loadcell_ads1220.SendResetCommand();
    ThisThread::sleep_for(1);
    loadcell_ads1220.Config();
    ThisThread::sleep_for(1);
    loadcell_ads1220.SendStartCommand();
}

void ads1220_read(void)
{
    ads1220_sample.raw       = loadcell_ads1220.ReadData();
    ads1220_sample.volt      = ads1220_sample.raw * LSB_SIZE(ADS1220_PGA, ADS1220_VREF);
    ads1220_sample.mass      = (ads1220_sample.raw - ads1220_sample.offset) * ads1220_sample.scale;
    ads1220_sample.available = false;
}

#endif


/******************************************************************************
 * Main
 ******************************************************************************/
int main()
{
    uint16_t x = 0;

    gOled2.clearDisplay();
    gOled2.printf("%ux%u OLED Display\r\n", gOled2.width(), gOled2.height());
    gOled2.display();

    #ifdef __HX711__
    hx711_init();
    #endif
    #ifdef __ADS1232__
    ads1232_init();
    #endif
    #ifdef __ADS1220__
    ads1220_init();
    #endif


    while (true) {
        led = !led;
        // led = 1;
        ThisThread::sleep_for(BLINKING_RATE_MS);


        gOled2.clearDisplay();
        gOled2.printf("%u\r", x);
        gOled2.display();


        #ifdef __HX711__
        uart.printf("HX711: raw=%ld volt=%.3fmV mass=%.3fg\r\n",
            hx711_sample.raw,
            hx711_sample.volt * 1000,
            hx711_sample.mass
            );
        #endif


        #ifdef __ADS1232__
        if (ads1232_sample.status == ADS1231::ADS1231_status_t::ADS1231_FAILURE)
            uart.printf("ADS1232 fail on readRaw()\r\n");
        else
            uart.printf("ADS1232: raw=%ld volt=%0.3fmV mass=%.3fg\r\n",
                ads1232_sample.count.myRawValue,
                ads1232_sample.calculated_volt.myVoltage * 1000,
                ads1232_sample.calculated_mass.myMass  // ADS1231::ADS1231_SCALE_g
                );
        #endif


        #ifdef __ADS1220__
        if (ads1220_sample.available)
        {
            ads1220_read();
            uart.printf("ADS1220: raw=%ld volt=%fmV mass=%.3fg\r\n",
                ads1220_sample.raw,
                ads1220_sample.volt * 1000,
                ads1220_sample.mass
                );
        }
        #endif


        x++;
    }
}
