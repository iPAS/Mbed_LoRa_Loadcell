/***********************************************************************************************************************************************************************************************
* File Name     : ADS1220.h                                                                                                                                                                                                                                                                                                                                  *
* Author        : Sandeep Reddy Malladi                                                                                                                                                                                                                                                                                                                  *
* Designation : Entwicklungs ingenieur                                                                                                                                                                                                                                                                                                                       *
* Version       : 1.0                                                                                                                                                                            *
* Description : Low level drivers to implement for ADS1220 Using SPI Communication.                                                                                                                                          *
*                                                                                                                                                                                              *
*                                                                                                                                                                                              *
************************************************************************************************************************************************************************************************/

#ifndef ADS1220_H_
#define ADS1220_H_
#include "mbed.h"

#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define gain_error_correction 0.9107468123861566//0.9980039920159681//6218905 // 0.977517107
#define weight_gram 408

/* Error Return Values */
#define ADS1220_NO_ERROR           0
#define ADS1220_ERROR               -1

/* Command Definitions */
#define ADS1220_CMD_RDATA       0x10
#define ADS1220_CMD_RREG        0x20
#define ADS1220_CMD_WREG        0x40
#define ADS1220_CMD_SYNC        0x08
#define ADS1220_CMD_SHUTDOWN  0x02
#define ADS1220_CMD_RESET       0x06

/* ADS1220 Register Definitions */
#define ADS1220_0_REGISTER        0x00
#define ADS1220_1_REGISTER      0x01
#define ADS1220_2_REGISTER      0x02
#define ADS1220_3_REGISTER      0x03


/* ADS1220 Register 0 Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0 
//--------------------------------------------------------------------------------------------
//                     MUX [3:0]                 |             GAIN[2:0]             | PGA_BYPASS
//
// Define MUX
#define ADS1220_MUX_0_1     0x00
#define ADS1220_MUX_0_2     0x10
#define ADS1220_MUX_0_3     0x20
#define ADS1220_MUX_1_2     0x30
#define ADS1220_MUX_1_3     0x40
#define ADS1220_MUX_2_3     0x50
#define ADS1220_MUX_1_0     0x60
#define ADS1220_MUX_3_2     0x70
#define ADS1220_MUX_0_G     0x80
#define ADS1220_MUX_1_G     0x90
#define ADS1220_MUX_2_G     0xa0
#define ADS1220_MUX_3_G     0xb0
#define ADS1220_MUX_EX_VREF 0xc0
#define ADS1220_MUX_AVDD    0xd0
#define ADS1220_MUX_DIV2    0xe0

// Define GAIN
#define ADS1220_GAIN_1      0x00
#define ADS1220_GAIN_2      0x02
#define ADS1220_GAIN_4      0x04
#define ADS1220_GAIN_8      0x06
#define ADS1220_GAIN_16     0x08
#define ADS1220_GAIN_32     0x0a
#define ADS1220_GAIN_64     0x0c
#define ADS1220_GAIN_128    0x0e

// Define PGA_BYPASS
#define ADS1220_PGA_BYPASS  0x01

/* ADS1220 Register 1 Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0 
//--------------------------------------------------------------------------------------------
//                DR[2:0]            |      MODE[1:0]        |     CM    |     TS    |    BCS
//
// Define DR (data rate)
#define ADS1220_DR_20           0x00
#define ADS1220_DR_45           0x20
#define ADS1220_DR_90           0x40
#define ADS1220_DR_175              0x60
#define ADS1220_DR_330              0x80
#define ADS1220_DR_600              0xa0
#define ADS1220_DR_1000             0xc0

// Define MODE of Operation
#define ADS1220_MODE_NORMAL 0x00
#define ADS1220_MODE_DUTY   0x08
#define ADS1220_MODE_TURBO  0x10
#define ADS1220_MODE_DCT    0x18

// Define CM (conversion mode)
#define ADS1220_CC          0x04

// Define TS (temperature sensor)
#define ADS1220_TEMP_SENSOR 0x02

// Define BCS (burnout current source)
#define ADS1220_BCS         0x01

/* ADS1220 Register 2 Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0 
//--------------------------------------------------------------------------------------------
//         VREF[1:0]     |        50/60[1:0]     |    PSW    |             IDAC[2:0]
//
// Define VREF
#define ADS1220_VREF_INT    0x00
#define ADS1220_VREF_EX_DED 0x40
#define ADS1220_VREF_EX_AIN 0x80
#define ADS1220_VREF_SUPPLY 0xc0

// Define 50/60 (filter response)
#define ADS1220_REJECT_OFF  0x00
#define ADS1220_REJECT_BOTH 0x10
#define ADS1220_REJECT_50   0x20
#define ADS1220_REJECT_60   0x30

// Define PSW (low side power switch)
#define ADS1220_PSW_SW      0x08

// Define IDAC (IDAC current)
#define ADS1220_IDAC_OFF    0x00
#define ADS1220_IDAC_10     0x01
#define ADS1220_IDAC_50     0x02
#define ADS1220_IDAC_100    0x03
#define ADS1220_IDAC_250    0x04
#define ADS1220_IDAC_500    0x05
#define ADS1220_IDAC_1000   0x06
#define ADS1220_IDAC_2000   0x07

/* ADS1220 Register 3 Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0 
//--------------------------------------------------------------------------------------------
//               I1MUX[2:0]          |               I2MUX[2:0]          |   DRDYM   | RESERVED
//
// Define I1MUX (current routing)
#define ADS1220_IDAC1_OFF   0x00
#define ADS1220_IDAC1_AIN0  0x20
#define ADS1220_IDAC1_AIN1  0x40
#define ADS1220_IDAC1_AIN2  0x60
#define ADS1220_IDAC1_AIN3  0x80
#define ADS1220_IDAC1_REFP0 0xa0
#define ADS1220_IDAC1_REFN0 0xc0

// Define I2MUX (current routing)
#define ADS1220_IDAC2_OFF   0x00
#define ADS1220_IDAC2_AIN0  0x04
#define ADS1220_IDAC2_AIN1  0x08
#define ADS1220_IDAC2_AIN2  0x0c
#define ADS1220_IDAC2_AIN3  0x10
#define ADS1220_IDAC2_REFP0 0x14
#define ADS1220_IDAC2_REFN0 0x18

// define DRDYM (DOUT/DRDY behaviour)

#define ADS1220_DRDY_MODE   0x02

class ADS1220 {
public:

    /** Create a new Digole Serial Display interface
     *
     * @param sda is the pin for I2C SDA
     * @param scl is the pin for I2C SCL
     * @param address is the 7-bit address (default is 0x27 for the device)
     */
    ADS1220(PinName mosi, PinName miso, PinName sclk, PinName cs);


        /* Low Level ADS1220 Device Functions */
        void Init(void);                         // Device intialization
        int WaitForDataReady(int Timeout);       // DRDY polling 
        void AssertCS(int fAssert);              // Assert/deassert CS
        void SendByte(unsigned char cData );     // Send byte to the ADS1220
        unsigned int ReceiveByte(void);          // Receive byte from the ADS1220

        /* ADS1220 Higher Level Functions */
        unsigned int ReadData(void);                     // Read the data results    
        void ReadRegister(int StartAddress, int NumRegs, unsigned * pData);  // Read the register(s)
        void WriteRegister(int StartAddress, int NumRegs, unsigned * pData); // Write the register(s)
        void SendResetCommand(void);             // Send a device Reset Command
        void SendStartCommand(void);             // Send a Start/SYNC command
        void SendShutdownCommand(void);          // Place the device in powerdown mode
                

        /* Register Set Value Commands */
        void Config(void);
        int SetChannel(int Mux);
        int SetGain(int Gain);
        int SetPGABypass(int Bypass);
        int SetDataRate(int DataRate);
        int SetClockMode(int ClockMode);
        int SetPowerDown(int PowerDown);
        int SetTemperatureMode(int TempMode);
        int SetBurnOutSource(int BurnOut);
        int SetVoltageReference(int VoltageRef);
        int Set50_60Rejection(int Rejection);
        int SetLowSidePowerSwitch(int PowerSwitch);
        int SetCurrentDACOutput(int CurrentOutput);
        int SetIDACRouting(int IDACRoute);
        int SetDRDYMode(int DRDYMode);

        /* Register Get Value Commands */
        int GetChannel(void);
        int GetGain(void);
        int GetPGABypass(void);
        int GetDataRate(void);
        int GetClockMode(void);
        int GetPowerDown(void);
        int GetTemperatureMode(void);
        int GetBurnOutSource(void);
        int GetVoltageReference(void);
        int Get50_60Rejection(void);
        int GetLowSidePowerSwitch(void);
        int GetCurrentDACOutput(void);
        int GetIDACRouting(int WhichOne);
        int GetDRDYMode(void);

        /* Useful Functions within Main Program for Setting Register Contents
        *
        *   These functions show the programming flow based on the header definitions.
        *   The calls are not made within the demo example, but could easily be used by calling the function
        *       defined within the program to complete a fully useful program.
        *   Similar function calls were made in the firwmare design for the ADS1220EVM.
        *  
        *  The following function calls use ASCII data sent from a COM port to control settings 
        *   on the ADS1220.  The data is recontructed from ASCII and then combined with the
        *   register contents to save as new configuration settings.
        *
        *   Function names correspond to datasheet register definitions
        */
        void set_MUX(int c);
        void set_GAIN(char c);
        void set_PGA_BYPASS(char c);
        void set_DR(char c);
        void set_MODE(char c);
        void set_CM(char c);
        void set_TS(char c);
        void set_BCS(char c);
        void set_VREF(char c);
        void set_50_60(char c);
        void set_PSW(char c);
        void set_IDAC(char c);
        void set_IMUX(char c, int i);
        void set_DRDYM(char c);
        void set_ERROR(void);
        void set_ERROR_Transmit(void);
        void set_ERROR_Receive(void);
        
private:
    SPI                 _device;
    DigitalOut          nCS_;
    uint8_t _address;
    uint8_t _Comdelay;

    size_t printNumber(unsigned long n, uint8_t base);
    size_t printFloat(double number, uint8_t digits);
};

#endif /*ADS1220_H_*/
