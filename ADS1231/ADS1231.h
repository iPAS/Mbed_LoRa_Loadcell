/**
 * @brief       ADS1231.h
 * @details     24-Bit Analog-to-Digital Converter for Bridge Sensors.
 *              Header file.
 *
 *
 * @return      NA
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017    The ORIGIN
 * @pre         NaN.
 * @warning     NaN
 * @pre         This code belongs to Nimbus Centre ( http://www.nimbus.cit.ie ).
 */
#ifndef ADS1231_H
#define ADS1231_H

#include "mbed.h"


/**
    Example:

#include "mbed.h"
#include "ADS1231.h"

ADS1231  myWeightSensor      ( p5, p6 );
Serial pc                    ( USBTX, USBRX );

Ticker newReading;
DigitalOut myled1            ( LED1 );
DigitalOut myled2            ( LED2 );
DigitalOut myled3            ( LED3 );
DigitalOut myled4            ( LED4 );

ADS1231::ADS1231_status_t      aux;
ADS1231::Vector_count_t        myData;
ADS1231::Vector_mass_t         myCalculatedMass;
ADS1231::Vector_voltage_t      myCalculatedVoltage;


void readDATA ( void )
{
    myled4    =  1;

    aux                 =    myWeightSensor.ADS1231_ReadRawData       ( &myData, 4 );
    myCalculatedMass    =    myWeightSensor.ADS1231_CalculateMass     ( &myData, 1.0, ADS1231::ADS1231_SCALE_kg );
    myCalculatedVoltage =    myWeightSensor.ADS1231_CalculateVoltage  ( &myData, 5.0 );

    pc.printf( "Raw Data: %ld  Mass: %0.5f kg Voltage: %0.5f mV\r\n", (uint32_t)myData.myRawValue, myCalculatedMass.myMass, 1000*myCalculatedVoltage.myVoltage );

    myled4    =  0;
}


int main()
{
    pc.baud ( 115200 );


    // Reset and wake the device up
    aux = myWeightSensor.ADS1231_PowerDown    ();
    aux = myWeightSensor.ADS1231_Reset        ();
    wait(1);


    // CALIBRATION time start!
    // 1. REMOVE THE MASS ON THE LOAD CELL ( ALL LEDs OFF ). Read data without any mass on the load cell
    aux = myWeightSensor.ADS1231_ReadData_WithoutMass ( &myData, 4 );

    myled1   =   1;
    wait(3);


    // 2. PUT A KNOWN MASS ON THE LOAD CELL ( JUST LED1 ON ). Read data with an user-specified calibration mass
    aux = myWeightSensor.ADS1231_ReadData_WithCalibratedMass ( &myData, 4 );
    // CALIBRATION time end!


    // [ OPTIONAL ] REMOVE THE MASS ON THE LOAD CELL ( JUST LED2 ON ). Read the device without any mass to calculate the tare weight for 5 seconds
    myled1   =   0;
    myled2   =   1;
    wait(3);
    myled2   =   0;

    // Calculating the tare weight ( JUST LED3 ON )
    myled3   =   1;
    aux = myWeightSensor.ADS1231_SetAutoTare ( ADS1231::ADS1231_SCALE_kg, &myData, 5 );
    myled3   =   0;


    newReading.attach( &readDATA, 0.5 );                                        // the address of the function to be attached ( readDATA ) and the interval ( 0.5s ) ( JUST LED4 BLINKING )

    // Let the callbacks take care of everything
    while(1)
    {
        sleep();
    }
}

*/


/*!
 Library for the ADS1231 24-Bit Analog-to-Digital Converter for Bridge Sensors.
*/
class ADS1231
{
public:
     /**
      * @brief   READY/BUSY DATA
      */
    typedef enum {
        ADS1231_DATA_BUSY         =   0,              /*!<  ADS1231 data is NOT ready to be read.                                */
        ADS1231_DATA_READY        =   1               /*!<  ADS1231 data is ready to be read.                                    */
    } ADS1231_data_output_status_t;



    /**
      * @brief   SCALE
      */
    typedef enum {
        ADS1231_SCALE_kg          =   0,              /*!<  ADS1231 Scale in kg.                                                 */
        ADS1231_SCALE_g           =   1,              /*!<  ADS1231 Scale in  g.                                                 */
        ADS1231_SCALE_mg          =   2,              /*!<  ADS1231 Scale in mg.                                                 */
        ADS1231_SCALE_ug          =   3               /*!<  ADS1231 Scale in ug.                                                 */
    } ADS1231_scale_t;




#ifndef VECTOR_STRUCT_H
#define VECTOR_STRUCT_H
    typedef struct {
        float myRawValue_WithCalibratedMass;
        float myRawValue_WithoutCalibratedMass;
        float myRawValue_TareWeight;
        uint32_t myRawValue;
    } Vector_count_t;

    typedef struct {
        float myMass;
    } Vector_mass_t;

    typedef struct {
        float myVoltage;
    } Vector_voltage_t;
#endif



    /**
      * @brief   INTERNAL CONSTANTS
      */
#define ADS1231_PIN_HIGH           0x01               /*!<   Pin 'HIGH'                                                       */
#define ADS1231_PIN_LOW            0x00               /*!<   Pin 'LOW'                                                        */

    typedef enum {
        ADS1231_SUCCESS     =       0,
        ADS1231_FAILURE     =       1,
    } ADS1231_status_t;




    /** Create an ADS1231 object connected to the specified pins.
      *
      * @param sclk             ADS1231 Power down control (high active) and serial clock input
      * @param dout             ADS1231 Serial data output
      */
    ADS1231 ( PinName SCLK, PinName DOUT );

    /** Delete ADS1231 object.
     */
    ~ADS1231();

    /** It performs an internal reset.
     */
    ADS1231_status_t  ADS1231_Reset                         ( void );

    /** It puts the device into power-down mode.
     */
    ADS1231_status_t  ADS1231_PowerDown                     ( void );

    /** It reads raw data from the device.
     */
    ADS1231_status_t  ADS1231_ReadRawData                   ( Vector_count_t* myNewRawData, uint32_t myAverage );

    /** It reads raw data with an user-specified calibrated mass.
     */
    ADS1231_status_t  ADS1231_ReadData_WithCalibratedMass   ( Vector_count_t* myNewRawData, uint32_t myAverage );

    /** It reads raw data without any mass.
     */
    ADS1231_status_t  ADS1231_ReadData_WithoutMass          ( Vector_count_t* myNewRawData, uint32_t myAverage );

    /** It reads raw data without any mass after the system is calibrated.
     */
    ADS1231_status_t  ADS1231_SetAutoTare                   ( float myCalibratedMass, ADS1231_scale_t myScaleCalibratedMass, Vector_count_t* myNewRawData, float myTime );

    /** It sets a tare weight manually.
     */
    Vector_count_t  ADS1231_SetManualTare                 ( float myTareWeight );

    /** It calculates scaled data.
     */
    Vector_mass_t  ADS1231_CalculateMass                  ( Vector_count_t* myNewRawData, float myCalibratedMass, ADS1231_scale_t myScaleCalibratedMass );

    /** It calculates voltage data.
     */
    Vector_voltage_t  ADS1231_CalculateVoltage            ( Vector_count_t* myNewRawData, float myVoltageReference );




private:
    DigitalOut              _SCLK;
    DigitalIn               _DOUT;
    ADS1231_scale_t         _ADS1231_SCALE;
    float                   _ADS1231_USER_CALIBATED_MASS;
};

#endif
