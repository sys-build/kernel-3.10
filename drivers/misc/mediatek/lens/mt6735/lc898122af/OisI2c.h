//********************************************************************************
//		<< GPIO software I2C >>
//	    File Name	: OisI2c.h
//		Author		: ON Semiconductor
//********************************************************************************
#ifndef	_OISI2C_H_
#define	_OISI2C_H_

#define	_GPIO_BUS_H	1
#define	_GPIO_BUS_L	0

//*************************************************************************************************
//*************************************************************************************************
//	Own coding by customer for each system.
//*************************************************************************************************
void			GPIO_SoftwareI2cBus( void );
void			GPIO_PeripheralI2cBus( void );
int				GPIO_getScl( void );
void			GPIO_setScl( int );
int				GPIO_getSda( void );
void			GPIO_setSda( int );
void			GPIO_Wait( int pcnt );



//*************************************************************************************************
//	Provided functions.
//*************************************************************************************************
void			GPIO_WriteI2cOutBus( void );
void			GPIO_StopI2cOutBus( void );
void			GPIO_LessStartCondition( void );
void			GPIO_LessStopCondition( void );
void			GPIO_StartCondition( void );
void			GPIO_StopCondition( void );
void			GPIO_SendByte( unsigned char );
unsigned char	GPIO_RecvByte( void );
void			GPIO_SendData( unsigned int adrs, unsigned int );
unsigned int	GPIO_RecvData( unsigned int );
int				GPIO_DspLessSendByte( unsigned char, unsigned char );
void			GPIO_DSPLessMode( void );

int				DSPLESSEraseMagicCode( void );

#endif /* _OISI2C_H_ */
