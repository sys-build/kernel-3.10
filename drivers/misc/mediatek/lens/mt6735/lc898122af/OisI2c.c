//********************************************************************************
//		<< GPIO software I2C >>
//	    File Name	: OisI2c.c
//		Author		: ON Semiconductor
//********************************************************************************
//**************************
//	Include Header File		
//**************************
#include	"Ois.h"
//#include	"I2c.h"		// for ATMEL
#include <linux/i2c.h>
#include	"OisI2c.h"

//*************************************************************************************************
//*************************************************************************************************
//	Own coding by customer for each system.
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//	Change to GPIOl mode I2C
//*************************************************************************************************
void GPIO_SoftwareI2cBus( void )
{
	// for ATMEL
	//InitializeI2cBus();
}

//*************************************************************************************************
//	Change to peripheral mode I2C
//*************************************************************************************************
void GPIO_PeripheralI2cBus( void )
{
	// for ATMEL
	//ShutdownI2cBus();
}

//*************************************************************************************************
//	Get state of SCL
//*************************************************************************************************
int GPIO_getScl( void )
{
	// for ATMEL
	return 0;
}

//*************************************************************************************************
//	Set state to SCL
//*************************************************************************************************
void GPIO_setScl( int pflag )
{
	// for ATMEL
	//setI2cBusScl( pflag );
}

//*************************************************************************************************
//	Get state of SDA
//*************************************************************************************************
int GPIO_getSda( void )
{
	// for ATMEL
	return 0;
	//return getI2cBusSda();
}

//*************************************************************************************************
//	Set state to SDA
//*************************************************************************************************
void GPIO_setSda( int pflag )
{
	// for ATMEL
	//setI2cBusSda( pflag );
}

//*************************************************************************************************
//	Wait loop 
//*************************************************************************************************
void GPIO_Wait( int pcnt )
{
	int i;

	if( pcnt != 0 ) {
		for( i = 0; i < pcnt; i++ );
	}
}



//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//	Provided functions.
//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_WriteI2cOutBus( void )
{
	GPIO_setScl( _GPIO_BUS_H );				// SCL	rise(1)
	GPIO_setSda( _GPIO_BUS_H );				// SDA	high(1)
	GPIO_Wait( 3 );							// Wait
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_StopI2cOutBus( void )
{
	GPIO_setScl( _GPIO_BUS_H );				// SCL	rise(1)
	GPIO_setSda( _GPIO_BUS_H );				// SDA	high(1)
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_LessStartCondition( void )
{
	GPIO_setScl( _GPIO_BUS_H );				// SCL	rise(1)
	GPIO_setSda( _GPIO_BUS_H );				// SDA	high(1)
	GPIO_setScl( _GPIO_BUS_L );				// SCL	fall(0)
	GPIO_Wait( 0 );							// Wait
	GPIO_setSda( _GPIO_BUS_L );				// SDA	Low(0)
	GPIO_Wait( 0 );							// Wait
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_LessStopCondition( void )
{
	GPIO_Wait( 1 );						// Wait
	GPIO_setScl( _GPIO_BUS_L );			// SCL	fall
	GPIO_Wait( 0 );						// Wait
	GPIO_setSda( _GPIO_BUS_H );			// SDA	high(1)
	GPIO_Wait( 1 );						// Wait
	GPIO_setScl( _GPIO_BUS_H );			// SCL	rise
	GPIO_Wait( 1 );						// Wait
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_StartCondition(void)
{
	GPIO_setScl( _GPIO_BUS_H );				// SCL	rise
	GPIO_setSda( _GPIO_BUS_H );				// SDA	high(1)
	GPIO_setSda( _GPIO_BUS_L );				// SDA	low(0)
	GPIO_Wait( 0 );							// Wait
	GPIO_setScl( _GPIO_BUS_L );				// SCL	fall
	GPIO_Wait( 0 );							// Wait
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_StopCondition(void)
{
	GPIO_Wait( 1 );							// Wait
	GPIO_setSda( _GPIO_BUS_L );				// SDA	low(0)
	GPIO_Wait( 0 );							// Wait
	GPIO_setScl( _GPIO_BUS_H );				// SCL	rise
	GPIO_Wait( 1 );							// Wait
	GPIO_setSda( _GPIO_BUS_H );				// SCL	rise
	GPIO_Wait( 1 );							// Wait
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_SendByte( unsigned char pdata )
{
	int i;

	for ( i = 0; i < 8; i++ ) {
		if ( ( pdata & ( 0x80 >> i ) ) != 0 ) {
			GPIO_setSda( _GPIO_BUS_H );		// data		high(1)
			GPIO_setScl( _GPIO_BUS_H );		// clock	rise
		} else {
			GPIO_setSda( _GPIO_BUS_L );		// data		low(0)
			GPIO_setScl( _GPIO_BUS_H );		// clock	rise
		}
		GPIO_Wait( 0 );						// Wait
		GPIO_setScl( _GPIO_BUS_L );			// clock	fall
		GPIO_Wait( 0 );						// Wait
	}
}

//*************************************************************************************************
//
//*************************************************************************************************
unsigned char GPIO_RecvByte( void )
{
	unsigned char pByte = 0x00;
	int i;

	for ( i = 0; i < 8; i++ ) {
		GPIO_setScl( _GPIO_BUS_H );			// SCL		rise
		GPIO_Wait(0); 						// Wait
		pByte <<= 1;
		if( GPIO_getSda() )					// SDA		read
			pByte |= 0x01;

		GPIO_setScl( _GPIO_BUS_L );			// SCL		fall
		GPIO_Wait(0); 						// Wait
	}
	GPIO_Wait(1); 							// Wait

	GPIO_setScl( _GPIO_BUS_L );				// SCL		fall
	GPIO_setSda( _GPIO_BUS_H );				// SDA		high(1)

	return pByte;
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_SendData( unsigned int adrs, unsigned int data )
{
	GPIO_WriteI2cOutBus();					// SCL & SDA High
	GPIO_StartCondition();					// make start condition

	GPIO_SendByte( (unsigned char)((adrs>>16) | 0x80) );
	GPIO_SendByte( (unsigned char)(adrs>>8) );
	GPIO_SendByte( (unsigned char)(adrs)    );

	GPIO_SendByte( (unsigned char)(data>>24) );
	GPIO_SendByte( (unsigned char)(data>>16) );
	GPIO_SendByte( (unsigned char)(data>>8) );
	GPIO_SendByte( (unsigned char)(data) );
	
	GPIO_StopCondition();
	GPIO_StopI2cOutBus();
}

//*************************************************************************************************
//
//*************************************************************************************************
unsigned int GPIO_RecvData( unsigned int adrs )
{
	unsigned int ans;

	GPIO_WriteI2cOutBus();					// SCL & SDA High
	GPIO_StartCondition();					// make start condition

	GPIO_SendByte( (unsigned char)((adrs>>16) & 0x7F) );
	GPIO_SendByte( (unsigned char)(adrs>>8) );
	GPIO_SendByte( (unsigned char)(adrs)    );

	GPIO_SendByte( 0x00 );					// dummy

	ans  = (GPIO_RecvByte() << 24);
	ans |= (GPIO_RecvByte() << 16);
	ans |= (GPIO_RecvByte() << 8);
	ans |= GPIO_RecvByte();

	GPIO_StopCondition();
	GPIO_StopI2cOutBus();
	
	return( ans );
}


//*************************************************************************************************
//
//*************************************************************************************************
int GPIO_DspLessSendByte( unsigned char pdata, unsigned char pAck )
{
	int i;

	// exchange the scl and ada line
	for ( i = 0; i < 8; i++ ) {
		if ( ( pdata & ( 0x80 >> i ) ) != 0 ) {
			GPIO_setScl( _GPIO_BUS_H );		// data		high(1)
			GPIO_setSda( _GPIO_BUS_H );		// clock	rise
		} else {
			GPIO_setScl( _GPIO_BUS_L );		// data		low(0)
			GPIO_setSda( _GPIO_BUS_H );		// clock	rise
		}
		GPIO_Wait( 0 );						// Wait
		GPIO_setSda( _GPIO_BUS_L );			// clock	fall
		GPIO_Wait( 0 );						// Wait
	}
	
	if( pAck ){
		GPIO_setScl( _GPIO_BUS_H );			// data		Ack high(1)
	}else{
		GPIO_setScl( _GPIO_BUS_L );			// data		Ack low(0)
	}
	GPIO_setSda( _GPIO_BUS_H );				// clock	rise
	GPIO_Wait( 0 );							// Wait
	GPIO_setSda( _GPIO_BUS_L );				// clock	low(0)
	GPIO_Wait( 0 );							// Wait

	return pAck;
}

//*************************************************************************************************
//
//*************************************************************************************************
void GPIO_DSPLessMode( void )
{
	GPIO_WriteI2cOutBus();					// SCL & SDA High

	// First time
	GPIO_LessStartCondition();				// make start condition

	GPIO_DspLessSendByte( 0x7C, 1 );
	GPIO_DspLessSendByte( 0xAA, 0 );

	GPIO_LessStopCondition();

	// Second time
	GPIO_LessStartCondition();

	GPIO_DspLessSendByte( 0x7C, 1 );
	GPIO_DspLessSendByte( 0xAA, 0 );

	GPIO_LessStopCondition();

	GPIO_StopI2cOutBus();
}

//********************************************************************************
// Function Name 	: DSPLESSEraseMagicCode
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Erase magic code by DSP LESS MODE
// History			: First edition 						
//********************************************************************************
int	DSPLESSEraseMagicCode( void )
{
	unsigned int	UlReadVal;
	unsigned short	UsNum;

	GPIO_SoftwareI2cBus();									// Peripheral I2C to software I2C

	GPIO_DSPLessMode();										// Change to DSP less mode

	// CVER read
	UlReadVal = GPIO_RecvData ( CVER_123 );
	if ( UlReadVal != CVER_123A ) {
		GPIO_SendData( STBOSCPLL, OSC_STB );				// Stop OSC
		GPIO_PeripheralI2cBus();
		return 1;
	}

	// Release interface reset
	UlReadVal = GPIO_RecvData ( SOFTRESET );
	UlReadVal |= 0x00000010;								// Reset release
	GPIO_SendData( SOFTRESET, UlReadVal );

	// Initialize flash interface
	GPIO_SendData( FLASHROM_TPGS, 118 );					// TPGS Flash spec.  min. 2.5usec max. 3.15uec
	GPIO_SendData( FLASHROM_TPROG, 70 );					// TPROG Flash spec.  min. 6usec max. 7.5usec
	GPIO_SendData( FLASHROM_TERASES, 92 );					// TERASES Flash spec.  Flash spec.  min. 4msec max. 5msec
	GPIO_SendData( FLASHROM_TERASEC, 115 );					// TERASEC Flash spec.  min. 40msec max. 50msec
	GPIO_SendData( FLASHROM_SEL, 7 );						// ARRAY SEL

	// set configuration initialize
	GPIO_SendData( FLASHROM_ADR, 0x00000000	  );
	GPIO_SendData( FLASHROM_ACSCNT, 0 );
	GPIO_SendData( FLASHROM_WPB, 1 );						// Disable write protect

	for( UsNum = 0; UsNum < 8; UsNum++ )
	{
		GPIO_SendData( FLASHROM_WDAT, 0xFFFFFFFF ); 
		GPIO_SendData( FLASHROM_CMD, 3);  					// Setconfig
	}

	// set auto configuration
	GPIO_SendData( FLASHROM_ADR, 0x00010100	  );
	GPIO_SendData( FLASHROM_ACSCNT, 7 );
	GPIO_SendData( FLASHROM_CMD, 7 );  						// Auto configuration
	GPIO_SendData( FLASHROM_SEL, 1 );						// ARRAY SEL

	// Erase Magic Code
	GPIO_SendData( FLASHROM_ACSCNT, 0 );					// Count
	GPIO_SendData( FLASHROM_WDAT, 0 );						// 0 data

	for ( UsNum = 1; UsNum < 8; UsNum++ ) {
		// NVR Addres Set
		GPIO_SendData( FLASHROM_ADR, 0x00010000 + UsNum  );
		GPIO_SendData( FLASHROM_CMD, 2);  					// Program
	}

	GPIO_SendData( FLASHROM_WPB, 0 );						// Enable write protect

	GPIO_SendData( STBOSCPLL, 1 );							// Reset analog circuit(reboot)

	GPIO_PeripheralI2cBus();
	return 0;
}
