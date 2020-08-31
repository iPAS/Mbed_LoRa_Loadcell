/***********************************************************************************************************************************************************************************************
* File Name     : ADS1220.cpp                                                                                                                                                                                                                                                                                                                                    *
* Author        : Sandeep Reddy Malladi                                                                                                                                                                                                                                                                                                                  *
* Designation : Entwicklungs ingenieur                                                                                                                                                         *                                                                                                                                                    *
* Version       : 1.0                                                                                                                                                                            *
* Description : Low level drivers to implement for ADS1220 Using SPI Communication.                                                                                                                                          *
*                                                                                                                                                                                              *
*                                                                                                                                                                                              *
************************************************************************************************************************************************************************************************/

#include "mbed.h"
#include "ADS1220.h"
#include <inttypes.h>

ADS1220::ADS1220(PinName mosi, PinName miso, PinName sclk,PinName cs):
    _device(mosi, miso, sclk),nCS_(cs)
{
    _device.frequency(4000000);
    _device.format(8,1);
}

void ADS1220::AssertCS(bool fAssert)
{
    if (fAssert)
    {
        nCS_ = 0;
        nCS_ = 1;
        nCS_ = 0;
    }
    else
    {
        nCS_ = 1;
    }
}

// ADS1220 Initial Configuration
void ADS1220::Config(void)
{
    unsigned reg;

    // Setting from page-59 of paper SBAS501C.pdf
    // REGISTER SETTING DESCRIPTION
    // 00h      3Eh     AINP = AIN1, AINN = AIN2, gain = 128, PGA enabled
    // 01h      04h     DR = 20 SPS, normal mode, continuous conversion mode
    // 02h      98h     External reference (REFP1, REFN1), simultaneous 50-Hz and 60-Hz rejection, PSW = 1
    // 03h      00h     No IDACs used

    ReadRegister(ADS1220_0_REGISTER, 0x01, &reg);
    reg &= 0x00;  // clear prev value;
    reg |= (ADS1220_MUX_1_2 | ADS1220_GAIN_128);  //ADS1220_GAIN_128);
    WriteRegister(ADS1220_0_REGISTER, 0x01, &reg);  // write the register value containing the new value back to the ADS

    ReadRegister(ADS1220_1_REGISTER, 0x01, &reg);
    reg &= 0x00;
    reg |= (ADS1220_DR_20 | ADS1220_CC);  // Set default start mode to 20sps and continuous conversions
    WriteRegister(ADS1220_1_REGISTER, 0x01, &reg);

    ReadRegister(ADS1220_2_REGISTER, 0x01, &reg);
    reg &= 0x00;
    reg |= (ADS1220_VREF_EX_AIN | ADS1220_REJECT_BOTH | ADS1220_PSW_SW);
    WriteRegister(ADS1220_2_REGISTER, 0x01, &reg);
}


void ADS1220::SendByte(unsigned char Value)
{
    _device.write(Value);
}


unsigned int ADS1220::ReceiveByte(void)
{
    unsigned int readvalue;
    readvalue = _device.write(0x00);

    return readvalue;
}

/*
******************************************************************************
 higher level functions
*/
unsigned int ADS1220::ReadData(void)
{
   uint32_t Data;
   
      
   // assert CS to start transfer
    AssertCS(true);

   // send the command byte
   SendByte(ADS1220_CMD_RDATA);
      
   // get the conversion result
#ifdef ADS1120
   Data = ReceiveByte();
   Data = (Data << 8) | ReceiveByte();
   //Data = (Data << 8) | ADS1220ReceiveByte();

   // sign extend data
   if (Data & 0x8000)
      Data |= 0xffff0000;
#else
   Data = ReceiveByte();
   Data = (Data << 8) |ReceiveByte();
   Data = (Data << 8) |ReceiveByte();

   // sign extend data
   if (Data & 0x800000)
      Data |= 0xff000000;
   
#endif
   // de-assert CS
   AssertCS(false);
   return Data;
}

void ADS1220::ReadRegister(int StartAddress, int NumRegs, unsigned * pData)
{
   int i;

    // assert CS to start transfer
        AssertCS(true);
   
    // send the command byte
    SendByte(ADS1220_CMD_RREG | (((StartAddress<<2) & 0x0c) |((NumRegs-1)&0x03)));
   
    // get the register content
    for (i=0; i< NumRegs; i++)
    {
        *pData++ = ReceiveByte();
    }
   
    // de-assert CS
        AssertCS(false);
    
    return;
}

void ADS1220::WriteRegister(int StartAddress, int NumRegs, unsigned * pData)
{
    int i;
   
    // assert CS to start transfer
    AssertCS(true);
   
    // send the command byte
    SendByte(ADS1220_CMD_WREG | (((StartAddress<<2) & 0x0c) |((NumRegs-1)&0x03)));
   
    // send the data bytes
    for (i=0; i< NumRegs; i++)
    {
        SendByte(*pData++);
    }
   
    // de-assert CS
    AssertCS(false);
   
    return;
}

void ADS1220::SendResetCommand(void)
{
    // assert CS to start transfer
    AssertCS(true);
   
    // send the command byte
    SendByte(ADS1220_CMD_RESET);
   
    // de-assert CS
    AssertCS(false);
   
    return;
}

void ADS1220::SendStartCommand(void)
{
    // assert CS to start transfer
    AssertCS(true);
   
    // send the command byte
    SendByte(ADS1220_CMD_SYNC);
   
    // de-assert CS
    AssertCS(false);
     
    return;
}

void ADS1220::SendShutdownCommand(void)
{
    // assert CS to start transfer
    AssertCS(true);
   
    // send the command byte
    SendByte(ADS1220_CMD_SHUTDOWN);
   
    // de-assert CS
    AssertCS(false);
     
    return;
}

/*
******************************************************************************
register set value commands
*/

int ADS1220::SetChannel(int Mux)
{
    unsigned int cMux = Mux;       
   // write the register value containing the new value back to the ADS
   WriteRegister(ADS1220_0_REGISTER, 0x01, &cMux);
   
   return ADS1220_NO_ERROR;
}

int ADS1220::SetGain(int Gain)
{
    unsigned int cGain = Gain;   
    // write the register value containing the new code back to the ADS
    WriteRegister(ADS1220_0_REGISTER, 0x01, &cGain);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetPGABypass(int Bypass)
{
    unsigned int cBypass = Bypass;
    // write the register value containing the new code back to the ADS
    WriteRegister(ADS1220_0_REGISTER, 0x01, &cBypass);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetDataRate(int DataRate)
{
    unsigned int cDataRate = DataRate;  
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_1_REGISTER, 0x01, &cDataRate);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetClockMode(int ClockMode)
{
    unsigned int cClockMode = ClockMode;
   
    // write the register value containing the value back to the ADS
    WriteRegister(ADS1220_1_REGISTER, 0x01, &cClockMode);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetPowerDown(int PowerDown)
{
    unsigned int cPowerDown = PowerDown;
   
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_1_REGISTER, 0x01, &cPowerDown);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetTemperatureMode(int TempMode)
{
    unsigned int cTempMode = TempMode;
   
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_1_REGISTER, 0x01, &cTempMode);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetBurnOutSource(int BurnOut)
{
    unsigned int cBurnOut = BurnOut;
   
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_1_REGISTER, 0x01, &cBurnOut);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetVoltageReference(int VoltageRef)
{
    unsigned int cVoltageRef = VoltageRef;
   
    // write the register value containing the new value back to the ADS
     WriteRegister(ADS1220_2_REGISTER, 0x01, &cVoltageRef);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::Set50_60Rejection(int Rejection)
{
    unsigned int cRejection = Rejection;
   
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_2_REGISTER, 0x01, &cRejection);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetLowSidePowerSwitch(int PowerSwitch)
{
    unsigned int cPowerSwitch = PowerSwitch;
   
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_2_REGISTER, 0x01, &cPowerSwitch);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetCurrentDACOutput(int CurrentOutput)
{
    unsigned int cCurrentOutput = CurrentOutput;
   
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_2_REGISTER, 0x01, &cCurrentOutput);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetIDACRouting(int IDACRoute)
{
    unsigned int cIDACRoute = IDACRoute;
    
    // write the register value containing the new value back to the ADS
    WriteRegister(ADS1220_3_REGISTER, 0x01, &cIDACRoute);
    
    return ADS1220_NO_ERROR;
}

int ADS1220::SetDRDYMode(int DRDYMode)
{
    unsigned int cDRDYMode = DRDYMode;
   
    // write the register value containing the new gain code back to the ADS
    WriteRegister(ADS1220_3_REGISTER, 0x01, &cDRDYMode);
    
    return ADS1220_NO_ERROR;
}

/*
******************************************************************************
register get value commands
*/

int ADS1220::GetChannel(void)
{
    unsigned Temp;
    
    //Parse Mux data from register
    ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return (Temp >>4);
}

int ADS1220::GetGain(void)
{
    unsigned Temp;
    
    //Parse Gain data from register
    ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( (Temp & 0x0e) >>1);
}

int ADS1220::GetPGABypass(void)
{
    unsigned Temp;
    
    //Parse Bypass data from register
    ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return (Temp & 0x01);
}

int ADS1220::GetDataRate(void)
{
    unsigned Temp;
    
    //Parse DataRate data from register
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( Temp >>5 );
}

int ADS1220::GetClockMode(void)
{
    unsigned Temp;
    
    //Parse ClockMode data from register
      ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( (Temp & 0x18) >>3 );
}

int ADS1220::GetPowerDown(void)
{
    unsigned Temp;
    
    //Parse PowerDown data from register
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( (Temp & 0x04) >>2 );
}

int ADS1220::GetTemperatureMode(void)
{
    unsigned Temp;
    
    //Parse TempMode data from register
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( (Temp & 0x02) >>1 );
}

int ADS1220::GetBurnOutSource(void)
{
    unsigned Temp;
    
    //Parse BurnOut data from register
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( Temp & 0x01 );
}

int ADS1220::GetVoltageReference(void)
{
    unsigned Temp;
    
    //Parse VoltageRef data from register
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( Temp >>6 );
}

int ADS1220::Get50_60Rejection(void)
{
    unsigned Temp;
    
    //Parse Rejection data from register
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( (Temp & 0x30) >>4 );
}

int ADS1220::GetLowSidePowerSwitch(void)
{
    unsigned Temp;
    
    //Parse PowerSwitch data from register
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( (Temp & 0x08) >>3);
}

int ADS1220::GetCurrentDACOutput(void)
{
    unsigned Temp;
    
    //Parse IDACOutput data from register
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( Temp & 0x07 );
}

int ADS1220::GetIDACRouting(int WhichOne)
{
    // Check WhichOne sizing
    if (WhichOne >1) return ADS1220_ERROR;
    
    unsigned Temp;
    
    //Parse Mux data from register
    ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    if (WhichOne) return ( (Temp & 0x1c) >>2);
    
    else return ( Temp >>5 );
    
}

int ADS1220::GetDRDYMode(void)
{
    unsigned Temp;
    
    //Parse DRDYMode data from register
    ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
    
    // return the parsed data
    return ( (Temp & 0x02) >>1 );
}



/* Useful Functions within Main Program for Setting Register Contents
*
*   These functions show the programming flow based on the header definitions.
*   The calls are not made within the demo example, but could easily be used by calling the function
*       defined within the program to complete a fully useful program.
*   Similar function calls were made in the firwmare design for the ADS1220EVM.
*  
*  The following function calls use ASCII data sent from a COM port to control settings 
*   on the   The data is recontructed from ASCII and then combined with the
*   register contents to save as new configuration settings.
*
*   Function names correspond to datasheet register definitions
*/
void ADS1220::set_MUX(int c)
{       
          int mux = c - 48;
        int dERROR;
        unsigned Temp;      
    
        
    if (mux>=49 && mux<=54) mux -= 39;
    
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);

    Temp &= 0x0f;                                   // strip out old settings
    // Change Data rate
    switch(mux) {
        case 0:
            dERROR = SetChannel(Temp + ADS1220_MUX_0_1);
            break;
        case 1:
            dERROR = SetChannel(Temp + ADS1220_MUX_0_2);
            break;
        case 2:
            dERROR = SetChannel(Temp + ADS1220_MUX_0_3);
            break;
        case 3:
            dERROR = SetChannel(Temp + ADS1220_MUX_1_2);
            break;
        case 4:
            dERROR = SetChannel(Temp + ADS1220_MUX_1_3);
            break;
        case 5:
            dERROR = SetChannel(Temp + ADS1220_MUX_2_3);
            break;
        case 6:
            dERROR = SetChannel(Temp + ADS1220_MUX_1_0);
            break;
        case 7:
            dERROR = SetChannel(Temp + ADS1220_MUX_3_2);
            break;
        case 8:
            dERROR = SetChannel(Temp + ADS1220_MUX_0_G);
            break;
        case 9:
            dERROR = SetChannel(Temp + ADS1220_MUX_1_G);
            break;
        case 10:
            dERROR = SetChannel(Temp + ADS1220_MUX_2_G);
            break;
        case 11:
            dERROR = SetChannel(Temp + ADS1220_MUX_3_G);
            break;
        case 12:
            dERROR = SetChannel(Temp + ADS1220_MUX_EX_VREF);
            break;
        case 13:
            dERROR = SetChannel(Temp + ADS1220_MUX_AVDD);
            break;
        case 14:
            dERROR = SetChannel(Temp + ADS1220_MUX_DIV2);
            break;
        case 15:
            dERROR = SetChannel(Temp + ADS1220_MUX_DIV2);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;                                              
    }
    
    if (dERROR==ADS1220_ERROR)
        set_ERROR();
    
}

void ADS1220::set_GAIN(char c)
{
    int pga = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
    
    Temp &= 0xf1;                                   // strip out old settings
    // Change gain rate
    switch(pga) {
        case 0:
            dERROR = SetGain(Temp + ADS1220_GAIN_1);
            break;
        case 1:
            dERROR = SetGain(Temp + ADS1220_GAIN_2);
            break;
        case 2:
            dERROR = SetGain(Temp + ADS1220_GAIN_4);
            break;
        case 3:
            dERROR = SetGain(Temp + ADS1220_GAIN_8);
            break;
        case 4:
            dERROR = SetGain(Temp + ADS1220_GAIN_16);
            break;
        case 5:
            dERROR = SetGain(Temp + ADS1220_GAIN_32);
            break;
        case 6:
            dERROR = SetGain(Temp + ADS1220_GAIN_64);
            break;
        case 7:
            dERROR = SetGain(Temp + ADS1220_GAIN_128);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;  
        }
                                            
    
    
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
}

void ADS1220::set_PGA_BYPASS(char c)
{
    int buff = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_0_REGISTER, 0x01, &Temp);
        
    Temp &= 0xfe;                                   // strip out old settings
    // Change PGA Bypass
    switch(buff) {
        case 0:
            dERROR = SetPGABypass(Temp);
            break;
        case 1:
            dERROR = SetPGABypass(Temp + ADS1220_PGA_BYPASS);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
    
}

void ADS1220::set_DR(char c)
{
    int spd = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    Temp &= 0x1f;                                   // strip out old settings
    // Change Data rate
    switch(spd) {
        case 0:
            dERROR = SetDataRate(Temp + ADS1220_DR_20);
            break;
        case 1:
            dERROR = SetDataRate(Temp + ADS1220_DR_45);
            break;
        case 2:
            dERROR = SetDataRate(Temp + ADS1220_DR_90);
            break;
        case 3:
            dERROR = SetDataRate(Temp + ADS1220_DR_175);
            break;
        case 4:
            dERROR = SetDataRate(Temp + ADS1220_DR_330);
            break;
        case 5:
            dERROR = SetDataRate(Temp + ADS1220_DR_600);
            break;
        case 6:
            dERROR = SetDataRate(Temp + ADS1220_DR_1000);
            break;
        case 7:
            dERROR = SetDataRate(Temp + ADS1220_DR_1000);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
            
}

void ADS1220::set_MODE(char c)
{
    int spd = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    Temp &= 0xe7;                                   // strip out old settings
    // Change Data rate
    switch(spd) {
        case 0:
            dERROR = SetClockMode(Temp + ADS1220_MODE_NORMAL);
            break;
        case 1:
            dERROR = SetClockMode(Temp + ADS1220_MODE_DUTY);
            break;
        case 2:
            dERROR = SetClockMode(Temp + ADS1220_MODE_TURBO);
            break;
        case 3:
            dERROR = SetClockMode(Temp + ADS1220_MODE_DCT);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
        }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();

}

void ADS1220::set_CM(char c)
{
    int pwrdn = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    Temp &= 0xfb;                                   // strip out old settings
    // Change power down mode
    switch(pwrdn) {
        case 0:
            dERROR = SetPowerDown(Temp);
            break;
        case 1:
            dERROR = SetPowerDown(Temp + ADS1220_CC);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
    
}

void ADS1220::set_TS(char c)
{
    int tmp = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    Temp &= 0xfd;                                   // strip out old settings
    // Change Temp Diode Setting
    switch(tmp) {
        case 0:
            dERROR = SetTemperatureMode(Temp);
            break;
        case 1:
            dERROR = SetTemperatureMode(Temp + ADS1220_TEMP_SENSOR);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
    
}

void ADS1220::set_BCS(char c)
{
    int bo = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_1_REGISTER, 0x01, &Temp);
    
    Temp &= 0xfe;                                   // strip out old settings
    // Change PGA Bypass
    switch(bo) {
        case 0:
            dERROR = SetBurnOutSource(Temp);
            break;
        case 1:
            dERROR = SetBurnOutSource(Temp + ADS1220_BCS);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
    
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
        
}

void ADS1220::set_VREF(char c)
{
    int ref = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    Temp &= 0x3f;                                   // strip out old settings
    // Change Reference
    switch(ref) {
        case 0:
            dERROR = SetVoltageReference(Temp + ADS1220_VREF_INT);
            break;
        case 1:
            dERROR = SetVoltageReference(Temp + ADS1220_VREF_EX_DED);
            break;
        case 2:
            dERROR = SetVoltageReference(Temp + ADS1220_VREF_EX_AIN);
            break;
        case 3:
            dERROR = SetVoltageReference(Temp + ADS1220_VREF_SUPPLY);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
        
}
void ADS1220::set_50_60(char c)
{
    int flt = (int) c - 48;
    int dERROR;
    unsigned Temp;
    
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    Temp &= 0xcf;                                   // strip out old settings
    // Change Filter
    switch(flt) {
        case 0:
            dERROR = Set50_60Rejection(Temp + ADS1220_REJECT_OFF);
            break;
        case 1:
            dERROR = Set50_60Rejection(Temp + ADS1220_REJECT_BOTH);
            break;
        case 2:
            dERROR = Set50_60Rejection(Temp + ADS1220_REJECT_50);
            break;
        case 3:
            dERROR = Set50_60Rejection(Temp + ADS1220_REJECT_60);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
        
}

void ADS1220::set_PSW(char c)
{
    int sw = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    Temp &= 0xf7;                                   // strip out old settings
        // Change power down mode
    switch(sw) {
        case 0:
            dERROR = SetLowSidePowerSwitch(Temp);
            break;
        case 1:
            dERROR = SetLowSidePowerSwitch(Temp + ADS1220_PSW_SW);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
    
}

void ADS1220::set_IDAC(char c)
{
    int current = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_2_REGISTER, 0x01, &Temp);
    
    Temp &= 0xf8;                                   // strip out old settings
    // Change IDAC Current Output
    switch(current) {
        case 0:
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_OFF);
            break;
        case 1:
          #ifdef ADS1120
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_OFF);
          #else
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_10);
          #endif

            break;
        case 2:
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_50);
            break;
        case 3:
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_100);
            break;
        case 4:
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_250);
            break;
        case 5:
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_500);
            break;
        case 6:
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_1000);
            break;
        case 7:
            dERROR = SetCurrentDACOutput(Temp + ADS1220_IDAC_2000);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
        
        }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
    
}

void ADS1220::set_IMUX(char c, int i)
{
    int mux = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
    
    if (i==1) {
        Temp &= 0xe3;                                   // strip out old settings
        
        // Change IDAC2 MUX Output
    
        switch(mux) {
            case 0:
                Temp |= ADS1220_IDAC2_OFF;
                break;
            case 1:
                Temp |= ADS1220_IDAC2_AIN0;
                break;
            case 2:
                Temp |= ADS1220_IDAC2_AIN1;
                break;
            case 3:
                Temp |= ADS1220_IDAC2_AIN2;
                break;
            case 4:
                Temp |= ADS1220_IDAC2_AIN3;
                break;
            case 5:
                Temp |= ADS1220_IDAC2_REFP0;
                break;
            case 6:
                Temp |= ADS1220_IDAC2_REFN0;
                break;
            case 7:
                Temp |= ADS1220_IDAC2_REFN0;
                break;
            default:
                dERROR = ADS1220_ERROR;
                break;
        
        }
    }
    else {
        Temp &= 0x1f;
        // Change IDAC1 MUX Output
        switch(mux) {
            case 0:
                Temp |= ADS1220_IDAC1_OFF;
                break;
            case 1:
                Temp |= ADS1220_IDAC1_AIN0;
                break;
            case 2:
                Temp |= ADS1220_IDAC1_AIN1;
                break;
            case 3:
                Temp |= ADS1220_IDAC1_AIN2;
                break;
            case 4:
                Temp |= ADS1220_IDAC1_AIN3;
                break;
            case 5:
                Temp |= ADS1220_IDAC1_REFP0;
                break;
            case 6:
                Temp |= ADS1220_IDAC1_REFN0;
                break;
            case 7:
                Temp |= ADS1220_IDAC1_REFN0;
                break;
            default:
                dERROR = ADS1220_ERROR;
                break;
        }
    }
    
    if (dERROR==ADS1220_NO_ERROR) 
        dERROR = SetIDACRouting(Temp); 
    
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
    
}

void ADS1220::set_DRDYM(char c)
{
    int drdy = (int) c - 48;
    int dERROR;
    unsigned Temp;
        
    // the DataRate value is only part of the register, so we have to read it back
    // and massage the new value into it
    ReadRegister(ADS1220_3_REGISTER, 0x01, &Temp);
    
    Temp &= 0xfd;                                   // strip out old settings
    // Change DRDY Mode Setting
    switch(drdy) {
        case 0:
            dERROR = SetDRDYMode(Temp);
            break;
        case 1:
            dERROR = SetDRDYMode(Temp + ADS1220_DRDY_MODE);
            break;
        default:
            dERROR = ADS1220_ERROR;
            break;
    
    }
                                
    if (dERROR==ADS1220_ERROR) 
        set_ERROR();
    
}




void ADS1220::set_ERROR_Transmit(void)
{
    /* De-initialize the SPI comunication BUS */
}

void ADS1220::set_ERROR_Receive(void)
{
    /* De-initialize the SPI comunication BUS */
// serial.printf("\rSPI Failed during Reception\n\r");
}

void ADS1220::set_ERROR(void)
{
    /* De-initialize the SPI comunication BUS */
 //serial.printf("\rADS1220 Error\n\r");
}
