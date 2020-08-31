/**
 * @brief       ADS1231.h
 * @details     24-Bit Analog-to-Digital Converter for Bridge Sensors.
 *              Function file.
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

#include "ADS1231.h"


ADS1231::ADS1231 ( PinName SCLK, PinName DOUT )
    : _SCLK                 ( SCLK )
    , _DOUT                 ( DOUT )
{

}


ADS1231::~ADS1231()
{
}



/**
 * @brief       ADS1231_Reset   ( void )
 *
 * @details     It performs an internal reset.
 *
 * @param[in]    NaN.
 *
 * @param[out]   NaN.
 *
 *
 * @return       Status of ADS1231_Reset.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         When SCLK pin changes from low to high and stays at high for
 *              longer than 26μs, ADS1231 enters power down mode.
 *
 *              When SCLK returns to low, chip will reset and enter normal
 *              operation mode.
 * @warning     NaN.
 */
ADS1231::ADS1231_status_t  ADS1231::ADS1231_Reset   ( void )
{
    _SCLK  =  ADS1231_PIN_HIGH;
    wait_us ( 52 );                                                             // Datasheet p15. At least 26us ( Security Factor: 2*26us = 52us )
    _SCLK  =  ADS1231_PIN_LOW;



    if ( _DOUT == ADS1231_PIN_HIGH )
        return   ADS1231_SUCCESS;
    else
        return   ADS1231_FAILURE;
}



/**
 * @brief       ADS1231_PowerDown   ( void )
 *
 * @details     It puts the device in power-down mode.
 *
 * @param[in]    NaN.
 *
 * @param[out]   NaN.
 *
 *
 * @return       Status of ADS1231_PowerDown.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         When SCLK pin changes from low to high and stays at high for
 *              longer than 26μs, ADS1231 enters power down mode.
 * @warning     NaN.
 */
ADS1231::ADS1231_status_t  ADS1231::ADS1231_PowerDown   ( void )
{
    _SCLK  =  ADS1231_PIN_HIGH;
    wait_us ( 52 );                                                             // Datasheet p15. At least 26us ( Security Factor: 2*26us = 52us )



    if ( _DOUT == ADS1231_PIN_HIGH )
        return   ADS1231_SUCCESS;
    else
        return   ADS1231_FAILURE;
}



/**
 * @brief       ADS1231_ReadRawData   ( Vector_count_t*, uint32_t )
 *
 * @details     It reads the raw data from the device.
 *
 * @param[in]    myAverage:         How many measurement we have to get and deliver the average.
 *
 * @param[out]   myNewRawData:      The new value from the device.
 *
 *
 * @return       Status of ADS1231_ReadRawData.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         NaN.
 * @warning     NaN.
 */
ADS1231::ADS1231_status_t  ADS1231::ADS1231_ReadRawData    ( Vector_count_t* myNewRawData, uint32_t myAverage )
{
    uint32_t i           =   0;                                                 // Counter and timeout variable
    uint32_t ii          =   0;                                                 // Counter variable
    uint32_t myAuxData   =   0;



    myNewRawData->myRawValue    =   0;                                          // Reset variable at the beginning

    // Start collecting the new measurement as many as myAverage
    for ( ii = 0; ii < myAverage; ii++ ) {
        // Reset the value
        myAuxData    =   0;

        // Wait until the device is ready or timeout
        i        =   23232323;
        _SCLK  =  ADS1231_PIN_LOW;
        while ( ( _DOUT == ADS1231_PIN_HIGH ) && ( --i ) );

        // Check if something is wrong with the device because of the timeout
        if ( i < 1 )
            return   ADS1231_FAILURE;


        // Read the data
        for ( i = 0; i < 24; i++ ) {
            // wait_us ( 1 );                                                   // Datasheet p13.  t_SCLK ( Min. 100ns )
            _SCLK  =  ADS1231_PIN_HIGH;
            // wait_us ( 1 );                                                   // Datasheet p13.  t_SCLK ( Min. 100ns )
            myAuxData    <<=     1;
            _SCLK  =  ADS1231_PIN_LOW;

            // High or Low bit
            if ( _DOUT == ADS1231_PIN_HIGH )
                myAuxData++;
        }

        // Last bit to release the bus
        // wait_us ( 1 );                                                       // Datasheet p13.  t_SCLK ( Min. 100ns )
        _SCLK  =  ADS1231_PIN_HIGH;
        // wait_us ( 1 );                                                       // Datasheet p13.  t_SCLK ( Min. 100ns )
        _SCLK  =  ADS1231_PIN_LOW;


        // Update data to get the average
        myAuxData                  ^=    0x800000;
        myNewRawData->myRawValue   +=    myAuxData;
    }

    myNewRawData->myRawValue    /=    ( float )myAverage;



    if ( _DOUT == ADS1231_PIN_HIGH )
        return   ADS1231_SUCCESS;
    else
        return   ADS1231_FAILURE;
}



/**
 * @brief       ADS1231_ReadData_WithCalibratedMass ( Vector_count_t* myNewRawData, uint32_t myAverage )
 *
 * @details     It reads data with a calibrated mass on the load cell.
 *
 * @param[in]    myAverage:             How many data to read.
 *
 * @param[out]   myNewRawData:          myRawValue_WithCalibratedMass ( ADC code taken with calibrated mass ).
 *
 *
 * @return       Status of ADS1231_ReadData_WithCalibratedMass.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         NaN.
 * @warning     NaN.
 */
ADS1231::ADS1231_status_t  ADS1231::ADS1231_ReadData_WithCalibratedMass   ( Vector_count_t* myNewRawData, uint32_t myAverage )
{
    ADS1231_status_t        aux;

    // Perform a new bunch of readings
    aux  =   ADS1231_ReadRawData ( myNewRawData, myAverage );


    // Update the value with a calibrated mass
    myNewRawData->myRawValue_WithCalibratedMass  =   myNewRawData->myRawValue;



    if ( aux == ADS1231_SUCCESS )
        return   ADS1231_SUCCESS;
    else
        return   ADS1231_FAILURE;
}



/**
 * @brief       ADS1231_ReadData_WithoutMass ( Vector_count_t* myNewRawData, uint32_t myAverage )
 *
 * @details     It reads data without any mass on the load cell.
 *
 * @param[in]    myAverage:             How many data to read.
 *
 * @param[out]   myNewRawData:          myRawValue_WithoutCalibratedMass ( ADC code taken without any mass ).
 *
 *
 * @return       Status of ADS1231_ReadData_WithoutMass.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         NaN.
 * @warning     NaN.
 */
ADS1231::ADS1231_status_t  ADS1231::ADS1231_ReadData_WithoutMass   ( Vector_count_t* myNewRawData, uint32_t myAverage )
{
    ADS1231_status_t        aux;

    // Perform a new bunch of readings
    aux  =   ADS1231_ReadRawData ( myNewRawData, myAverage );


    // Update the value without any mass
    myNewRawData->myRawValue_WithoutCalibratedMass  =   myNewRawData->myRawValue;



    if ( aux == ADS1231_SUCCESS )
        return   ADS1231_SUCCESS;
    else
        return   ADS1231_FAILURE;
}



/**
 * @brief       ADS1231_CalculateMass ( Vector_count_t* myNewRawData, uint32_t myCalibratedMass, ADS1231_scale_t myScaleCalibratedMass )
 *
 * @details     It calculates the mass.
 *
 * @param[in]    myNewRawData:              It has myRawValue_WithCalibratedMass ( ADC code taken with calibrated mass ),
 *                                          myRawValue_WithoutCalibratedMass ( ADC code taken without any mass ) and
 *                                          myRawValue ( the current data taken by the system ).
 * @param[in]    myCalibratedMass:          A known value for the calibrated mass when myRawValue_WithCalibratedMass was
 *                                          calculated.
 * @param[in]    myScaleCalibratedMass:     The range of the calibrated mass ( kg, g, mg or ug ).
 *
 * @param[out]   NaN.
 *
 *
 * @return       The calculated mass.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         NaN.
 * @warning     NaN.
 */
ADS1231::Vector_mass_t  ADS1231::ADS1231_CalculateMass ( Vector_count_t* myNewRawData, float myCalibratedMass, ADS1231_scale_t myScaleCalibratedMass )
{
    // Terminology by Texas Instruments: sbau175a.pdf, p8 2.1.1 Calculation of Mass
    float m, w_zs;
    float c_zs, w_fs, c_fs, w_t;
    float c = 0;
    float myFactor   =   1.0;

    Vector_mass_t w;


    // Adapt the scale ( kg as reference )
    switch ( myScaleCalibratedMass ) {
        default:
        case ADS1231_SCALE_kg:
            // myFactor     =   1.0;
            break;

        case ADS1231_SCALE_g:
            myFactor     /=   1000.0;
            break;

        case ADS1231_SCALE_mg:
            myFactor     /=   1000000.0;
            break;

        case ADS1231_SCALE_ug:
            myFactor     /=   1000000000.0;
            break;

    }


    // Calculate the Calibration Constant ( m )
    w_fs    =    ( myCalibratedMass / myFactor );                               // User-specified calibration mass
    c_zs    =    myNewRawData->myRawValue_WithoutCalibratedMass;                // ADC measurement taken with no load
    c_fs    =    myNewRawData->myRawValue_WithCalibratedMass;                   // ADC code taken with the calibration mass applied

    m       =    ( float )( w_fs / ( ( c_fs ) - c_zs  ) );                      // The Calibration Constant


    // Calculate the zero-scale mass ( w_zs )
    w_zs    =    - ( m * c_zs );


    // Calculate the mass ( w )
    w_t     =    myNewRawData->myRawValue_TareWeight;                           // ADC code taken without any mass after the system is calibrated;
    c       =    myNewRawData->myRawValue;                                      // The ADC code

    w.myMass   =    ( m * c ) + w_zs - w_t;                                     // The mass according to myScaleCalibratedMass


    // Update Internal Parameters
    _ADS1231_USER_CALIBATED_MASS   =   myCalibratedMass;
    _ADS1231_SCALE                 =   myScaleCalibratedMass;



    return   w;
}



/**
 * @brief       ADS1231_SetAutoTare ( float ,ADS1231_scale_t ,Vector_count_t* ,float )
 *
 * @details     It reads data without any mass on the load cell after the system is calibrated to calculate the tare weight.
 *
 * @param[in]    myCalibratedMass:          A known value for the calibrated mass when myRawValue_WithCalibratedMass was
 *                                          calculated.
 * @param[in]    myScaleCalibratedMass:     The range of the calibrated mass ( kg, g, mg or ug ).
 * @param[in]    myTime:                    How long the auto-set lasts.
 *
 * @param[out]   myNewRawData:              myRawValue_TareWeight ( ADC code taken without any mass ).
 *
 *
 * @return       Status of ADS1231_SetAutoTare.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         NaN.
 * @warning     NaN.
 */
ADS1231::ADS1231_status_t  ADS1231::ADS1231_SetAutoTare   ( float myCalibratedMass, ADS1231_scale_t myScaleCalibratedMass, Vector_count_t* myNewRawData, float myTime )
{
    ADS1231_status_t      aux;
    Vector_mass_t         myCalculatedMass;
    float                 myAuxData = 0;
    uint32_t              i = 0;


    // Perform a new bunch of readings every 1 second
    for ( i = 0; i < myTime; i++ ) {
        aux          =   ADS1231_ReadRawData ( myNewRawData, 10 );
        myAuxData   +=   myNewRawData->myRawValue;
        wait(1);
    }

    myNewRawData->myRawValue    =    ( float )( myAuxData / myTime );

    // Turn it into mass
    myCalculatedMass     =   ADS1231_CalculateMass ( myNewRawData, myCalibratedMass, myScaleCalibratedMass );

    // Update the value without any mass
    myNewRawData->myRawValue_TareWeight  =   myCalculatedMass.myMass;
    
    
    // Update Internal Parameters
    _ADS1231_USER_CALIBATED_MASS   =   myCalibratedMass;
    _ADS1231_SCALE                 =   myScaleCalibratedMass;



    if ( aux == ADS1231_SUCCESS )
        return   ADS1231_SUCCESS;
    else
        return   ADS1231_FAILURE;
}



/**
 * @brief       ADS1231_SetManualTare ( float myTareWeight )
 *
 * @details     It sets a tare weight manually.
 *
 * @param[in]    myTareWeight:          Tare weight.
 *
 * @param[out]   NaN.
 *
 *
 * @return       myRawValue_TareWeight.
 *
 *
 * @author      Manuel Caballero
 * @date        12/September/2017
 * @version     12/September/2017   The ORIGIN
 * @pre         NaN.
 * @warning     NaN.
 */
ADS1231::Vector_count_t  ADS1231::ADS1231_SetManualTare   ( float myTareWeight )
{
    Vector_count_t myNewTareWeight;

    // Update the value defined by the user
    myNewTareWeight.myRawValue_TareWeight  =   myTareWeight;



    return   myNewTareWeight;
}



/**
 * @brief       ADS1231_CalculateVoltage ( Vector_count_t* ,float )
 *
 * @details     It calculates the mass.
 *
 * @param[in]    myNewRawData:              myRawValue ( the current data taken by the system ).
 * @param[in]    myVoltageReference:        The voltage at the converter reference input.
 *
 * @param[out]   NaN.
 *
 *
 * @return       The calculated voltage.
 *
 *
 * @author      Manuel Caballero
 * @date        18/September/2017
 * @version     18/September/2017   The ORIGIN
 * @pre         NaN.
 * @warning     NaN.
 */
ADS1231::Vector_voltage_t  ADS1231::ADS1231_CalculateVoltage ( Vector_count_t* myNewRawData, float myVoltageReference )
{
    // Terminology by Texas Instruments: sbau175a.pdf, p12 3.2 Measurement Modes Raw
    float x, B, A;

    Vector_voltage_t v;

    
    x   =    myNewRawData->myRawValue;
    B   =    ( 16777216.0 - 1.0 );
    A   =    128.0;  // PGA



    // Calculate the voltage ( v )
    v.myVoltage = ( float )( ( x / B ) * ( myVoltageReference / A ) );          // The voltage




    return   v;
}
