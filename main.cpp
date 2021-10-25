/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include "Hx711.h"
#include "ADS1231.h"
#include "ADS1220.h"

#include "trace_helper.h"
#define TRACE_GROUP "main"


/******************************************************************************
 * Definitions
 ******************************************************************************/
#define __HX711__
// #define __ADS1232__
// #define __ADS1220__

#define TEST_AMOUNT 20
#define LSB_SIZE(PGA, VREF) ((VREF/PGA) / (((long int)1<<23)))

// Blinking rate in milliseconds
#define BLINKING_RATE_MS 1000
DigitalOut led(LED3);  // Initialise the digital pin LED1 as an output
volatile static uint16_t sample_count = 0;

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
//Serial uart(UART_TX, UART_RX, NULL, 115200);
//#ifdef UART_INTR
//static void on_uart_receive(void)
//{
//    char buf[10];
//    while (uart.readable())
//    {
//        // __disable_irq();
//        uart.read(buf, 10);
//        // __enable_irq();
//    }
//    sample_count = 0;
//}
//#endif


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

#define HX711_CAL_RAW       346209  // of the weight 100g
#define HX711_CAL_OFFSET    11286  // raw, without weight

#define HX711_PGA 64
#define HX711_VREF 5.
#define HX711_CAL_WEIGHT    100.  // 100g
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
    uint8_t num_avg;
    ADS1231::Vector_mass_t    calculated_mass;
    ADS1231::Vector_voltage_t calculated_volt;
} ads1232_sample;

void ads1232_read(void) {
    ads1232_sample.status          = loadcell_ads1232.ADS1231_ReadRawData(&ads1232_sample.count, ads1232_sample.num_avg);
    ads1232_sample.calculated_volt = loadcell_ads1232.ADS1231_CalculateVoltage(&ads1232_sample.count, ADS1232_VREF);
    ads1232_sample.calculated_mass = loadcell_ads1232.ADS1231_CalculateMass(&ads1232_sample.count, ADS1232_CAL_MASS, ADS1231::ADS1231_SCALE_g);
}

void ads1232_init(void)
{
    gOled2.clearDisplay();
    gOled2.printf("Calibrating...\r\n");
    gOled2.display();


    ads1232_sample.num_avg = 1;
    uint8_t num_avg_cal = 4;
    ADS1231::ADS1231_status_t sts;

    // Reset and wake the ADS1232 up
    sts = loadcell_ads1232.ADS1231_PowerDown();
    if (sts == ADS1231::ADS1231_status_t::ADS1231_FAILURE)
    {
        tr_debug("ADS1232 fail on power-down\r\n");
    }

    sts = loadcell_ads1232.ADS1231_Reset();
    if (sts == ADS1231::ADS1231_status_t::ADS1231_FAILURE)
    {
        tr_debug("ADS1232 fail on reset\r\n");
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
    tr_debug("ADS1232: please remove the calibrated mass before calibration ...");
    gOled2.printf("Remove mass...\r\n");
    gOled2.display();
    for (i = 5; i > 0; i--)
    {
        wait(1);
        tr_debug("%d ", i);
    }
    wait(1);
    tr_debug("WIP.\r\n");

    loadcell_ads1232.ADS1231_ReadData_WithoutMass(&ads1232_sample.count, num_avg_cal);
    tr_debug("ADS1232: .myRawValue_WithoutCalibratedMass > %f\r\n", ads1232_sample.count.myRawValue_WithoutCalibratedMass);

    tr_debug("ADS1232: please put a calibrated mass %.1fg on the scale ...", ADS1232_CAL_MASS * 1000);
    gOled2.printf("Put mass...\r\n");
    gOled2.display();
    for (i = 10; i > 0; i--)
    {
        wait(1);
        tr_debug("%d ", i);
    }
    wait(1);
    tr_debug("WIP.\r\n");

    loadcell_ads1232.ADS1231_ReadData_WithCalibratedMass(&ads1232_sample.count, num_avg_cal);
    tr_debug("ADS1232: .myRawValue_WithCalibratedMass > %f\r\n", ads1232_sample.count.myRawValue_WithCalibratedMass);

    tr_debug("ADS1232: please remove the calibrated mass before taring ...");
    gOled2.printf("Remove mass again...\r\n");
    gOled2.display();
    for (i = 5; i > 0; i--)
    {
        wait(1);
        tr_debug("%d ", i);
    }
    wait(1);
    tr_debug("WIP.\r\n");

    loadcell_ads1232.ADS1231_SetAutoTare(ADS1232_CAL_MASS, ADS1231::ADS1231_SCALE_g, &ads1232_sample.count, num_avg_cal);
    tr_debug("ADS1232: .myRawValue_TareWeight > %f\r\n", ads1232_sample.count.myRawValue_TareWeight);

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

#define ADS1220_CAL_RAW       342374  // of the weight 100g
#define ADS1220_CAL_OFFSET    14806  // raw, without weight

#define ADS1220_PGA 128
#define ADS1220_VREF 5.
#define ADS1220_CAL_WEIGHT    100.  // 100g
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
    // setup tracing
    setup_trace();


    float raw = 0;

    ThisThread::sleep_for(1000);  // Delay for showing splash

    gOled2.clearDisplay();
    gOled2.printf("%ux%u OLED Display\r\n", gOled2.width(), gOled2.height());
    gOled2.display();
    ThisThread::sleep_for(1000);

    //#ifdef UART_INTR
    //uart.attach(&on_uart_receive, Serial::RxIrq);  // Bind with on-receiving callback function.
    //#endif


    #ifdef __HX711__
    hx711_init();
    #endif
    #ifdef __ADS1232__
    ads1232_init();
    #endif
    #ifdef __ADS1220__
    ads1220_init();
    #endif


    tr_debug("----------------------------------------\r\n");

    while (true) {
        led = !led;
        ThisThread::sleep_for(BLINKING_RATE_MS);


        if (sample_count > TEST_AMOUNT+1)
        {
            /*
            #if ! defined(UART_INTR)  // Clear 'sample_count' without using rx interrupt
            if (uart.readable())
            {
                while (uart.readable())
                {
                    ThisThread::sleep_for(1);
                    uart.getc();
                }
                sample_count = 0;
                raw = 0;
                uart.printf("----------------------------------------\r\n");
            }
            #endif
            */

            // To send LoRaWAN packet
            //

                sample_count = 0;
                raw = 0;
                tr_debug("----------------------------------------\r\n");


            continue;
        }
        else
        if (++sample_count > TEST_AMOUNT)
        {
            gOled2.printf("\r\n   raw:%.1f\r\n", raw/TEST_AMOUNT);
            gOled2.printf("   -- Finish --  \r\n");
            gOled2.display();
            sample_count++;
            continue;
        }


        gOled2.clearDisplay();
        gOled2.setTextCursor(0, 0);


        #ifdef __HX711__
        tr_debug("[%d] HX711: raw=%ld volt=%.3fmV mass=%.3fg\r\n", sample_count,
            hx711_sample.raw,
            hx711_sample.volt * 1000,
            hx711_sample.mass
            );
        raw += hx711_sample.raw;
        gOled2.printf("%u:HX711 %.2fg\r\n", sample_count, hx711_sample.mass);
        #endif


        #ifdef __ADS1232__
        if (ads1232_sample.status == ADS1231::ADS1231_status_t::ADS1231_FAILURE)
        {
            tr_debug("ADS1232 fail on readRaw()\r\n");
        }
        else
        {
            tr_debug("[%d] ADS1232: raw=%ld volt=%.3fmV mass=%.3fg\r\n", sample_count,
                ads1232_sample.count.myRawValue,
                ads1232_sample.calculated_volt.myVoltage * 1000,
                ads1232_sample.calculated_mass.myMass  // ADS1231::ADS1231_SCALE_g
                );
            raw += ads1232_sample.count.myRawValue;
            gOled2.printf("%u:1232 %.2fg\r\n", sample_count, ads1232_sample.calculated_mass.myMass);
        }
        #endif


        #ifdef __ADS1220__
        if (ads1220_sample.available)
        {
            ads1220_read();
            tr_debug("[%d] ADS1220: raw=%ld volt=%.3fmV mass=%.3fg\r\n", sample_count,
                ads1220_sample.raw,
                ads1220_sample.volt * 1000,
                ads1220_sample.mass
                );
            raw += ads1220_sample.raw;
            gOled2.printf("%u:1220 %.2fg\r\n", sample_count, ads1220_sample.mass);
        }
        #endif

        gOled2.display();
    }
}
