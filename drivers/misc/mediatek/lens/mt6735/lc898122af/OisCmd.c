//********************************************************************************
//
//		<< LC898123 Evaluation Soft >>
//	    Program Name	: OisCmd.c
//		Design			: Y.Shigoeka
//		History			: First edition						
//********************************************************************************
//**************************
//	Include Header File		
//**************************
#define		OISCMD

#include	"Ois.h"
//#include	<stdlib.h>

#ifndef  abs
#define abs(a) (((a) < 0) ? -(a) : (a))
#endif

UINT8	Flash_Sector[ 256 ];

#define	FLASH_SECTOR_BUFFER		Flash_Sector

//**************************
//	Local Function Prottype	
//**************************
void	IniCmd( void ) ;									// Command Execute Process Initial
void	IniPtAve( void ) ;									// Average setting
void	MesFil( UINT8 ) ;									// Measure Filter Setting
void	MeasureStart( INT32 , INT32 , INT32 ) ;				// Measure Start Function
void	MeasureWait( void ) ;								// Measure Wait
void	MemoryClear( UINT16 , UINT16 ) ;					// Memory Cloear
void	SetWaitTime( UINT16 ) ; 							// Set Wait Timer

UINT32	TneOff( UnDwdVal, UINT8 ) ;							// Hall Offset Tuning
UINT32	TneBia( UnDwdVal, UINT8 ) ;							// Hall Bias Tuning


void	SetSineWave(   UINT8 , UINT8 );
UINT8	TstActMov( UINT8 );



//**************************
//	define					
//**************************
#define 	MES_XG1			0								// LXG1 Measure Mode
#define 	MES_XG2			1								// LXG2 Measure Mode

#define 	HALL_ADJ		0
#define 	LOOPGAIN		1
#define 	THROUGH			2
#define 	NOISE			3
#define		OSCCHK			4

// Measure Mode

 #define 	TNE 			80								// Waiting Time For Movement

 /******* Hall calibration Type 1 *******/
 
 #define 	OFFSET_DIV		2								// Divide Difference For Offset Step
 #define 	TIME_OUT		20								// Time Out Count
 #define	BIAS_HLMT		(UINT32)0xBF000000
 #define	BIAS_LLMT		(UINT32)0x20000000
 #define 	MARGIN			0x0300							// Margin

 #define 	BIAS_ADJ_RANGE_XY	0xCCCC						// 80%

 #define 	HALL_MAX_RANGE_XY	BIAS_ADJ_RANGE_XY + MARGIN
 #define 	HALL_MIN_RANGE_XY	BIAS_ADJ_RANGE_XY - MARGIN


#ifdef	SEL_CLOSED_AF
 #define	BIAS_ADJ_RANGE_Z	0xAFDE						// 68.7%
 #define 	HALL_MAX_RANGE_Z	BIAS_ADJ_RANGE_Z + MARGIN
 #define 	HALL_MIN_RANGE_Z	BIAS_ADJ_RANGE_Z - MARGIN
#endif

 #define 	DECRE_CAL		0x0100							// decrease value
/***************************************/
#define		SLT_OFFSET		(0x0E55)
#define		LENS_MARGIN		(0x0800)
#define		PIXEL_SIZE		(1.12f)							// pixel size 1.12um
#define		SPEC_RANGE		(85.0f)							// spec need movable range 85um
//#define		SPEC_PIXEL		(PIXEL_SIZE * SPEC_RANGE)		// spec need movable range pixel
#define		SPEC_PIXEL		(SPEC_RANGE / PIXEL_SIZE)		// spec need movable range pixel
/***************************************/
// Threshold of osciration amplitude
#define ULTHDVAL	0x01000000								// Threshold of the hale value


//**************************
//	Global Variable			
//**************************
INT16		SsNvcX = 1 ;									// NVC move direction X
INT16		SsNvcY = 1 ;									// NVC move direction Y




//**************************
//	Const					
//**************************
//********************************************************************************
// Function Name 	: IniCmd
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Command Execute Process Initial
// History			: First edition 						
//********************************************************************************
void	IniCmd( void )
{

	MemClr( ( UINT8 * )&StAdjPar, sizeof( stAdjPar ) ) ;	// Adjust Parameter Clear
	
}


//********************************************************************************
// Function Name 	: MemClr
// Retun Value		: void
// Argment Value	: Clear Target Pointer, Clear Byte Number
// Explanation		: Memory Clear Function
// History			: First edition 						
//********************************************************************************
void	MemClr( UINT8	*NcTgtPtr, UINT16	UsClrSiz )
{
	UINT16	UsClrIdx ;

	for ( UsClrIdx = 0 ; UsClrIdx < UsClrSiz ; UsClrIdx++ )
	{
		*NcTgtPtr	= 0 ;
		NcTgtPtr++ ;
	}
}



//********************************************************************************
// Function Name 	: WitTim
// Retun Value		: NON
// Argment Value	: Wait Time(ms)
// Explanation		: Timer Wait Function
// History			: First edition 						
//********************************************************************************
#if 0//float disabled
void	WitTim( UINT16	UsWitTim )
{
	UINT32	UlLopIdx, UlWitCyc ;

	UlWitCyc	= ( UINT32 )( ( float )UsWitTim / NOP_TIME / ( float )12 ) ;

	for( UlLopIdx = 0 ; UlLopIdx < UlWitCyc ; UlLopIdx++ )
	{
		;
	}
}
#endif



//********************************************************************************
// Function Name 	: TneRun
// Retun Value		: Hall Tuning SUCCESS or FAILURE
// Argment Value	: NON
// Explanation		: Hall System Auto Adjustment Function
// History			: First edition 						
//********************************************************************************
UINT32	TneRun( void )
{
	UINT32	UlHlySts, UlHlxSts, UlAtxSts, UlAtySts, UlGvcSts ;
//	UINT8	UcDrvMod ;
	UnDwdVal		StTneVal ;
	UINT32	UlFinSts , UlReadVal ;

	// Check relation the firmware version of LC898123
	/* Chaeck Driver gain */
	UlHlxSts = UlHlySts = 0x00 ;
	RamRead32A( CURDRV_X_SiPlsVal , &UlReadVal ) ;
	if( ( UlReadVal > (UINT32)0x7FFFFFFF ) || ( (UINT32)0x60000000 > UlReadVal )){
		UlHlxSts = EXE_HXADJ;
	}
	RamRead32A( CURDRV_X_SiMnsVal , &UlReadVal ) ;
	if( ( UlReadVal > (UINT32)0x7FFFFFFF ) || ( (UINT32)0x60000000 > UlReadVal )){
		UlHlxSts = EXE_HXADJ;
	}
	RamRead32A( CURDRV_Y_SiPlsVal , &UlReadVal ) ;
	if( ( UlReadVal > (UINT32)0x7FFFFFFF ) || ( (UINT32)0x60000000 > UlReadVal )){
		UlHlySts = EXE_HYADJ;
	}
	RamRead32A( CURDRV_Y_SiMnsVal , &UlReadVal ) ;
	if( ( UlReadVal > (UINT32)0x7FFFFFFF ) || ( (UINT32)0x60000000 > UlReadVal )){
		UlHlySts = EXE_HYADJ;
	}
	if( UlHlxSts || UlHlySts ){
		UlFinSts	= UlHlxSts | UlHlySts ;
		StAdjPar.StHalAdj.UlAdjPhs = UlFinSts ;
		return( UlFinSts ) ;
	}


	RtnCen( BOTH_OFF ) ;		// Both OFF
	WitTim( TNE ) ;

	RamWrite32A( HALL_RAM_HXOFF,  0x00000000 ) ;		// X Offset Clr
	RamWrite32A( HALL_RAM_HYOFF,  0x00000000 ) ;		// Y Offset Clr
	RamWrite32A( HallFilterCoeffX_hxgain0 , SXGAIN_LOP ) ;
	RamWrite32A( HallFilterCoeffY_hygain0 , SYGAIN_LOP ) ;
	DacControl( 0 , HLXBO , XY_BIAS ) ;
	RamWrite32A( StCaliData_UiHallBias_X , XY_BIAS ) ;
	DacControl( 0 , HLYBO , XY_BIAS ) ;
	RamWrite32A( StCaliData_UiHallBias_Y , XY_BIAS ) ;
	DacControl( 0 , HLXO, XY_OFST ) ;
	RamWrite32A( StCaliData_UiHallOffset_X , XY_OFST ) ;
	DacControl( 0 , HLYO, XY_OFST ) ;
	RamWrite32A( StCaliData_UiHallOffset_Y , XY_OFST ) ;

	StTneVal.UlDwdVal	= TnePtp( Y_DIR , PTP_BEFORE ) ;
	UlHlySts	= TneCen( Y_DIR, StTneVal ) ;
	
	RtnCen( YONLY_ON ) ;		// Y ON / X OFF
	WitTim( TNE ) ;

	StTneVal.UlDwdVal	= TnePtp( X_DIR , PTP_BEFORE ) ;
	UlHlxSts	= TneCen( X_DIR, StTneVal ) ;

	if( (UlHlxSts != EXE_HXADJ) && (UlHlySts != EXE_HYADJ) ){
		RtnCen( XONLY_ON ) ;		// Y OFF / X ON
		WitTim( TNE ) ;

		StTneVal.UlDwdVal	= TnePtp( Y_DIR , PTP_AFTER ) ;
		UlHlySts	= TneCen( Y_DIR, StTneVal ) ;
	
		RtnCen( YONLY_ON ) ;		// Y ON / X OFF
		WitTim( TNE ) ;

		StTneVal.UlDwdVal	= TnePtp( X_DIR , PTP_AFTER ) ;
		UlHlxSts	= TneCen( X_DIR, StTneVal ) ;
	
		RtnCen( BOTH_OFF ) ;		// Both OFF
	
#ifdef	NEUTRAL_CENTER
		TneHvc();
#endif	//NEUTRAL_CENTER
	

		WitTim( TNE ) ;

		StAdjPar.StHalAdj.UsAdxOff = StAdjPar.StHalAdj.UsHlxCna  ;
		StAdjPar.StHalAdj.UsAdyOff = StAdjPar.StHalAdj.UsHlyCna  ;
		
	
		RamWrite32A( HALL_RAM_HXOFF,  (UINT32)((StAdjPar.StHalAdj.UsAdxOff << 16 ) & 0xFFFF0000 )) ;
		RamWrite32A( HALL_RAM_HYOFF,  (UINT32)((StAdjPar.StHalAdj.UsAdyOff << 16 ) & 0xFFFF0000 )) ;
		
		RamRead32A( StCaliData_UiHallOffset_X , &UlReadVal ) ;
		StAdjPar.StHalAdj.UsHlxOff = (UINT16)( UlReadVal >> 16 ) ;
		
		RamRead32A( StCaliData_UiHallBias_X , &UlReadVal ) ;
		StAdjPar.StHalAdj.UsHlxGan = (UINT16)( UlReadVal >> 16 ) ;
		
		RamRead32A( StCaliData_UiHallOffset_Y , &UlReadVal ) ;
		StAdjPar.StHalAdj.UsHlyOff = (UINT16)( UlReadVal >> 16 ) ;
		
		RamRead32A( StCaliData_UiHallBias_Y , &UlReadVal ) ;
		StAdjPar.StHalAdj.UsHlyGan = (UINT16)( UlReadVal >> 16 ) ;
		
#ifdef	NEUTRAL_CENTER_FINE
		TneFin();

		RamWrite32A( HALL_RAM_HXOFF,  (UINT32)((StAdjPar.StHalAdj.UsAdxOff << 16 ) & 0xFFFF0000 )) ;
		RamWrite32A( HALL_RAM_HYOFF,  (UINT32)((StAdjPar.StHalAdj.UsAdyOff << 16 ) & 0xFFFF0000 )) ;
#endif	//NEUTRAL_CENTER_FINE

		RtnCen( BOTH_ON ) ;		// Y ON / X ON

		WitTim( TNE ) ;

		// X Loop Gain Adjust
		UlAtxSts	= LopGan( X_DIR ) ;
	
		// Y Loop Gain Adjust
		UlAtySts	= LopGan( Y_DIR ) ;
	
	}else{
		RtnCen( BOTH_OFF ) ;		// Both OFF
		WitTim( TNE ) ;
		UlAtxSts = EXE_LXADJ;
		UlAtySts = EXE_LYADJ;
	}
	
	UlGvcSts = TneGvc() ;

	UlFinSts	= ( UlHlxSts - EXE_END ) + ( UlHlySts - EXE_END ) + ( UlAtxSts - EXE_END ) + ( UlAtySts - EXE_END )  + ( UlGvcSts - EXE_END ) + EXE_END ;
	StAdjPar.StHalAdj.UlAdjPhs = UlFinSts ;

	
	return( UlFinSts ) ;
}


#ifdef	SEL_CLOSED_AF
//********************************************************************************
// Function Name 	: TneRunZ
// Retun Value		: Z axis Hall Tuning SUCCESS or FAILURE
// Argment Value	: NON
// Explanation		: Z_axis Hall System Auto Adjustment Function
// History			: First edition 						
//********************************************************************************
UINT32	TneRunZ( void )
{
	UINT32	UlHlzSts, UlAtzSts ;
	UnDwdVal		StTneVal ;
	UINT32	UlFinSts , UlReadVal ;

	
	RtnCen( ZONLY_OFF ) ;
	WitTim( TNE ) ;

	
	RamWrite32A( CLAF_RAMA_AFOFFSET,  0x00000000 ) ;		// Z Offset Clr
	RamWrite32A( CLAF_Gain_afloop2 , SZGAIN_LOP ) ;
	DacControl( 0 , HLAFBO , Z_BIAS ) ;
	RamWrite32A( StCaliData_UiHallBias_AF , Z_BIAS) ;
	DacControl( 0 , HLAFO, Z_OFST ) ;
	RamWrite32A( StCaliData_UiHallOffset_AF , Z_OFST ) ;

	StTneVal.UlDwdVal	= TnePtp( Z_DIR , PTP_BEFORE ) ;
	UlHlzSts	= TneCen( Z_DIR, StTneVal ) ;
	
	WitTim( TNE ) ;

	UlReadVal = 0x00010000 - (UINT32)StAdjPar.StHalAdj.UsHlzCna ;
	StAdjPar.StHalAdj.UsAdzOff = (UINT16)UlReadVal ;
	
	
	RamWrite32A( CLAF_RAMA_AFOFFSET,  (UINT32)((StAdjPar.StHalAdj.UsAdzOff << 16 ) & 0xFFFF0000 )) ;
	
	RamRead32A( StCaliData_UiHallOffset_AF , &UlReadVal ) ;
	StAdjPar.StHalAdj.UsHlzOff = (UINT16)( UlReadVal >> 16 ) ;
	
	RamRead32A( StCaliData_UiHallBias_AF , &UlReadVal ) ;
	StAdjPar.StHalAdj.UsHlzGan = (UINT16)( UlReadVal >> 16 ) ;
	
	RtnCen( ZONLY_ON ) ;		// Z ON
	
	WitTim( TNE ) ;

	// Z Loop Gain Adjust
	UlAtzSts	= LopGan( Z_DIR ) ;
//	UlAtzSts	= EXE_END ;

	UlFinSts	= ( UlHlzSts - EXE_END ) + ( UlAtzSts - EXE_END ) + EXE_END ;
	StAdjPar.StHalAdj.UlAdjPhs = UlFinSts ;

	
	return( UlFinSts ) ;
}

//********************************************************************************
// Function Name 	: TneRunA
// Retun Value		: AF + OIS Hall Tuning SUCCESS or FAILURE
// Argment Value	: NON
// Explanation		: AF + OIS Hall System Auto Adjustment Function
// History			: First edition 						
//********************************************************************************
UINT32	TneRunA( void )
{
	UINT32	UlFinSts ;

	UlFinSts = TneRunZ();
	UlFinSts |= TneRun();
	
	StAdjPar.StHalAdj.UlAdjPhs = UlFinSts ;
	return( UlFinSts ) ;
}

#endif

//********************************************************************************
// Function Name 	: TnePtp
// Retun Value		: Hall Top & Bottom Gaps
// Argment Value	: X,Y Direction, Adjust Before After Parameter
// Explanation		: Measuring Hall Paek To Peak
// History			: First edition 						
//********************************************************************************

UINT32	TnePtp ( UINT8	UcDirSel, UINT8	UcBfrAft )
{
	UnDwdVal		StTneVal ;
	INT32			SlMeasureParameterA , SlMeasureParameterB ;
	INT32			SlMeasureParameterNum ;
	INT32			SlMeasureMaxValue , SlMeasureMinValue ;

//	SlMeasureParameterNum	=	2004 ;		// 20.0195/0.010 < x
	SlMeasureParameterNum	=	2000 ;		// 20.0195/0.010 < x

	if( UcDirSel == X_DIR ) {								// X axis
		SlMeasureParameterA		=	HALL_RAM_HXIDAT ;		// Set Measure RAM Address
		SlMeasureParameterB		=	HALL_RAM_HYIDAT ;		// Set Measure RAM Address
	} else if( UcDirSel == Y_DIR ) {						// Y axis
		SlMeasureParameterA		=	HALL_RAM_HYIDAT ;		// Set Measure RAM Address
		SlMeasureParameterB		=	HALL_RAM_HXIDAT ;		// Set Measure RAM Address
#ifdef	SEL_CLOSED_AF
	} else {												// Z axis
		SlMeasureParameterA		=	CLAF_RAMA_AFADIN ;		// Set Measure RAM Address
		SlMeasureParameterB		=	CLAF_RAMA_AFADIN ;		// Set Measure RAM Address
#endif
	}
	SetSinWavGenInt();
	
	RamWrite32A( SinWave_Offset		,	0x105E36 ) ;				// Freq Setting = Freq * 80000000h / Fs	: 10Hz
	RamWrite32A( SinWave_Gain		,	0x7FFFFFFF ) ;				// Set Sine Wave Gain
	RamWrite32A( SinWaveC_Regsiter	,	0x00000001 ) ;				// Sine Wave Start
	if( UcDirSel == X_DIR ) {
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)HALL_RAM_SINDX1 ) ;	// Set Sine Wave Input RAM
	}else if( UcDirSel == Y_DIR ){
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)HALL_RAM_SINDY1 ) ;	// Set Sine Wave Input RAM
#ifdef	SEL_CLOSED_AF
	}else{
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)CLAF_RAMA_AFOUT ) ;	// Set Sine Wave Input RAM
#endif
	}
	
	MesFil( THROUGH ) ;					// Filter setting for measurement

	MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;					// Start measure
	
	MeasureWait() ;						// Wait complete of measurement
	
	RamWrite32A( SinWaveC_Regsiter	,	0x00000000 ) ;								// Sine Wave Stop
	
	if( UcDirSel == X_DIR ) {
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)0x00000000 ) ;	// Set Sine Wave Input RAM
		RamWrite32A( HALL_RAM_SINDX1		,	0x00000000 ) ;				// DelayRam Clear
	}else if( UcDirSel == Y_DIR ){
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)0x00000000 ) ;	// Set Sine Wave Input RAM
		RamWrite32A( HALL_RAM_SINDY1		,	0x00000000 ) ;				// DelayRam Clear
#ifdef	SEL_CLOSED_AF
	}else{
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)0x00000000 ) ;	// Set Sine Wave Input RAM
		RamWrite32A( CLAF_RAMA_AFOUT		,	0x00000000 ) ;				// DelayRam Clear
#endif
	}
	RamRead32A( StMeasFunc_MFA_SiMax1 , ( UINT32 * )&SlMeasureMaxValue ) ;	// Max value
	RamRead32A( StMeasFunc_MFA_SiMin1 , ( UINT32 * )&SlMeasureMinValue ) ;	// Min value

	StTneVal.StDwdVal.UsHigVal = (UINT16)((SlMeasureMaxValue >> 16) & 0x0000FFFF );
	StTneVal.StDwdVal.UsLowVal = (UINT16)((SlMeasureMinValue >> 16) & 0x0000FFFF );
	
	if( UcBfrAft == 0 ) {
		if( UcDirSel == X_DIR ) {
			StAdjPar.StHalAdj.UsHlxCen	= ( ( INT16 )StTneVal.StDwdVal.UsHigVal + ( INT16 )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar.StHalAdj.UsHlxMax	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar.StHalAdj.UsHlxMin	= StTneVal.StDwdVal.UsLowVal ;
		} else if( UcDirSel == Y_DIR ){
			StAdjPar.StHalAdj.UsHlyCen	= ( ( INT16 )StTneVal.StDwdVal.UsHigVal + ( INT16 )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar.StHalAdj.UsHlyMax	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar.StHalAdj.UsHlyMin	= StTneVal.StDwdVal.UsLowVal ;
#ifdef	SEL_CLOSED_AF
		} else {
			StAdjPar.StHalAdj.UsHlzCen	= ( ( INT16 )StTneVal.StDwdVal.UsHigVal + ( INT16 )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar.StHalAdj.UsHlzMax	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar.StHalAdj.UsHlzMin	= StTneVal.StDwdVal.UsLowVal ;
#endif
		}
	} else {
		if( UcDirSel == X_DIR ){
			StAdjPar.StHalAdj.UsHlxCna	= ( ( INT16 )StTneVal.StDwdVal.UsHigVal + ( INT16 )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar.StHalAdj.UsHlxMxa	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar.StHalAdj.UsHlxMna	= StTneVal.StDwdVal.UsLowVal ;
		} else if( UcDirSel == Y_DIR ){
			StAdjPar.StHalAdj.UsHlyCna	= ( ( INT16 )StTneVal.StDwdVal.UsHigVal + ( INT16 )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar.StHalAdj.UsHlyMxa	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar.StHalAdj.UsHlyMna	= StTneVal.StDwdVal.UsLowVal ;
#ifdef	SEL_CLOSED_AF
		} else {
			StAdjPar.StHalAdj.UsHlzCna	= ( ( INT16 )StTneVal.StDwdVal.UsHigVal + ( INT16 )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar.StHalAdj.UsHlzMxa	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar.StHalAdj.UsHlzMna	= StTneVal.StDwdVal.UsLowVal ;
#endif
		}
	}

	StTneVal.StDwdVal.UsHigVal	= 0x7fff - StTneVal.StDwdVal.UsHigVal ;		// Maximum Gap = Maximum - Hall Peak Top
	StTneVal.StDwdVal.UsLowVal	= StTneVal.StDwdVal.UsLowVal - 0x7fff ; 	// Minimum Gap = Hall Peak Bottom - Minimum

	
	return( StTneVal.UlDwdVal ) ;
}

//********************************************************************************
// Function Name 	: TneCen
// Retun Value		: Hall Center Tuning Result
// Argment Value	: X,Y Direction, Hall Top & Bottom Gaps
// Explanation		: Hall Center Tuning Function
// History			: First edition 						
//********************************************************************************
UINT16	UsValBef,UsValNow ;
UINT32	TneCen( UINT8	UcTneAxs, UnDwdVal	StTneVal )
{
	UINT8 	UcTmeOut, UcTofRst ;
	UINT16	UsBiasVal ;
	UINT32	UlTneRst, UlBiasVal , UlValNow ;
	UINT16	UsHalMaxRange , UsHalMinRange ;
	
	UcTmeOut	= 1 ;
	UlTneRst	= FAILURE ;
	UcTofRst	= FAILURE ;

#ifdef	SEL_CLOSED_AF
	if(UcTneAxs == Z_DIR){
		UsHalMaxRange = HALL_MAX_RANGE_Z ;
		UsHalMinRange = HALL_MIN_RANGE_Z ;
	}else{
		UsHalMaxRange = HALL_MAX_RANGE_XY ;
		UsHalMinRange = HALL_MIN_RANGE_XY ;
	}
#else
	UsHalMaxRange = HALL_MAX_RANGE_XY ;
	UsHalMinRange = HALL_MIN_RANGE_XY ;
#endif
	while ( UlTneRst && (UINT32)UcTmeOut )
	{
		if( UcTofRst == FAILURE ) {
			StTneVal.UlDwdVal	= TneOff( StTneVal, UcTneAxs ) ;
		} else {
			StTneVal.UlDwdVal	= TneBia( StTneVal, UcTneAxs ) ;
			UcTofRst	= FAILURE ;
			if( UcTneAxs == X_DIR ) {
				RamRead32A( StCaliData_UiHallBias_X , &UlBiasVal ) ;
			}else if( UcTneAxs == Y_DIR ){
				RamRead32A( StCaliData_UiHallBias_Y , &UlBiasVal ) ;
#ifdef	SEL_CLOSED_AF
			}else{
				RamRead32A( StCaliData_UiHallBias_AF , &UlBiasVal ) ;
#endif
			}
			if(UlBiasVal == 0x00000000){
				UcTmeOut = TIME_OUT;
			}
		}

		if( (StTneVal.StDwdVal.UsHigVal > MARGIN ) && (StTneVal.StDwdVal.UsLowVal > MARGIN ) )	/* position check */
		{
			UcTofRst	= SUCCESS ;
			UsValBef = UsValNow = 0x0000 ;
		}else if( (StTneVal.StDwdVal.UsHigVal <= MARGIN ) && (StTneVal.StDwdVal.UsLowVal <= MARGIN ) ){
			UcTofRst	= SUCCESS ;
			UlTneRst	= (UINT32)FAILURE ;
//		}else if( ((UINT16)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) > BIAS_ADJ_OVER ) {
//			UcTofRst	= SUCCESS ;
//			UlTneRst	= (UINT32)FAILURE ;
		}else{
			UcTofRst	= FAILURE ;
			
			UsValBef = UsValNow ;

			if( UcTneAxs == X_DIR  ) {
				RamRead32A( StCaliData_UiHallOffset_X , &UlValNow ) ;
				UsValNow = (UINT16)( UlValNow >> 16 ) ;
			}else if( UcTneAxs == Y_DIR ){
				RamRead32A( StCaliData_UiHallOffset_Y , &UlValNow ) ;
				UsValNow = (UINT16)( UlValNow >> 16 ) ;
#ifdef	SEL_CLOSED_AF
			}else{
				RamRead32A( StCaliData_UiHallOffset_AF , &UlValNow ) ;
				UsValNow = (UINT16)( UlValNow >> 16 ) ;
#endif
			}
			if( ((( UsValBef & 0xFF00 ) == 0x1000 ) && ( UsValNow & 0xFF00 ) == 0x1000 )
			 || ((( UsValBef & 0xFF00 ) == 0xEF00 ) && ( UsValNow & 0xFF00 ) == 0xEF00 ) )
			{
				if( UcTneAxs == X_DIR ) {
					RamRead32A( StCaliData_UiHallBias_X , &UlBiasVal ) ;
					UsBiasVal = (UINT16)( UlBiasVal >> 16 ) ;
				}else if( UcTneAxs == Y_DIR ){
					RamRead32A( StCaliData_UiHallBias_Y , &UlBiasVal ) ;
					UsBiasVal = (UINT16)( UlBiasVal >> 16 ) ;
#ifdef	SEL_CLOSED_AF
				}else{
					RamRead32A( StCaliData_UiHallBias_AF , &UlBiasVal ) ;
					UsBiasVal = (UINT16)( UlBiasVal >> 16 ) ;
#endif
				}
				
				if( UsBiasVal > DECRE_CAL )
				{
					UsBiasVal -= DECRE_CAL ;
				}
				
				if( UcTneAxs == X_DIR ) {
					UlBiasVal = ( UINT32 )( UsBiasVal << 16 ) ;
					DacControl( 0 , HLXBO , UlBiasVal ) ;
					RamWrite32A( StCaliData_UiHallBias_X , UlBiasVal ) ;
				}else if( UcTneAxs == Y_DIR ){
					UlBiasVal = ( UINT32 )( UsBiasVal << 16 ) ;
					DacControl( 0 , HLYBO , UlBiasVal ) ;
					RamWrite32A( StCaliData_UiHallBias_Y , UlBiasVal ) ;
#ifdef	SEL_CLOSED_AF
				}else{
					UlBiasVal = ( UINT32 )( UsBiasVal << 16 ) ;
					DacControl( 0 , HLAFBO , UlBiasVal ) ;
					RamWrite32A( StCaliData_UiHallBias_AF , UlBiasVal ) ;
#endif
				}
			}

		}
		
		if((( (UINT16)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) < UsHalMaxRange )
		&& (( (UINT16)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) > UsHalMinRange ) ) {
			if(UcTofRst	== SUCCESS)
			{
				UlTneRst	= (UINT32)SUCCESS ;
				break ;
			}
		}
		UlTneRst	= (UINT32)FAILURE ;
		UcTmeOut++ ;

		if ( UcTmeOut >= TIME_OUT ) {
			UcTmeOut	= 0 ;
		}		 																							// Set Time Out Count
	}

	SetSinWavGenInt() ;		// 
	
	if( UlTneRst == (UINT32)FAILURE ) {
		if( UcTneAxs == X_DIR ) {
			UlTneRst					= EXE_HXADJ ;
			StAdjPar.StHalAdj.UsHlxGan	= 0xFFFF ;
			StAdjPar.StHalAdj.UsHlxOff	= 0xFFFF ;
		}else if( UcTneAxs == Y_DIR ) {
			UlTneRst					= EXE_HYADJ ;
			StAdjPar.StHalAdj.UsHlyGan	= 0xFFFF ;
			StAdjPar.StHalAdj.UsHlyOff	= 0xFFFF ;
#ifdef	SEL_CLOSED_AF
		} else {
			UlTneRst					= EXE_HZADJ ;
			StAdjPar.StHalAdj.UsHlzGan	= 0xFFFF ;
			StAdjPar.StHalAdj.UsHlzOff	= 0xFFFF ;
#endif
		}
	} else {
		UlTneRst	= EXE_END ;
	}

	return( UlTneRst ) ;
}



//********************************************************************************
// Function Name 	: TneBia
// Retun Value		: Hall Top & Bottom Gaps
// Argment Value	: Hall Top & Bottom Gaps , X,Y Direction
// Explanation		: Hall Bias Tuning Function
// History			: First edition 						
//********************************************************************************
UINT32	TneBia( UnDwdVal	StTneVal, UINT8	UcTneAxs )
{
	UINT32			UlSetBia , UlSetBia_Bk;
	UINT16			UsHalAdjRange;
#ifdef	SEL_CLOSED_AF
	if(UcTneAxs == Z_DIR){
		UsHalAdjRange = BIAS_ADJ_RANGE_Z ;
	}else{
		UsHalAdjRange = BIAS_ADJ_RANGE_XY ;
	}
#else
		UsHalAdjRange = BIAS_ADJ_RANGE_XY ;
#endif

	if( UcTneAxs == X_DIR ) {
		RamRead32A( StCaliData_UiHallBias_X , &UlSetBia ) ;		
	} else if( UcTneAxs == Y_DIR ) {
		RamRead32A( StCaliData_UiHallBias_Y , &UlSetBia ) ;		
#ifdef	SEL_CLOSED_AF
	} else {
		RamRead32A( StCaliData_UiHallBias_AF , &UlSetBia ) ;		
#endif
	}
	UlSetBia_Bk = UlSetBia ;	// backup

	if( UlSetBia == 0x00000000 )	UlSetBia = 0x01000000 ;
	UlSetBia = (( UlSetBia >> 16 ) & (UINT32)0x0000FF00 ) ;
	UlSetBia *= (UINT32)UsHalAdjRange ;
	UlSetBia /= (UINT32)( 0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) ;
	if( UlSetBia > (UINT32)0x0000FFFF )		UlSetBia = 0x0000FFFF ;
	UlSetBia = ( UlSetBia << 16 ) ;
	if( UlSetBia > BIAS_HLMT )		UlSetBia = BIAS_HLMT ;
	if( UlSetBia < BIAS_LLMT )		UlSetBia = BIAS_LLMT ;
	
	if(( (UlSetBia_Bk == BIAS_LLMT) && (UlSetBia == BIAS_LLMT) ) || ( UlSetBia_Bk == BIAS_HLMT && UlSetBia == BIAS_HLMT )){
		UlSetBia = 0x00000000 ;		// note :  0x20000000 =< Bia =< 0xBF000000
	}

	if( UcTneAxs == X_DIR ) {
		DacControl( 0 , HLXBO , UlSetBia ) ;
		RamWrite32A( StCaliData_UiHallBias_X , UlSetBia) ;
	} else if( UcTneAxs == Y_DIR ){
		DacControl( 0 , HLYBO , UlSetBia ) ;
		RamWrite32A( StCaliData_UiHallBias_Y , UlSetBia) ;
#ifdef	SEL_CLOSED_AF
	} else {
		DacControl( 0 , HLAFBO , UlSetBia ) ;
		RamWrite32A( StCaliData_UiHallBias_AF , UlSetBia) ;
#endif
	}
	
	StTneVal.UlDwdVal	= TnePtp( UcTneAxs , PTP_AFTER ) ;

	return( StTneVal.UlDwdVal ) ;
}



//********************************************************************************
// Function Name 	: TneOff
// Retun Value		: Hall Top & Bottom Gaps
// Argment Value	: Hall Top & Bottom Gaps , X,Y Direction
// Explanation		: Hall Offset Tuning Function
// History			: First edition 						
//********************************************************************************
UINT32	TneOff( UnDwdVal	StTneVal, UINT8	UcTneAxs )
{
	UINT32	UlSetOff ;
	UINT32	UlSetVal ;

	
	if( UcTneAxs == X_DIR ) {
		RamRead32A( StCaliData_UiHallOffset_X , &UlSetOff ) ;
	} else if( UcTneAxs == Y_DIR ){
		RamRead32A( StCaliData_UiHallOffset_Y , &UlSetOff ) ;
#ifdef	SEL_CLOSED_AF
	} else {
		RamRead32A( StCaliData_UiHallOffset_AF , &UlSetOff ) ;
#endif
	}
	UlSetOff 	= ( UlSetOff >> 16 ) ;

	if ( StTneVal.StDwdVal.UsHigVal > StTneVal.StDwdVal.UsLowVal ) {
		UlSetVal	= ( UINT32 )(( StTneVal.StDwdVal.UsHigVal - StTneVal.StDwdVal.UsLowVal ) / OFFSET_DIV ) ;	// Calculating Value For Increase Step
		UlSetOff	+= UlSetVal ;	// Calculating Value For Increase Step
		if( UlSetOff > 0x0000FFFF )		UlSetOff = 0x0000FFFF ;
	} else {
		UlSetVal	= ( UINT32 )(( StTneVal.StDwdVal.UsLowVal - StTneVal.StDwdVal.UsHigVal ) / OFFSET_DIV ) ;	// Calculating Value For Decrease Step
		if( UlSetOff < UlSetVal ){
			UlSetOff	= 0x00000000 ;
		}else{
			UlSetOff	-= UlSetVal ;	// Calculating Value For Decrease Step
		}
	}

	if( UlSetOff > ( INT32 )0x0000EFFF ) {
		UlSetOff	= 0x0000EFFF ;
	} else if( UlSetOff < ( INT32 )0x00001000 ) {
		UlSetOff	= 0x00001000 ;
	}

	UlSetOff = ( UlSetOff << 16 ) ;
	
	if( UcTneAxs == X_DIR ) {
		DacControl( 0 , HLXO, UlSetOff ) ;
		RamWrite32A( StCaliData_UiHallOffset_X , UlSetOff ) ;
	} else if( UcTneAxs == Y_DIR ){
		DacControl( 0 , HLYO, UlSetOff ) ;
		RamWrite32A( StCaliData_UiHallOffset_Y , UlSetOff ) ;
#ifdef	SEL_CLOSED_AF
	} else {
		DacControl( 0 , HLAFO, UlSetOff ) ;
		RamWrite32A( StCaliData_UiHallOffset_AF , UlSetOff ) ;
#endif
	}

	StTneVal.UlDwdVal	= TnePtp( UcTneAxs, PTP_AFTER ) ;

	return( StTneVal.UlDwdVal ) ;
}


//********************************************************************************
// Function Name 	: MesFil
// Retun Value		: NON
// Argment Value	: Measure Filter Mode
// Explanation		: Measure Filter Setting Function
// History			: First edition 		
//********************************************************************************
void	MesFil( UINT8	UcMesMod )		// 20.019kHz
{
	UINT32	UlMeasFilaA , UlMeasFilaB , UlMeasFilaC ;
	UINT32	UlMeasFilbA , UlMeasFilbB , UlMeasFilbC ;

	if( !UcMesMod ) {								// Hall Bias&Offset Adjust
		
		UlMeasFilaA	=	0x02F19B01 ;	// LPF 150Hz
		UlMeasFilaB	=	0x02F19B01 ;
		UlMeasFilaC	=	0x7A1CC9FF ;
		UlMeasFilbA	=	0x7FFFFFFF ;	// Through
		UlMeasFilbB	=	0x00000000 ;
		UlMeasFilbC	=	0x00000000 ;
		
	} else if( UcMesMod == LOOPGAIN ) {				// Loop Gain Adjust

		UlMeasFilaA	=	0x115CC757 ;	// LPF1000Hz
		UlMeasFilaB	=	0x115CC757 ;
		UlMeasFilaC	=	0x5D467153 ;
		UlMeasFilbA	=	0x7F667431 ;	// HPF30Hz
		UlMeasFilbB	=	0x80998BCF ;
		UlMeasFilbC	=	0x7ECCE863 ;
		
	} else if( UcMesMod == THROUGH ) {				// for Through

		UlMeasFilaA	=	0x7FFFFFFF ;	// Through
		UlMeasFilaB	=	0x00000000 ;
		UlMeasFilaC	=	0x00000000 ;
		UlMeasFilbA	=	0x7FFFFFFF ;	// Through
		UlMeasFilbB	=	0x00000000 ;
		UlMeasFilbC	=	0x00000000 ;

	} else if( UcMesMod == NOISE ) {				// SINE WAVE TEST for NOISE

		UlMeasFilaA	=	0x02F19B01 ;	// LPF150Hz
		UlMeasFilaB	=	0x02F19B01 ;
		UlMeasFilaC	=	0x7A1CC9FF ;
		UlMeasFilbA	=	0x02F19B01 ;	// LPF150Hz
		UlMeasFilbB	=	0x02F19B01 ;
		UlMeasFilbC	=	0x7A1CC9FF ;

	} else if(UcMesMod == OSCCHK) {
		UlMeasFilaA	=	0x05C141BB ;	// LPF300Hz
		UlMeasFilaB	=	0x05C141BB ;
		UlMeasFilaC	=	0x747D7C88 ;
		UlMeasFilbA	=	0x05C141BB ;	// LPF300Hz
		UlMeasFilbB	=	0x05C141BB ;
		UlMeasFilbC	=	0x747D7C88 ;
	}
	
	RamWrite32A ( MeasureFilterA_Coeff_a1	, UlMeasFilaA ) ;
	RamWrite32A ( MeasureFilterA_Coeff_b1	, UlMeasFilaB ) ;
	RamWrite32A ( MeasureFilterA_Coeff_c1	, UlMeasFilaC ) ;

	RamWrite32A ( MeasureFilterA_Coeff_a2	, UlMeasFilbA ) ;
	RamWrite32A ( MeasureFilterA_Coeff_b2	, UlMeasFilbB ) ;
	RamWrite32A ( MeasureFilterA_Coeff_c2	, UlMeasFilbC ) ;

	RamWrite32A ( MeasureFilterB_Coeff_a1	, UlMeasFilaA ) ;
	RamWrite32A ( MeasureFilterB_Coeff_b1	, UlMeasFilaB ) ;
	RamWrite32A ( MeasureFilterB_Coeff_c1	, UlMeasFilaC ) ;

	RamWrite32A ( MeasureFilterB_Coeff_a2	, UlMeasFilbA ) ;
	RamWrite32A ( MeasureFilterB_Coeff_b2	, UlMeasFilbB ) ;
	RamWrite32A ( MeasureFilterB_Coeff_c2	, UlMeasFilbC ) ;
}

//********************************************************************************
// Function Name 	: ClrMesFil
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Clear Measure Filter Function
// History			: First edition 						
//********************************************************************************
void	ClrMesFil( void )
{
	RamWrite32A ( MeasureFilterA_Delay_z11	, 0 ) ;
	RamWrite32A ( MeasureFilterA_Delay_z12	, 0 ) ;

	RamWrite32A ( MeasureFilterA_Delay_z21	, 0 ) ;
	RamWrite32A ( MeasureFilterA_Delay_z22	, 0 ) ;

	RamWrite32A ( MeasureFilterB_Delay_z11	, 0 ) ;
	RamWrite32A ( MeasureFilterB_Delay_z12	, 0 ) ;

	RamWrite32A ( MeasureFilterB_Delay_z21	, 0 ) ;
	RamWrite32A ( MeasureFilterB_Delay_z22	, 0 ) ;
}

//********************************************************************************
// Function Name 	: MeasureStart
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Measure start setting Function
// History			: First edition 						
//********************************************************************************
void	MeasureStart( INT32 SlMeasureParameterNum , INT32 SlMeasureParameterA , INT32 SlMeasureParameterB )
{
	MemoryClear( StMeasFunc_SiSampleNum , sizeof( MeasureFunction_Type ) ) ;
	RamWrite32A( StMeasFunc_MFA_SiMax1	 , 0x80000000 ) ;					// Set Min 
	RamWrite32A( StMeasFunc_MFB_SiMax2	 , 0x80000000 ) ;					// Set Min 
	RamWrite32A( StMeasFunc_MFA_SiMin1	 , 0x7FFFFFFF ) ;					// Set Max 
	RamWrite32A( StMeasFunc_MFB_SiMin2	 , 0x7FFFFFFF ) ;					// Set Max 
	
	SetTransDataAdr( StMeasFunc_MFA_PiMeasureRam1	 , ( UINT32 )SlMeasureParameterA ) ;		// Set Measure Filter A Ram Address
	SetTransDataAdr( StMeasFunc_MFB_PiMeasureRam2	 , ( UINT32 )SlMeasureParameterB ) ;		// Set Measure Filter B Ram Address
	RamWrite32A( StMeasFunc_SiSampleNum	 , 0 ) ;													// Clear Measure Counter 
	ClrMesFil() ;						// Clear Delay Ram
//	SetWaitTime(50) ;
	SetWaitTime(1) ;
	RamWrite32A( StMeasFunc_SiSampleMax	 , SlMeasureParameterNum ) ;						// Set Measure Max Number

}
	
//********************************************************************************
// Function Name 	: MeasureWait
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Wait complete of Measure Function
// History			: First edition 						
//********************************************************************************
void	MeasureWait( void )
{
	UINT32			SlWaitTimerSt ;
	
	SlWaitTimerSt = 1 ;
	while( SlWaitTimerSt ){
		RamRead32A( StMeasFunc_SiSampleMax , &SlWaitTimerSt ) ;
	}
}
	
//********************************************************************************
// Function Name 	: MemoryClear
// Retun Value		: NON
// Argment Value	: Top pointer , Size
// Explanation		: Memory Clear Function
// History			: First edition 						
//********************************************************************************
void	MemoryClear( UINT16 UsSourceAddress, UINT16 UsClearSize )
{
	UINT16	UsLoopIndex ;

	for ( UsLoopIndex = 0 ; UsLoopIndex < UsClearSize ;  ) {
		RamWrite32A( UsSourceAddress	, 	0x00000000 ) ;				// 4Byte
		UsSourceAddress += 4;
		UsLoopIndex += 4 ;
	}
}
	
//********************************************************************************
// Function Name 	: SetWaitTime
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Set Timer wait Function
// History			: First edition 						
//********************************************************************************
#define 	ONE_MSEC_COUNT	20			// 20.0195kHz * 20 ≒ 1ms
void	SetWaitTime( UINT16 UsWaitTime )
{
	RamWrite32A( WaitTimerData_UiWaitCounter	, 0 ) ;
	RamWrite32A( WaitTimerData_UiTargetCount	, (UINT32)(ONE_MSEC_COUNT * UsWaitTime)) ;
}


//********************************************************************************
// Function Name 	: LopGan
// Retun Value		: Execute Result
// Argment Value	: X,Y Direction
// Explanation		: Loop Gain Adjust Function
// History			: First edition 						
//********************************************************************************
#define 	LOOP_NUM		2136			// 20.019kHz/0.150kHz*16times
#define 	LOOP_FREQ		0x00F586D9		// 	150Hz  = Freq * 80000000h / Fs
//#define 	LOOP_NUM		2002			// 20.019kHz/0.160kHz*16times
//#define 	LOOP_FREQ		0x0105E52C		// 	160Hz  = Freq * 80000000h / Fs
//#define 	LOOP_GAIN		0x0CCCCCCD		// -20dB
#define 	LOOP_GAIN		0x040C3713		// -30dB
//#define 	LOOP_GAIN		0x0207567A		// -36dB

#define 	LOOP_MAX_X		SXGAIN_LOP << 1	// x2
#define 	LOOP_MIN_X		SXGAIN_LOP >> 1	// x0.5
#define 	LOOP_MAX_Y		SYGAIN_LOP << 1	// x2
#define 	LOOP_MIN_Y		SYGAIN_LOP >> 1	// x0.5

#ifdef	SEL_CLOSED_AF
#define 	LOOP_NUM_Z		1885			// 20.019kHz/0.170kHz*16times
#define 	LOOP_FREQ_Z		0x0116437F		// 	170Hz  = Freq * 80000000h / Fs
#define 	LOOP_GAIN_Z		0x0207567A		// -36dB
#define 	LOOP_MAX_Z		SZGAIN_LOP << 1	// x2
#define 	LOOP_MIN_Z		SZGAIN_LOP >> 1	// x0.5
#endif

UINT32	LopGan( UINT8	UcDirSel )
{
	UnllnVal		StMeasValueA , StMeasValueB ;
	INT32			SlMeasureParameterA , SlMeasureParameterB ;
	INT32			SlMeasureParameterNum ;
	UINT64	UllCalculateVal ;
	UINT32	UlReturnState ;
	UINT16	UsSinAdr ;
	UINT32	UlLopFreq , UlLopGain;
#ifdef	SEL_CLOSED_AF
	UINT32	UlSwitchBk ;
#endif
	
	SlMeasureParameterNum	=	(INT32)LOOP_NUM ;
	
	if( UcDirSel == X_DIR ) {		// X axis
		SlMeasureParameterA		=	HALL_RAM_HXOUT1 ;		// Set Measure RAM Address
		SlMeasureParameterB		=	HALL_RAM_HXLOP ;		// Set Measure RAM Address
		RamWrite32A( HallFilterCoeffX_hxgain0 , SXGAIN_LOP ) ;
		UsSinAdr = HALL_RAM_SINDX0;
		UlLopFreq = LOOP_FREQ;
		UlLopGain = LOOP_GAIN;
	} else if( UcDirSel == Y_DIR ){						// Y axis
		SlMeasureParameterA		=	HALL_RAM_HYOUT1 ;		// Set Measure RAM Address
		SlMeasureParameterB		=	HALL_RAM_HYLOP ;		// Set Measure RAM Address
		RamWrite32A( HallFilterCoeffY_hygain0 , SYGAIN_LOP ) ;
		UsSinAdr = HALL_RAM_SINDY0;
		UlLopFreq = LOOP_FREQ;
		UlLopGain = LOOP_GAIN;
#ifdef	SEL_CLOSED_AF
	} else {						// Y axis
		SlMeasureParameterNum	=	(INT32)LOOP_NUM_Z ;
		SlMeasureParameterA		=	CLAF_RAMA_AFLOP2 ;		// Set Measure RAM Address
		SlMeasureParameterB		=	CLAF_DELAY_AFPZ0 ;		// Set Measure RAM Address
		RamWrite32A( CLAF_Gain_afloop2 , SZGAIN_LOP ) ;
		RamRead32A( CLAF_RAMA_AFCNT , &UlSwitchBk ) ;
//		RamWrite32A( CLAF_RAMA_AFCNT , UlSwitchBk & 0xffffffef ) ;
		RamWrite32A( CLAF_RAMA_AFCNT , UlSwitchBk & 0xffffff0f ) ;
		UsSinAdr = CLAF_RAMA_AFCNTO;
		UlLopFreq = LOOP_FREQ_Z;
		UlLopGain = LOOP_GAIN_Z;
#endif
	}
	
	SetSinWavGenInt();

	RamWrite32A( SinWave_Offset		,	UlLopFreq ) ;								// Freq Setting
	RamWrite32A( SinWave_Gain		,	UlLopGain ) ;								// Set Sine Wave Gain					
	
	RamWrite32A( SinWaveC_Regsiter	,	0x00000001 ) ;								// Sine Wave Start

	SetTransDataAdr( SinWave_OutAddr	,	( UINT32 )UsSinAdr ) ;	// Set Sine Wave Input RAM
	
	MesFil( LOOPGAIN ) ;					// Filter setting for measurement
	
	MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;					// Start measure
	
	MeasureWait() ;						// Wait complete of measurement

	RamRead32A( StMeasFunc_MFA_LLiAbsInteg1 		, &StMeasValueA.StUllnVal.UlLowVal ) ;	// X axis
	RamRead32A( StMeasFunc_MFA_LLiAbsInteg1 + 4 	, &StMeasValueA.StUllnVal.UlHigVal ) ;
	RamRead32A( StMeasFunc_MFB_LLiAbsInteg2 		, &StMeasValueB.StUllnVal.UlLowVal ) ;	// Y axis
	RamRead32A( StMeasFunc_MFB_LLiAbsInteg2 + 4		, &StMeasValueB.StUllnVal.UlHigVal ) ;
	
	SetSinWavGenInt();		// Sine wave stop
	
	SetTransDataAdr( SinWave_OutAddr	,	(UINT32)0x00000000 ) ;	// Set Sine Wave Input RAM
	RamWrite32A( UsSinAdr		,	0x00000000 ) ;				// DelayRam Clear
	
	if( UcDirSel == X_DIR ) {		// X axis
		UllCalculateVal = ( StMeasValueB.UllnValue * 1000 / StMeasValueA.UllnValue ) * SXGAIN_LOP / 1000 ;
		if( UllCalculateVal > (UINT64)0x000000007FFFFFFF )		UllCalculateVal = (UINT64)0x000000007FFFFFFF ;
		StAdjPar.StLopGan.UlLxgVal = (UINT32)UllCalculateVal ;
		RamWrite32A( HallFilterCoeffX_hxgain0 , StAdjPar.StLopGan.UlLxgVal ) ;
		if( UllCalculateVal > LOOP_MAX_X ){
			UlReturnState = EXE_LXADJ ;
		}else if( UllCalculateVal < LOOP_MIN_X ){
			UlReturnState = EXE_LXADJ ;
		}else{
			UlReturnState = EXE_END ;
		}
		
	}else if( UcDirSel == Y_DIR ){							// Y axis
		UllCalculateVal = ( StMeasValueB.UllnValue * 1000 / StMeasValueA.UllnValue ) * SYGAIN_LOP / 1000 ;
		if( UllCalculateVal > (UINT64)0x000000007FFFFFFF )		UllCalculateVal = (UINT64)0x000000007FFFFFFF ;
		StAdjPar.StLopGan.UlLygVal = (UINT32)UllCalculateVal ;
		RamWrite32A( HallFilterCoeffY_hygain0 , StAdjPar.StLopGan.UlLygVal ) ;
		if( UllCalculateVal > LOOP_MAX_Y ){
			UlReturnState = EXE_LYADJ ;
		}else if( UllCalculateVal < LOOP_MIN_Y ){
			UlReturnState = EXE_LYADJ ;
		}else{
			UlReturnState = EXE_END ;
		}
#ifdef	SEL_CLOSED_AF
	}else{							// Z axis
		UllCalculateVal = ( StMeasValueB.UllnValue * 1000 / StMeasValueA.UllnValue ) * SZGAIN_LOP / 1000 ;
		if( UllCalculateVal > (UINT64)0x000000007FFFFFFF )		UllCalculateVal = (UINT64)0x000000007FFFFFFF ;
		StAdjPar.StLopGan.UlLzgVal = (UINT32)UllCalculateVal ;
		RamWrite32A( CLAF_Gain_afloop2 , StAdjPar.StLopGan.UlLzgVal ) ;
		if( UllCalculateVal > LOOP_MAX_Z ){
			UlReturnState = EXE_LZADJ ;
		}else if( UllCalculateVal < LOOP_MIN_Z ){
			UlReturnState = EXE_LZADJ ;
		}else{
			UlReturnState = EXE_END ;
		}
		RamWrite32A( CLAF_RAMA_AFCNT , UlSwitchBk ) ;
#endif
	}
	
	return( UlReturnState ) ;

}




//********************************************************************************
// Function Name 	: TneGvc
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Tunes the Gyro VC offset
// History			: First edition 						
//********************************************************************************
#define 	GYROF_NUM		2048			// 2048times
#define 	GYROF_UPPER		0x0600			// 
#define 	GYROF_LOWER		0xFA00			// 
UINT32	TneGvc( void )
{
	UINT32	UlRsltSts;
	INT32			SlMeasureParameterA , SlMeasureParameterB ;
	INT32			SlMeasureParameterNum ;
	UnllnVal		StMeasValueA , StMeasValueB ;
	INT32			SlMeasureAveValueA , SlMeasureAveValueB ;
	
	
	//平均値測定
	
	MesFil( THROUGH ) ;					// Set Measure Filter

	SlMeasureParameterNum	=	GYROF_NUM ;					// Measurement times
	SlMeasureParameterA		=	GYRO_RAM_GX_ADIDAT ;		// Set Measure RAM Address
	SlMeasureParameterB		=	GYRO_RAM_GY_ADIDAT ;		// Set Measure RAM Address
	
	MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;					// Start measure
	
//	ClrMesFil();					// Clear Delay RAM
//	SetWaitTime(50) ;
//	SetWaitTime(1) ;
	
	MeasureWait() ;					// Wait complete of measurement
	
	RamRead32A( StMeasFunc_MFA_LLiIntegral1 		, &StMeasValueA.StUllnVal.UlLowVal ) ;	// X axis
	RamRead32A( StMeasFunc_MFA_LLiIntegral1 + 4		, &StMeasValueA.StUllnVal.UlHigVal ) ;
	RamRead32A( StMeasFunc_MFB_LLiIntegral2 		, &StMeasValueB.StUllnVal.UlLowVal ) ;	// Y axis
	RamRead32A( StMeasFunc_MFB_LLiIntegral2 + 4		, &StMeasValueB.StUllnVal.UlHigVal ) ;
	
	SlMeasureAveValueA = (INT32)( (INT64)StMeasValueA.UllnValue / SlMeasureParameterNum ) ;
	SlMeasureAveValueB = (INT32)( (INT64)StMeasValueB.UllnValue / SlMeasureParameterNum ) ;
	
	SlMeasureAveValueA = ( SlMeasureAveValueA >> 16 ) & 0x0000FFFF ;
	SlMeasureAveValueB = ( SlMeasureAveValueB >> 16 ) & 0x0000FFFF ;
	
	SlMeasureAveValueA = 0x00010000 - SlMeasureAveValueA ;
	SlMeasureAveValueB = 0x00010000 - SlMeasureAveValueB ;
	
	UlRsltSts = EXE_END ;
	StAdjPar.StGvcOff.UsGxoVal = ( UINT16 )( SlMeasureAveValueA & 0x0000FFFF );		//Measure Result Store
	if(( (INT16)StAdjPar.StGvcOff.UsGxoVal > (INT16)GYROF_UPPER ) || ( (INT16)StAdjPar.StGvcOff.UsGxoVal < (INT16)GYROF_LOWER )){
		UlRsltSts |= EXE_GXADJ ;
	}
	RamWrite32A( GYRO_RAM_GXOFFZ , (( SlMeasureAveValueA << 16 ) & 0xFFFF0000 ) ) ;		// X axis Gyro offset
	
	StAdjPar.StGvcOff.UsGyoVal = ( UINT16 )( SlMeasureAveValueB & 0x0000FFFF );		//Measure Result Store
	if(( (INT16)StAdjPar.StGvcOff.UsGyoVal > (INT16)GYROF_UPPER ) || ( (INT16)StAdjPar.StGvcOff.UsGyoVal < (INT16)GYROF_LOWER )){
		UlRsltSts |= EXE_GYADJ ;
	}
	RamWrite32A( GYRO_RAM_GYOFFZ , (( SlMeasureAveValueB << 16 ) & 0xFFFF0000 ) ) ;		// Y axis Gyro offset
	
	
	RamWrite32A( GYRO_RAM_GYROX_OFFSET , 0x00000000 ) ;			// X axis Drift Gyro offset
	RamWrite32A( GYRO_RAM_GYROY_OFFSET , 0x00000000 ) ;			// Y axis Drift Gyro offset
	RamWrite32A( GyroFilterDelayX_GXH1Z2 , 0x00000000 ) ;		// X axis H1Z2 Clear
	RamWrite32A( GyroFilterDelayY_GYH1Z2 , 0x00000000 ) ;		// Y axis H1Z2 Clear
	
	return( UlRsltSts );
	
		
}



//********************************************************************************
// Function Name 	: RtnCen
// Retun Value		: Command Status
// Argment Value	: Command Parameter
// Explanation		: Return to center Command Function
// History			: First edition 						
//********************************************************************************
UINT8	RtnCen( UINT8	UcCmdPar )
{
	UINT8	UcSndDat = 1 ;
	
	if( !UcCmdPar ){								// X,Y centering
		RamWrite32A( CMD_RETURN_TO_CENTER , BOTH_SRV_ON ) ;
	}else if( UcCmdPar == XONLY_ON ){				// only X centering
		RamWrite32A( CMD_RETURN_TO_CENTER , XAXS_SRV_ON ) ;
	}else if( UcCmdPar == YONLY_ON ){				// only Y centering
		RamWrite32A( CMD_RETURN_TO_CENTER , YAXS_SRV_ON ) ;
#ifdef	SEL_CLOSED_AF
	}else if( UcCmdPar == ZONLY_OFF ){				// only Z centering off
		RamWrite32A( CMD_RETURN_TO_CENTER , ZAXS_SRV_OFF ) ;
	}else if( UcCmdPar == ZONLY_ON ){				// only Z centering
		RamWrite32A( CMD_RETURN_TO_CENTER , ZAXS_SRV_ON ) ;
#endif
	}else{											// Both off
		RamWrite32A( CMD_RETURN_TO_CENTER , BOTH_SRV_OFF ) ;
	}
	
	while( UcSndDat ) {
		UcSndDat = RdStatus(1);
	}
#ifdef	SEL_CLOSED_AF
	if( UcCmdPar == ZONLY_OFF ){				// only Z centering off
		RamWrite32A( CLAF_RAMA_AFOUT		,	0x00000000 ) ;				// DelayRam Clear
	}
#endif
	
	return( UcSndDat );
}



//********************************************************************************
// Function Name 	: OisEna
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: OIS Enable Control Function
// History			: First edition 						
//********************************************************************************
void	OisEna( void )
{
	UINT8	UcStRd = 1;
	
	RamWrite32A( CMD_OIS_ENABLE , OIS_ENABLE ) ;
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}
}

//********************************************************************************
// Function Name 	: OisEnaNCL
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: OIS Enable Control Function w/o delay clear
// History			: First edition 						
//********************************************************************************
void	OisEnaNCL( void )
{
	UINT8	UcStRd = 1;
	
	RamWrite32A( CMD_OIS_ENABLE , OIS_ENA_NCL | OIS_ENABLE ) ;
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}
}

//********************************************************************************
// Function Name 	: OisEnaDrCl
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: OIS Enable Control Function w/o delay clear
// History			: First edition 						
//********************************************************************************
void	OisEnaDrCl( void )
{
	UINT8	UcStRd = 1;
	
	RamWrite32A( CMD_OIS_ENABLE , OIS_ENA_DOF | OIS_ENABLE ) ;
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}
}

//********************************************************************************
// Function Name 	: OisEnaDrNcl
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: OIS Enable Control Function w/o delay clear
// History			: First edition 						
//********************************************************************************
void	OisEnaDrNcl( void )
{
	UINT8	UcStRd = 1;
	
	RamWrite32A( CMD_OIS_ENABLE , OIS_ENA_DOF | OIS_ENA_NCL | OIS_ENABLE ) ;
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}
}
//********************************************************************************
// Function Name 	: OisDis
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: OIS Disable Control Function
// History			: First edition 						
//********************************************************************************
void	OisDis( void )
{
	UINT8	UcStRd = 1;
	
	RamWrite32A( CMD_OIS_ENABLE , OIS_DISABLE ) ;
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}
}


//********************************************************************************
// Function Name 	: SetRec
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: Rec Mode Enable Function
// History			: First edition 						
//********************************************************************************
void	SetRec( void )
{
	UINT8	UcStRd = 1;
	
	RamWrite32A( CMD_MOVE_STILL_MODE ,	MOVIE_MODE ) ;
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}
}


//********************************************************************************
// Function Name 	: SetStill
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: Set Still Mode Enable Function
// History			: First edition 						
//********************************************************************************
void	SetStill( void )
{
	UINT8	UcStRd = 1;
	
	RamWrite32A( CMD_MOVE_STILL_MODE ,	STILL_MODE ) ;
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}
}


//********************************************************************************
// Function Name 	: SetSinWavePara
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Sine wave Test Function
// History			: First edition 						
//********************************************************************************
	/********* Parameter Setting *********/
	/* Servo Sampling Clock		=	20.0195kHz						*/
	/* Freq						=	SinFreq*80000000h/Fs			*/
	/* 05 00 XX MM 				XX:Freq MM:Sin or Circle */
const UINT32	CucFreqVal[ 17 ]	= {
		0xFFFFFFFF,				//  0:  Stop
		0x0001A306,				//  1: 1Hz
		0x0003460B,				//  2: 2Hz
		0x0004E911,				//  3: 3Hz	
		0x00068C16,				//  4: 4Hz
		0x00082F1C,				//  5: 5Hz
		0x0009D222,				//  6: 6Hz
		0x000B7527,				//  7: 7Hz
		0x000D182D,				//  8: 8Hz
		0x000EBB32,				//  9: 9Hz
		0x00105E38,				//  A: 10Hz
		0x0012013E,				//  B: 11Hz
		0x0013A443,				//  C: 12Hz
		0x00154749,				//  D: 13Hz
		0x0016EA4E,				//  E: 14Hz
		0x00188D54,				//  F: 15Hz
		0x001A305A				// 10: 16Hz
	} ;
	
// 	RamWrite32A( SinWave.Gain , 0x00000000 ) ;			// Gainはそれぞれ設定すること
// 	RamWrite32A( CosWave.Gain , 0x00000000 ) ;			// Gainはそれぞれ設定すること
void	SetSinWavePara( UINT8 UcTableVal ,  UINT8 UcMethodVal )
{
	UINT32	UlFreqDat ;

	
	if(UcTableVal > 0x10 )
		UcTableVal = 0x10 ;			/* Limit */
	UlFreqDat = CucFreqVal[ UcTableVal ] ;	
	
	if( UcMethodVal == CIRCWAVE) {
		RamWrite32A( SinWave_Phase	,	0x00000000 ) ;		// 正弦波の位相量
		RamWrite32A( CosWave_Phase 	,	0x20000000 );		// 正弦波の位相量
	}else{
		RamWrite32A( SinWave_Phase	,	0x00000000 ) ;		// 正弦波の位相量
		RamWrite32A( CosWave_Phase 	,	0x00000000 );		// 正弦波の位相量
	}


	if( UlFreqDat == 0xFFFFFFFF )			/* Sine波中止 */
	{
		RamWrite32A( SinWave_Offset		,	0x00000000 ) ;									// 発生周波数のオフセットを設定
		RamWrite32A( SinWave_Phase		,	0x00000000 ) ;									// 正弦波の位相量
//		RamWrite32A( SinWave_Gain		,	0x00000000 ) ;									// 発生周波数のアッテネータ(初期値は0[dB])
//		SetTransDataAdr( SinWave_OutAddr	,	 (UINT32)SinWave_Output );			// 出力先アドレス

		RamWrite32A( CosWave_Offset		,	0x00000000 );									// 発生周波数のオフセットを設定
		RamWrite32A( CosWave_Phase 		,	0x00000000 );									// 正弦波の位相量
//		RamWrite32A( CosWave_Gain 		,	0x00000000 );									// 発生周波数のアッテネータ(初期値はCut)
//		SetTransDataAdr( CosWave_OutAddr	,	 (UINT32)CosWave_Output );			// 出力先アドレス

		RamWrite32A( SinWaveC_Regsiter	,	0x00000000 ) ;									// Sine Wave Stop
		SetTransDataAdr( SinWave_OutAddr	,	0x00000000 ) ;		// 出力先アドレス
		SetTransDataAdr( CosWave_OutAddr	,	0x00000000 );		// 出力先アドレス
		RamWrite32A( HALL_RAM_HXOFF1		,	0x00000000 ) ;				// DelayRam Clear
		RamWrite32A( HALL_RAM_HYOFF1		,	0x00000000 ) ;				// DelayRam Clear
	}
	else
	{
		RamWrite32A( SinWave_Offset		,	UlFreqDat ) ;									// 発生周波数のオフセットを設定
		RamWrite32A( CosWave_Offset		,	UlFreqDat );									// 発生周波数のオフセットを設定

		RamWrite32A( SinWaveC_Regsiter	,	0x00000001 ) ;									// Sine Wave Start
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)HALL_RAM_HXOFF1 ) ;		// 出力先アドレス
		SetTransDataAdr( CosWave_OutAddr	,	(UINT32)HALL_RAM_HYOFF1 ) ;		// 出力先アドレス

	}
	
	
}




//********************************************************************************
// Function Name 	: SetPanTiltMode
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Pan-Tilt Enable/Disable
// History			: First edition 						
//********************************************************************************
void	SetPanTiltMode( UINT8 UcPnTmod )
{
	UINT8	UcStRd = 1;
	
	switch ( UcPnTmod ) {
		case OFF :
			RamWrite32A( CMD_PAN_TILT ,	PAN_TILT_OFF ) ;
			break ;
		case ON :
			RamWrite32A( CMD_PAN_TILT ,	PAN_TILT_ON ) ;
			break ;
	}
	while( UcStRd ) {
		UcStRd = RdStatus(1);
	}

}


//********************************************************************************
// Function Name 	: DrvPwmSw
// Retun Value		: Mode Status
//					: bit4( 1:PWM / 0:LinearPwm)
// Argment Value	: NON
// Explanation		: Select Driver mode Function
// History			: First edition 					
//********************************************************************************
UINT8	DrvPwmSw( UINT8 UcSelPwmMod )
{

	switch ( UcSelPwmMod ) {
		case Mlnp :
 			break ;
		case Mpwm :
 			break ;
	}
	
	return( UcSelPwmMod << 4 ) ;
}

 #ifdef	NEUTRAL_CENTER
//********************************************************************************
// Function Name 	: TneHvc
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Tunes the Hall VC offset
// History			: First edition 				
//********************************************************************************
UINT8	TneHvc( void )
{
	UINT8	UcRsltSts;
	INT32			SlMeasureParameterA , SlMeasureParameterB ;
	INT32			SlMeasureParameterNum ;
	UnllnVal		StMeasValueA , StMeasValueB ;
	INT32			SlMeasureAveValueA , SlMeasureAveValueB ;
	
	RtnCen( BOTH_OFF ) ;		// Both OFF
	
	WitTim( 500 ) ;
	
	//平均値測定
	
	MesFil( THROUGH ) ;					// Set Measure Filter

	SlMeasureParameterNum	=	64 ;		// 64times
	SlMeasureParameterA		=	(UINT32)HALL_RAM_HXIDAT ;		// Set Measure RAM Address
	SlMeasureParameterB		=	(UINT32)HALL_RAM_HYIDAT ;		// Set Measure RAM Address

	MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;					// Start measure

//	ClrMesFil();					// Clear Delay RAM
//	SetWaitTime(50) ;

	MeasureWait() ;					// Wait complete of measurement

	RamRead32A( StMeasFunc_MFA_LLiIntegral1 		, &StMeasValueA.StUllnVal.UlLowVal ) ;	// X axis
	RamRead32A( StMeasFunc_MFA_LLiIntegral1 + 4 	, &StMeasValueA.StUllnVal.UlHigVal ) ;
	RamRead32A( StMeasFunc_MFB_LLiIntegral2 		, &StMeasValueB.StUllnVal.UlLowVal ) ;	// Y axis
	RamRead32A( StMeasFunc_MFB_LLiIntegral2 + 4	, &StMeasValueB.StUllnVal.UlHigVal ) ;

	SlMeasureAveValueA = (INT32)((( (INT64)StMeasValueA.UllnValue * 100 ) / SlMeasureParameterNum ) / 100 ) ;
	SlMeasureAveValueB = (INT32)((( (INT64)StMeasValueB.UllnValue * 100 ) / SlMeasureParameterNum ) / 100 ) ;

	StAdjPar.StHalAdj.UsHlxCna = ( UINT16 )(( SlMeasureAveValueA >> 16 ) & 0x0000FFFF );		//Measure Result Store
	StAdjPar.StHalAdj.UsHlxCen = StAdjPar.StHalAdj.UsHlxCna;											//Measure Result Store

	StAdjPar.StHalAdj.UsHlyCna = ( UINT16 )(( SlMeasureAveValueB >> 16 ) & 0x0000FFFF );		//Measure Result Store
	StAdjPar.StHalAdj.UsHlyCen = StAdjPar.StHalAdj.UsHlyCna;											//Measure Result Store

	UcRsltSts = EXE_END ;				// Clear Status

	return( UcRsltSts );
}
 #endif	//NEUTRAL_CENTER

 #ifdef	NEUTRAL_CENTER_FINE
//********************************************************************************
// Function Name 	: TneFin
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Tunes the Hall VC offset current optimize
// History			: First edition 				
//********************************************************************************
void	TneFin( void )
{
	UINT32	UlReadVal ;
	UINT16	UsAdxOff, UsAdyOff ;
	INT32			SlMeasureParameterNum ;
//	INT32			SlMeasureMaxValueA , SlMeasureMaxValueB ;
//	INT32			SlMeasureMinValueA , SlMeasureMinValueB ;
//	INT32			SlMeasureAmpValueA , SlMeasureAmpValueB ;
	INT32			SlMeasureAveValueA , SlMeasureAveValueB ;
	UnllnVal		StMeasValueA , StMeasValueB ;
	UINT32	UlMinimumValueA, UlMinimumValueB ;
	UINT16	UsAdxMin, UsAdyMin ;
	UINT8	UcFin ;
	
	// Get natural center offset
	RamRead32A( HALL_RAM_HXOFF,  &UlReadVal ) ;
	UsAdxOff = UsAdxMin = (UINT16)( UlReadVal >> 16 ) ;

	RamRead32A( HALL_RAM_HYOFF,  &UlReadVal ) ;
	UsAdyOff = UsAdyMin = (UINT16)( UlReadVal >> 16 ) ;

	// Servo ON
	RtnCen( BOTH_ON ) ;
	WitTim( TNE ) ;

	MesFil( THROUGH ) ;					// Filter setting for measurement

	SlMeasureParameterNum = 2000 ;

	MeasureStart( SlMeasureParameterNum , HALL_RAM_HALL_X_OUT , HALL_RAM_HALL_Y_OUT ) ;					// Start measure

	MeasureWait() ;						// Wait complete of measurement

//	RamRead32A( StMeasFunc_MFA_SiMax1 , ( UINT32 * )&SlMeasureMaxValueA ) ;		// Max value
//	RamRead32A( StMeasFunc_MFA_SiMin1 , ( UINT32 * )&SlMeasureMinValueA ) ;		// Min value
//	RamRead32A( StMeasFunc_MFA_UiAmp1 , ( UINT32 * )&SlMeasureAmpValueA ) ;		// Amp value
	RamRead32A( StMeasFunc_MFA_LLiIntegral1 	, &StMeasValueA.StUllnVal.UlLowVal ) ;	// X axis
	RamRead32A( StMeasFunc_MFA_LLiIntegral1 + 4 , &StMeasValueA.StUllnVal.UlHigVal ) ;
	SlMeasureAveValueA = (INT32)((( (INT64)StMeasValueA.UllnValue * 100 ) / SlMeasureParameterNum ) / 100 ) ;

//	RamRead32A( StMeasFunc_MFB_SiMax2 , ( UINT32 * )&SlMeasureMaxValueB ) ;	// Max value
//	RamRead32A( StMeasFunc_MFB_SiMin2 , ( UINT32 * )&SlMeasureMinValueB ) ;	// Min value
//	RamRead32A( StMeasFunc_MFB_UiAmp2 , ( UINT32 * )&SlMeasureAmpValueB ) ;		// Amp value
	RamRead32A( StMeasFunc_MFB_LLiIntegral2 	, &StMeasValueB.StUllnVal.UlLowVal ) ;	// Y axis
	RamRead32A( StMeasFunc_MFB_LLiIntegral2 + 4	, &StMeasValueB.StUllnVal.UlHigVal ) ;
	SlMeasureAveValueB = (INT32)((( (INT64)StMeasValueB.UllnValue * 100 ) / SlMeasureParameterNum ) / 100 ) ;




	UlMinimumValueA = abs(SlMeasureAveValueA) ;
	UlMinimumValueB = abs(SlMeasureAveValueB) ;
	UcFin = 0x11 ;

	while( UcFin ) {
		if( UcFin & 0x01 ) {
			if( UlMinimumValueA >= abs(SlMeasureAveValueA) ) {
				UlMinimumValueA = abs(SlMeasureAveValueA) ;
				UsAdxMin = UsAdxOff ;
				// 収束を早めるために、出力値に比例させる
				if( SlMeasureAveValueA > 0 )
					UsAdxOff = (INT16)UsAdxOff + (SlMeasureAveValueA >> 17) + 1 ;
				else
					UsAdxOff = (INT16)UsAdxOff + (SlMeasureAveValueA >> 17) - 1 ;

				RamWrite32A( HALL_RAM_HXOFF,  (UINT32)((UsAdxOff << 16 ) & 0xFFFF0000 )) ;
			} else {
				UcFin &= 0xFE ;
			}
		}

		if( UcFin & 0x10 ) {
			if( UlMinimumValueB >= abs(SlMeasureAveValueB) ) {
				UlMinimumValueB = abs(SlMeasureAveValueB) ;
				UsAdyMin = UsAdyOff ;
				// 収束を早めるために、出力値に比例させる
				if( SlMeasureAveValueB > 0 )
					UsAdyOff = (INT16)UsAdyOff + (SlMeasureAveValueB >> 17) + 1 ;
				else
					UsAdyOff = (INT16)UsAdyOff + (SlMeasureAveValueB >> 17) - 1 ;

				RamWrite32A( HALL_RAM_HYOFF,  (UINT32)((UsAdyOff << 16 ) & 0xFFFF0000 )) ;
			} else {
				UcFin &= 0xEF ;
			}
		}
		
		MeasureStart( SlMeasureParameterNum , HALL_RAM_HALL_X_OUT , HALL_RAM_HALL_Y_OUT ) ;					// Start measure
		MeasureWait() ;						// Wait complete of measurement

//		RamRead32A( StMeasFunc_MFA_SiMax1 , ( UINT32 * )&SlMeasureMaxValueA ) ;		// Max value
//		RamRead32A( StMeasFunc_MFA_SiMin1 , ( UINT32 * )&SlMeasureMinValueA ) ;		// Min value
//		RamRead32A( StMeasFunc_MFA_UiAmp1 , ( UINT32 * )&SlMeasureAmpValueA ) ;		// Amp value
		RamRead32A( StMeasFunc_MFA_LLiIntegral1 	, &StMeasValueA.StUllnVal.UlLowVal ) ;	// X axis
		RamRead32A( StMeasFunc_MFA_LLiIntegral1 + 4 , &StMeasValueA.StUllnVal.UlHigVal ) ;
		SlMeasureAveValueA = (INT32)((( (INT64)StMeasValueA.UllnValue * 100 ) / SlMeasureParameterNum ) / 100 ) ;

//		RamRead32A( StMeasFunc_MFB_SiMax2 , ( UINT32 * )&SlMeasureMaxValueB ) ;	// Max value
//		RamRead32A( StMeasFunc_MFB_SiMin2 , ( UINT32 * )&SlMeasureMinValueB ) ;	// Min value
//		RamRead32A( StMeasFunc_MFB_UiAmp2 , ( UINT32 * )&SlMeasureAmpValueB ) ;		// Amp value
		RamRead32A( StMeasFunc_MFB_LLiIntegral2 	, &StMeasValueB.StUllnVal.UlLowVal ) ;	// Y axis
		RamRead32A( StMeasFunc_MFB_LLiIntegral2 + 4	, &StMeasValueB.StUllnVal.UlHigVal ) ;
		SlMeasureAveValueB = (INT32)((( (INT64)StMeasValueB.UllnValue * 100 ) / SlMeasureParameterNum ) / 100 ) ;



	}	// while




	StAdjPar.StHalAdj.UsHlxCna = UsAdxMin;								//Measure Result Store
	StAdjPar.StHalAdj.UsHlxCen = StAdjPar.StHalAdj.UsHlxCna;			//Measure Result Store

	StAdjPar.StHalAdj.UsHlyCna = UsAdyMin;								//Measure Result Store
	StAdjPar.StHalAdj.UsHlyCen = StAdjPar.StHalAdj.UsHlyCna;			//Measure Result Store

	StAdjPar.StHalAdj.UsAdxOff = StAdjPar.StHalAdj.UsHlxCna  ;
	StAdjPar.StHalAdj.UsAdyOff = StAdjPar.StHalAdj.UsHlyCna  ;

	// Servo OFF
	RtnCen( BOTH_OFF ) ;		// Both OFF

}
 #endif	//NEUTRAL_CENTER_FINE

//********************************************************************************
// Function Name 	: IniNvc
// Retun Value		: NON
// Argment Value	: direction
// Explanation		: Set each direction sign function
//********************************************************************************
void	IniNvc( INT16 SsX, INT16 SsY )
{
	SsNvcX = SsX ;
	SsNvcY = SsY ;
}

//********************************************************************************
// Function Name 	: TneSltPos
// Retun Value		: NON
// Argment Value	: Position number(1, 2, 3, 4, 5, 6, 7, 0:reset)
// Explanation		: Move measurement position function
//********************************************************************************
void	TneSltPos( UINT8 UcPos )
{
	INT16 SsOff = 0x0000 ;

	UcPos &= 0x07 ;
	
	if ( UcPos ) {
		SsOff = SLT_OFFSET * (UcPos - 4);
	}


	RamWrite32A( HALL_RAM_HXOFF1,  (INT32)((SsOff * SsNvcX) << 16) ) ;
	RamWrite32A( HALL_RAM_HYOFF1,  (INT32)((SsOff * SsNvcY) << 16) ) ;

}

//********************************************************************************
// Function Name 	: TneVrtPos
// Retun Value		: NON
// Argment Value	: Position number(1, 2, 3, 4, 5, 6, 7, 0:reset)
// Explanation		: Move measurement position function
//********************************************************************************
void	TneVrtPos( UINT8 UcPos )
{
	INT16 SsOff = 0x0000 ;

	UcPos &= 0x07 ;
	
	if ( UcPos ) {
		SsOff = SLT_OFFSET * (UcPos - 4);
	}


	RamWrite32A( HALL_RAM_HXOFF1,  (INT32)0 ) ;
	RamWrite32A( HALL_RAM_HYOFF1,  (INT32)((SsOff * SsNvcY) << 16) ) ;
}

//********************************************************************************
// Function Name 	: TneHrzPos
// Retun Value		: NON
// Argment Value	: Position number(1, 2, 3, 4, 5, 6, 7, 0:reset)
// Explanation		: Move measurement position function
//********************************************************************************
void	TneHrzPos( UINT8 UcPos )
{
	INT16 SsOff = 0x0000 ;

	UcPos &= 0x07 ;
	
	if ( UcPos ) {
		SsOff = SLT_OFFSET * (UcPos - 4);
	}


	RamWrite32A( HALL_RAM_HXOFF1,  (INT32)((SsOff * SsNvcX) << 16) ) ;
	RamWrite32A( HALL_RAM_HYOFF1,  (INT32)0 ) ;
}

//********************************************************************************
// Function Name 	: SetSinWavGenInt
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Sine wave generator initial Function
// History			: First edition 						
//********************************************************************************
void	SetSinWavGenInt( void )
{
	
	RamWrite32A( SinWave_Offset		,	0x00000000 ) ;		// 発生周波数のオフセットを設定
	RamWrite32A( SinWave_Phase		,	0x00000000 ) ;		// 正弦波の位相量
	RamWrite32A( SinWave_Gain		,	0x00000000 ) ;		// 発生周波数のアッテネータ(初期値は0[dB])
//	RamWrite32A( SinWave_Gain		,	0x7FFFFFFF ) ;		// 発生周波数のアッテネータ(初期値はCut)
//	SetTransDataAdr( SinWave_OutAddr	,	(UINT32)SinWave_Output ) ;		// 初期値の出力先アドレスは、自分のメンバ

	RamWrite32A( CosWave_Offset		,	0x00000000 );		// 発生周波数のオフセットを設定
	RamWrite32A( CosWave_Phase 		,	0x20000000 );		// 正弦波の位相量
	RamWrite32A( CosWave_Gain 		,	0x00000000 );		// 発生周波数のアッテネータ(初期値はCut)
//	RamWrite32A( CosWave_Gain 		,	0x7FFFFFFF );		// 発生周波数のアッテネータ(初期値は0[dB])
//	SetTransDataAdr( CosWave_OutAddr	,	(UINT32)CosWave_Output );		// 初期値の出力先アドレスは、自分のメンバ
	
	RamWrite32A( SinWaveC_Regsiter	,	0x00000000 ) ;								// Sine Wave Stop
	
}


//********************************************************************************
// Function Name 	: SetTransDataAdr
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Trans Address for Data Function
// History			: First edition 						
//********************************************************************************
void	SetTransDataAdr( UINT16 UsLowAddress , UINT32 UlLowAdrBeforeTrans )
{
	UnDwdVal	StTrsVal ;
	
	if( UlLowAdrBeforeTrans < 0x00009000 ){
		StTrsVal.UlDwdVal = UlLowAdrBeforeTrans ;
	}else{
		StTrsVal.StDwdVal.UsHigVal = (UINT16)(( UlLowAdrBeforeTrans & 0x0000F000 ) >> 8 ) ;
		StTrsVal.StDwdVal.UsLowVal = (UINT16)( UlLowAdrBeforeTrans & 0x00000FFF ) ;
	}
	RamWrite32A( UsLowAddress	,	StTrsVal.UlDwdVal );
	
}

//********************************************************************************
// Function Name 	: RdFwVr
// Retun Value		: Firmware version
// Argment Value	: NON
// Explanation		: Read Fw Version Function
// History			: First edition 						
//********************************************************************************
UINT16	RdFwVr( void )
{
	UINT16	UsVerVal ;
	
	UsVerVal = (UINT16)((MDL_VER << 8) | FW_VER ) ;
	return( UsVerVal ) ;
}

//********************************************************************************
// Function Name 	: RdStatus
// Retun Value		: 0:success 1:FAILURE
// Argment Value	: bit check  0:ALL  1:bit24
// Explanation		: High level status check Function
// History			: First edition 						
//********************************************************************************
UINT8	RdStatus( UINT8 UcStBitChk )
{
	UINT32	UlReadVal ;
	
	RamRead32A( CMD_READ_STATUS , &UlReadVal );
	if( UcStBitChk ){
		UlReadVal &= READ_STATUS_INI ;
	}
	if( !UlReadVal ){
		return( SUCCESS );
	}else{
		return( FAILURE );
	}
}


//********************************************************************************
// Function Name 	: DacControl
// Retun Value		: Firmware version
// Argment Value	: NON
// Explanation		: Dac Control Function
// History			: First edition 						
//********************************************************************************
void	DacControl( UINT8 UcMode, UINT32 UiChannel, UINT32 PuiData )
{
	UINT32	UlAddaInt ;
	if( !UcMode ) {
		RamWrite32A( CMD_IO_ADR_ACCESS , ADDA_DASEL ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS , UiChannel ) ;
		RamWrite32A( CMD_IO_ADR_ACCESS , ADDA_DAO ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS , PuiData ) ;
		;
		;
		UlAddaInt = 0x00000040 ;
		while ( (UlAddaInt & 0x00000040) != 0 ) {
			RamWrite32A( CMD_IO_ADR_ACCESS , ADDA_ADDAINT ) ;
			RamRead32A(  CMD_IO_DAT_ACCESS , &UlAddaInt ) ;
			; 
		}
	} else {
		RamWrite32A( CMD_IO_ADR_ACCESS , ADDA_DASEL ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS , UiChannel ) ;
		RamWrite32A( CMD_IO_ADR_ACCESS , ADDA_DAO ) ;
		RamRead32A(  CMD_IO_DAT_ACCESS , &PuiData ) ;
		;
		;
		UlAddaInt = 0x00000040 ;
		while ( (UlAddaInt & 0x00000040) != 0 ) {
			RamWrite32A( CMD_IO_ADR_ACCESS , ADDA_ADDAINT ) ;
			RamRead32A(  CMD_IO_DAT_ACCESS , &UlAddaInt ) ;
			;
		}
	}

	return ;
}

//********************************************************************************
// Function Name 	: WrHallCalData
// Retun Value		: 0:OK, 1:NG
// Argment Value	: NON
// Explanation		: Flash Write Hall Calibration Data Function
// History			: First edition 						
//********************************************************************************
INT16	WrHallCalData( void )
{
	UINT32	UlReadVal ;
	INT16 iRetVal;

	RamRead32A(  StCaliData_UsCalibrationStatus , &UlReadVal ) ;
#ifdef	SEL_CLOSED_AF
	UlReadVal &= ~( HALL_CALB_FLG | CLAF_CALB_FLG | HALL_CALB_BIT );
#else
	UlReadVal &= ~( HALL_CALB_FLG | HALL_CALB_BIT );
#endif
	UlReadVal |= StAdjPar.StHalAdj.UlAdjPhs ;
	RamWrite32A( StCaliData_UsCalibrationStatus , 	UlReadVal ) ;

	RamWrite32A( StCaliData_SiHallMax_Before_X ,	(UINT32)(StAdjPar.StHalAdj.UsHlxMax << 16)) ;
	RamWrite32A( StCaliData_SiHallMin_Before_X ,	(UINT32)(StAdjPar.StHalAdj.UsHlxMin << 16)) ;
	RamWrite32A( StCaliData_SiHallMax_After_X ,		(UINT32)(StAdjPar.StHalAdj.UsHlxMxa << 16)) ;
	RamWrite32A( StCaliData_SiHallMin_After_X ,		(UINT32)(StAdjPar.StHalAdj.UsHlxMna << 16)) ;
	RamWrite32A( StCaliData_SiHallMax_Before_Y ,	(UINT32)(StAdjPar.StHalAdj.UsHlyMax << 16)) ;
	RamWrite32A( StCaliData_SiHallMin_Before_Y ,	(UINT32)(StAdjPar.StHalAdj.UsHlyMin << 16)) ;
	RamWrite32A( StCaliData_SiHallMax_After_Y ,		(UINT32)(StAdjPar.StHalAdj.UsHlyMxa << 16)) ;
	RamWrite32A( StCaliData_SiHallMin_After_Y ,		(UINT32)(StAdjPar.StHalAdj.UsHlyMna << 16)) ;
	RamWrite32A( StCaliData_UiHallBias_X ,			(UINT32)(StAdjPar.StHalAdj.UsHlxGan << 16)) ;
	RamWrite32A( StCaliData_UiHallOffset_X ,		(UINT32)(StAdjPar.StHalAdj.UsHlxOff << 16)) ;
	RamWrite32A( StCaliData_UiHallBias_Y ,			(UINT32)(StAdjPar.StHalAdj.UsHlyGan << 16)) ;
	RamWrite32A( StCaliData_UiHallOffset_Y ,		(UINT32)(StAdjPar.StHalAdj.UsHlyOff << 16)) ;
	
	RamWrite32A( StCaliData_SiLoopGain_X ,			StAdjPar.StLopGan.UlLxgVal ) ;
	RamWrite32A( StCaliData_SiLoopGain_Y ,			StAdjPar.StLopGan.UlLygVal ) ;
	RamWrite32A( StCaliData_SiLensCen_Offset_X ,	(UINT32)(StAdjPar.StHalAdj.UsAdxOff << 16)) ;
	RamWrite32A( StCaliData_SiLensCen_Offset_Y ,	(UINT32)(StAdjPar.StHalAdj.UsAdyOff << 16)) ;
	RamWrite32A( StCaliData_SiOtpCen_Offset_X ,		0L ) ;
	RamWrite32A( StCaliData_SiOtpCen_Offset_Y ,		0L ) ;
	RamWrite32A( StCaliData_SiGyroOffset_X ,		(UINT32)(StAdjPar.StGvcOff.UsGxoVal << 16)) ;
	RamWrite32A( StCaliData_SiGyroOffset_Y ,		(UINT32)(StAdjPar.StGvcOff.UsGyoVal << 16)) ;
	
#ifdef	SEL_CLOSED_AF
	RamWrite32A( StCalDatAd_SiHallMax_Before_Z ,	(UINT32)(StAdjPar.StHalAdj.UsHlzMax << 16)) ;
	RamWrite32A( StCalDatAd_SiHallMin_Before_Z ,	(UINT32)(StAdjPar.StHalAdj.UsHlzMin << 16)) ;
	RamWrite32A( StCalDatAd_SiHallMax_After_Z ,		(UINT32)(StAdjPar.StHalAdj.UsHlzMxa << 16)) ;
	RamWrite32A( StCalDatAd_SiHallMin_After_Z ,		(UINT32)(StAdjPar.StHalAdj.UsHlzMna << 16)) ;
	RamWrite32A( StCaliData_UiHallBias_AF ,			(UINT32)(StAdjPar.StHalAdj.UsHlzGan << 16)) ;
	RamWrite32A( StCaliData_UiHallOffset_AF ,		(UINT32)(StAdjPar.StHalAdj.UsHlzOff << 16)) ;
	RamWrite32A( StCaliData_SiAD_Offset_AF ,		(UINT32)(StAdjPar.StHalAdj.UsAdzOff << 16)) ;
	RamWrite32A( StCaliData_SiLoopGain_AF ,			StAdjPar.StLopGan.UlLzgVal ) ;
#endif

	//
	// Flash update procedure
	//
	// Read calibration sector to buffer
	FlashNVR_ReadData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	_PUT_UINT32( UlReadVal,											CALIBRATION_STATUS	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlxMax << 16),	HALL_MAX_BEFORE_X	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlxMin << 16),	HALL_MIN_BEFORE_X	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlxMxa << 16),	HALL_MAX_AFTER_X	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlxMna << 16),	HALL_MIN_AFTER_X	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlyMax << 16),	HALL_MAX_BEFORE_Y	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlyMin << 16),	HALL_MIN_BEFORE_Y	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlyMxa << 16),	HALL_MAX_AFTER_Y	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlyMna << 16),	HALL_MIN_AFTER_Y	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlxGan << 16),	HALL_BIAS_DAC_X		) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlxOff << 16),	HALL_OFFSET_DAC_X	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlyGan << 16),	HALL_BIAS_DAC_Y		) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlyOff << 16),	HALL_OFFSET_DAC_Y	) ;
	
	_PUT_UINT32( StAdjPar.StLopGan.UlLxgVal, 						LOOP_GAIN_VALUE_X	) ;
	_PUT_UINT32( StAdjPar.StLopGan.UlLygVal,							LOOP_GAIN_VALUE_Y	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsAdxOff << 16),	LENS_CENTER_VALUE_X	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsAdyOff << 16),	LENS_CENTER_VALUE_Y	) ;
	_PUT_UINT32( 0L,													OPT_CENTER_VALUE_X	) ;
	_PUT_UINT32( 0L,													OPT_CENTER_VALUE_Y	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StGvcOff.UsGxoVal << 16),	GYRO_OFFSET_VALUE_X	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StGvcOff.UsGyoVal << 16),	GYRO_OFFSET_VALUE_Y	) ;

#ifdef	SEL_CLOSED_AF
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlzMax << 16),	HALL_MAX_BEFORE_Z	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlzMin << 16),	HALL_MIN_BEFORE_Z	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlzMxa << 16),	HALL_MAX_AFTER_Z	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlzMna << 16),	HALL_MIN_AFTER_Z	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlzGan << 16),	HALL_BIAS_DAC_AF	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsHlzOff << 16),	HALL_OFFSET_DAC_AF	) ;
	_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsAdzOff << 16),	LENS_CENTER_VALUE_AF) ;
	_PUT_UINT32( StAdjPar.StLopGan.UlLzgVal,							LOOP_GAIN_VALUE_AF	) ;
#endif

	// Sector erase
	FlashNVRSectorErase_Byte( CALIBRATION_DATA_ADDRESS );

	// Write calibration sector from buffer
	FlashNVR_WriteData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	// Sector erify function
	iRetVal = FlashNVRVerify_Byte( CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256 );

#ifdef	COPY_RAM
	if( iRetVal == 0 ) {
		UINT8	UcStRd = 1;

		FlashInitialSetting( 0 );
		// Flash to memory copy high level command
		RamWrite32A( CMD_FLASH_LOAD, 0x00000001 );
		while( UcStRd ) {
			UcStRd = RdStatus(1);
		}
//		FlashReset( );
	}
#endif
	return iRetVal;
}

//********************************************************************************
// Function Name 	: WrGyroGainData
// Retun Value		: 0:OK, 1:NG
// Argment Value	: NON
// Explanation		: Flash Write Hall Calibration Data Function
// History			: First edition 						
//********************************************************************************
INT16	WrGyroGainData( void )
{
	UINT32	UlReadVal ;
	UINT32	UlZoomX, UlZoomY;
	INT16 iRetVal;

	RamRead32A(  StCaliData_UsCalibrationStatus , &UlReadVal ) ;
	UlReadVal &= ~GYRO_GAIN_FLG;
	RamWrite32A( StCaliData_UsCalibrationStatus , 	UlReadVal ) ;
	
	RamRead32A(  GyroFilterTableX_gxzoom , &UlZoomX ) ;
	RamWrite32A( StCaliData_SiGyroGain_X ,	UlZoomX ) ;
	
	RamRead32A(  GyroFilterTableY_gyzoom , &UlZoomY ) ;
	RamWrite32A( StCaliData_SiGyroGain_Y ,	UlZoomY ) ;
	
	//
	// Flash update procedure
	//
	// Read calibration sector to buffer
	FlashNVR_ReadData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	_PUT_UINT32( UlReadVal,											CALIBRATION_STATUS	) ;
	_PUT_UINT32( UlZoomX,											GYRO_GAIN_VALUE_X	) ;
	_PUT_UINT32( UlZoomY,											GYRO_GAIN_VALUE_Y	) ;

	// Sector erase
	FlashNVRSectorErase_Byte( CALIBRATION_DATA_ADDRESS );

	// Write calibration sector from buffer
	FlashNVR_WriteData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	// Sector erify function
	iRetVal = FlashNVRVerify_Byte( CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256 );

#ifdef	COPY_RAM
	if( iRetVal == 0 ) {
		UINT8	UcStRd = 1;

		FlashInitialSetting( 0 );
		// Flash to memory copy high level command
		RamWrite32A( CMD_FLASH_LOAD, 0x00000001 );
		while( UcStRd ) {
			UcStRd = RdStatus(1);
		}
//		FlashReset( );
	}
#endif

	return iRetVal;
}

//********************************************************************************
// Function Name 	: WrGyroGainData_NV
// Retun Value		: 0:OK, 1:NG
// Argment Value	: UlReadValX: gyro gain X, UlReadValY: gyro gain Y
// Explanation		: Flash Write Hall Calibration Data Function
// History			: First edition 						
//********************************************************************************
INT16	WrGyroGainData_NV( UINT32 UlReadValX , UINT32 UlReadValY )
{
	UINT32	UlReadVal ;
	INT16 iRetVal;

	RamRead32A(  StCaliData_UsCalibrationStatus , &UlReadVal ) ;
	UlReadVal &= ~GYRO_GAIN_FLG;
	RamWrite32A( StCaliData_UsCalibrationStatus , 	UlReadVal ) ;
	
	RamWrite32A( StCaliData_SiGyroGain_X ,		UlReadValX ) ;
	
	RamWrite32A( StCaliData_SiGyroGain_Y ,		UlReadValY ) ;
	
	//
	// Flash update procedure
	//
	// Read calibration sector to buffer
	FlashNVR_ReadData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	_PUT_UINT32( UlReadVal,											CALIBRATION_STATUS	) ;
	_PUT_UINT32( UlReadValX,											GYRO_GAIN_VALUE_X	) ;
	_PUT_UINT32( UlReadValY,											GYRO_GAIN_VALUE_Y	) ;

	// Sector erase
	FlashNVRSectorErase_Byte( CALIBRATION_DATA_ADDRESS );

	// Write calibration sector from buffer
	FlashNVR_WriteData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	// Sector erify function
	iRetVal = FlashNVRVerify_Byte( CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256 );

#ifdef	COPY_RAM
	if( iRetVal == 0 ) {
		UINT8	UcStRd = 1;

		FlashInitialSetting( 0 );
		// Flash to memory copy high level command
		RamWrite32A( CMD_FLASH_LOAD, 0x00000001 );
		while( UcStRd ) {
			UcStRd = RdStatus(1);
		}
//		FlashReset( );
	}
#endif

	return iRetVal;
}


//********************************************************************************
// Function Name 	: RdHallCalData
// Retun Value		: Read calibration data
// Argment Value	: NON
// Explanation		: Flash Write Hall Calibration Data Function
// History			: First edition 						
//********************************************************************************
void	RdHallCalData( void )
{
	UnDwdVal		StReadVal ;

	RamRead32A(  StCaliData_UsCalibrationStatus, &StAdjPar.StHalAdj.UlAdjPhs ) ;

	RamRead32A( StCaliData_SiHallMax_Before_X,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlxMax = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiHallMin_Before_X, &StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlxMin = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiHallMax_After_X,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlxMxa = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiHallMin_After_X,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlxMna = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiHallMax_Before_Y, &StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlyMax = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiHallMin_Before_Y, &StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlyMin = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiHallMax_After_Y,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlyMxa = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiHallMin_After_Y,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlyMna = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_UiHallBias_X,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlxGan = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_UiHallOffset_X,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlxOff = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_UiHallBias_Y,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlyGan = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_UiHallOffset_Y,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsHlyOff = StReadVal.StDwdVal.UsHigVal ;

	RamRead32A( StCaliData_SiLoopGain_X,	&StAdjPar.StLopGan.UlLxgVal ) ;
	RamRead32A( StCaliData_SiLoopGain_Y,	&StAdjPar.StLopGan.UlLygVal ) ;

	RamRead32A( StCaliData_SiLensCen_Offset_X,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsAdxOff = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiLensCen_Offset_Y,	&StReadVal.UlDwdVal ) ;
	StAdjPar.StHalAdj.UsAdyOff = StReadVal.StDwdVal.UsHigVal ;

	RamRead32A( StCaliData_SiGyroOffset_X,		&StReadVal.UlDwdVal ) ;
	StAdjPar.StGvcOff.UsGxoVal = StReadVal.StDwdVal.UsHigVal ;
	RamRead32A( StCaliData_SiGyroOffset_Y,		&StReadVal.UlDwdVal ) ;
	StAdjPar.StGvcOff.UsGyoVal = StReadVal.StDwdVal.UsHigVal ;

}


//********************************************************************************
// Function Name 	: TneADO
// Retun Value		: 0x0000:PASS, 0x0001:X MAX OVER, 0x0002:Y MAX OVER, 0x0003:X MIN OVER, 0x0004:Y MIN OVER, FFFF:Verify error
//					: 0x0100:X MAX RANGE ERROR, 0x0200:Y MAX RANGE ERROR, 0x0300:X MIN RANGE ERROR, 0x0400:Y MIN ERROR
// Argment Value	: 
// Explanation		: calculation margin Function
// History			: First edition 						
//********************************************************************************
#if 0//float disabled
UINT16	TneADO( )
{
	UINT16	UsSts = 0 ;
	INT32 limit ;
	INT32 gxgain ;
	INT32 gygain ;
	INT16 gout_x_marginp ;
	INT16 gout_x_marginm ;
	INT16 gout_y_marginp ;
	INT16 gout_y_marginm ;

	INT16 x_max ;
	INT16 x_min ;
	INT16 x_off ;
	INT16 y_max ;
	INT16 y_min ;
	INT16 y_off ;
	INT16 x_max_after ;
	INT16 x_min_after ;
	INT16 y_max_after ;
	INT16 y_min_after ;
	INT16 gout_x ;
	INT16 gout_y ;

	//
	// Flash update procedure
	//
	// Read calibration sector to buffer
	FlashNVR_ReadData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	// Read calibration data
	RdHallCalData();

	x_max = (INT16)StAdjPar.StHalAdj.UsHlxMxa ;
	x_min = (INT16)StAdjPar.StHalAdj.UsHlxMna ;
	x_off = (INT16)StAdjPar.StHalAdj.UsAdxOff ;
	y_max = (INT16)StAdjPar.StHalAdj.UsHlyMxa ;
	y_min = (INT16)StAdjPar.StHalAdj.UsHlyMna ;
	y_off = (INT16)StAdjPar.StHalAdj.UsAdyOff ;

	RamRead32A( GF_LimitX_HLIMT,	&limit ) ;
	RamRead32A( StCaliData_SiGyroGain_X,	&gxgain ) ;
	RamRead32A( StCaliData_SiGyroGain_Y,	&gygain ) ;

	x_max_after = (x_max - x_off) ;
	if (x_off < 0)
	{
	    if ((0x7FFF - abs(x_max)) < abs(x_off)) x_max_after = 0x7FFF ;
	}

	x_min_after = (x_min - x_off) ;
	if (x_off > 0)
	{
	    if ((0x7FFF - abs(x_min)) < abs(x_off)) x_min_after = 0x8001 ;
	}

	y_max_after = (y_max - y_off) ;
	if (y_off < 0)
	{
	    if ((0x7FFF - abs(y_max)) < abs(y_off)) y_max_after = 0x7FFF ;
	}

	y_min_after = (y_min - y_off);
	if (y_off > 0)
	{
	    if ((0x7FFF - abs(y_min)) < abs(y_off)) y_min_after = 0x8001 ;
	}

	gout_x = (INT16)((INT32)(((float)gxgain / 0x7FFFFFFF) * limit * 4) >> 16);
	gout_y = (INT16)((INT32)(((float)gygain / 0x7FFFFFFF) * limit * 4) >> 16);


	gout_x_marginp = (INT16)(gout_x + LENS_MARGIN);			// MARGIN X+
	gout_x_marginm = (INT16)((gout_x + LENS_MARGIN) * -1);	// MARGIN X-
	gout_y_marginp = (INT16)(gout_y + LENS_MARGIN);			// MARGIN Y+
	gout_y_marginm = (INT16)((gout_y + LENS_MARGIN) * -1);	// MARGIN Y-



	// マージンがまったくないものは不良とする
	if (x_max_after < gout_x) {
		UsSts = 1 ;
	}
	else if (y_max_after < gout_y) {
		UsSts = 2 ;
	}
	else if (x_min_after > (gout_x * -1)) {
		UsSts = 3 ;
	}
	else if (y_min_after > (gout_y * -1)) {
		UsSts = 4 ;
	}
	else {
		// マージンオーバーであれば、ADOFFSETを更新する
		if (x_max_after < gout_x_marginp) {
			x_off -= (gout_x_marginp - x_max_after);
		}
		if (x_min_after > gout_x_marginm) {
			x_off += abs(x_min_after - gout_x_marginm);
		}
		if (y_max_after < gout_y_marginp) {
			y_off -= (gout_y_marginp - y_max_after);
		}
		if (y_min_after > gout_y_marginm) {
			y_off += abs(y_min_after - gout_y_marginm);
		}
		
		if ( (StAdjPar.StHalAdj.UsAdxOff != (UINT16)x_off) || (StAdjPar.StHalAdj.UsAdyOff != (UINT16)y_off) ) {
			StAdjPar.StHalAdj.UsAdxOff = x_off ;
			StAdjPar.StHalAdj.UsAdyOff = y_off ;

			RamWrite32A( StCaliData_SiLensCen_Offset_X ,	(UINT32)(StAdjPar.StHalAdj.UsAdxOff << 16) ) ;
			RamWrite32A( StCaliData_SiLensCen_Offset_Y ,	(UINT32)(StAdjPar.StHalAdj.UsAdyOff << 16) ) ;

			_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsAdxOff << 16),	LENS_CENTER_VALUE_X	) ;
			_PUT_UINT32( (UINT32)(StAdjPar.StHalAdj.UsAdyOff << 16),	LENS_CENTER_VALUE_Y	) ;

			// Sector erase
			FlashNVRSectorErase_Byte( CALIBRATION_DATA_ADDRESS );

			// Write calibration sector from buffer
			FlashNVR_WriteData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

			// Sector erify function
			UsSts = FlashNVRVerify_Byte( CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256 );

#ifdef	COPY_RAM
			if( UsSts == 0 ) {
				UINT8	UcStRd = 1;

				FlashInitialSetting( 0 );
				// Flash to memory copy high level command
				RamWrite32A( CMD_FLASH_LOAD, 0x00000001 );
				while( UcStRd ) {
					UcStRd = RdStatus(1);
				}
		//		FlashReset( );
			}
#endif

		}
	}

	// *******************************
	// effective range check
	// *******************************
	if (UsSts == 0) {
		UINT16 UsReadVal ;
		float flDistanceX, flDistanceY ;
		float flDistanceAD = SLT_OFFSET * 6 ;

		// effective range check
		_GET_UINT16( UsReadVal,	DISTANCE_X	) ;
//		flDistanceX = (float)UsReadVal / 10.0f ;
		flDistanceY = (float)UsReadVal / 10.0f ;	// Ver 000A2

		_GET_UINT16( UsReadVal,	DISTANCE_Y	) ;
//		flDistanceY = (float)UsReadVal / 10.0f ;
		flDistanceX = (float)UsReadVal / 10.0f ;	// Ver 000A2


		if ( (x_max_after * (flDistanceX / flDistanceAD)) < SPEC_PIXEL ) {
			// error
			UsSts |= 0x0100 ;
		}
		else if ( (y_max_after * (flDistanceY / flDistanceAD)) < SPEC_PIXEL ) {
			// error
			UsSts |= 0x0200 ;
		}
		else if ( (abs(x_min_after) * (flDistanceX / flDistanceAD)) < SPEC_PIXEL ) {
			// error
			UsSts |= 0x0300 ;
		}
		else if ( (abs(y_min_after) * (flDistanceY / flDistanceAD)) < SPEC_PIXEL ) {
			// error
			UsSts |= 0x0400 ;
		}
	}

	return( UsSts ) ;
}
#endif

//********************************************************************************
// Function Name 	: CalSectorWrite
// Retun Value		: 0:OK, 1:Address error, 2:Length error, -1:Verify error
// Argment Value	: NON
// Explanation		: 
// History			: First edition 						
//********************************************************************************
INT16 CalSectorWrite( UINT16 Address, UINT8 * DataPtr, UINT16 Num )
{
	INT16 iRetVal ;
	UINT16 i ;

	if ( Address < 0x0180 )
		return ( 1 ) ;
	
	if ( (Address + Num) > 0x0200 )
		return ( 2 ) ;

	//
	// Flash update procedure
	//
	// Read calibration sector to buffer
	FlashNVR_ReadData_Byte(	CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	for ( i = 0; i < Num; i++ ) {
		Flash_Sector[ (Address & 0xFF) + i ] = DataPtr[i] ;
	}

	// Sector erase
	FlashNVRSectorErase_Byte( CALIBRATION_DATA_ADDRESS );

	// Write calibration sector from buffer
	FlashNVR_WriteData_Byte( CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256	);

	// Sector erify function
	iRetVal = FlashNVRVerify_Byte( CALIBRATION_DATA_ADDRESS, FLASH_SECTOR_BUFFER, 256 );

	return ( iRetVal ) ;
}

//********************************************************************************
// Function Name 	: CalSectorRead
// Retun Value		: 0:OK, 1:Address error, 2:Length error
// Argment Value	: NON
// Explanation		: 
// History			: First edition 						
//********************************************************************************
INT16 CalSectorRead( UINT16 Address, UINT8 * DataPtr, UINT16 Num )
{
	if ( Address < 0x0100 )
		return ( 1 ) ;
	
	if ( (Address + Num) > 0x0200 )
		return ( 2 ) ;

	FlashNVR_ReadData_Byte(	Address, DataPtr, Num );

	return ( 0 ) ;
}

//********************************************************************************
// Function Name 	: OscStb
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: Osc Standby Function
// History			: First edition 						
//********************************************************************************
void	OscStb( void )
{
	RamWrite32A( CMD_IO_ADR_ACCESS , STBOSCPLL ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS , OSC_STB ) ;
}


//********************************************************************************
// Function Name 	: RunHea
// Retun Value		: Result
// Argment Value	: NON
// Explanation		: Hall Examination of Acceptance
// History			: First edition 						
//********************************************************************************
UINT8	RunHea( void )
{
	UINT8 	UcRst ;

	UcRst = EXE_END ;
	UcRst |= TstActMov( X_DIR) ;
	UcRst |= TstActMov( Y_DIR) ;
	
	
	return( UcRst ) ;
}
UINT8	TstActMov( UINT8 UcDirSel )
{
	INT32	SlMeasureParameterNum ;
	INT32	SlMeasureParameterA , SlMeasureParameterB ;
	UINT8	UcRsltSts;
	UINT32	UlMsppVal ;

	SlMeasureParameterNum	=	4004 ;		// 20.0195/0.005 < x

	if( UcDirSel == X_DIR ) {								// X axis
		SlMeasureParameterA		=	HallFilterD_HXDAZ1 ;		// Set Measure RAM Address
		SlMeasureParameterB		=	HallFilterD_HYDAZ1 ;		// Set Measure RAM Address
	} else if( UcDirSel == Y_DIR ) {						// Y axis
		SlMeasureParameterA		=	HallFilterD_HYDAZ1 ;		// Set Measure RAM Address
		SlMeasureParameterB		=	HallFilterD_HXDAZ1 ;		// Set Measure RAM Address
	}
	SetSinWavGenInt();
	
//	RamWrite32A( SinWave_Offset		,	0x00082F29 ) ;				// Freq Setting = Freq * 80000000h / Fs	: 5Hz
	RamWrite32A( SinWave_Offset		,	ACT_CHK_FRQ ) ;				// Freq Setting = Freq * 80000000h / Fs	: 2Hz
	RamWrite32A( SinWave_Gain		,	ACT_CHK_LVL ) ;				// Set Sine Wave Gain
	RamWrite32A( SinWaveC_Regsiter	,	0x00000001 ) ;				// Sine Wave Start
	if( UcDirSel == X_DIR ) {
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)HALL_RAM_HXOFF1 ) ;	// Set Sine Wave Input RAM
	}else if( UcDirSel == Y_DIR ){
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)HALL_RAM_HYOFF1 ) ;	// Set Sine Wave Input RAM
	}
	MesFil( NOISE ) ;					// 測定用フィルターを設定する。

	MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;					// Start measure
	
	MeasureWait() ;						// Wait complete of measurement
	
	RamWrite32A( SinWaveC_Regsiter	,	0x00000000 ) ;								// Sine Wave Stop
	
	if( UcDirSel == X_DIR ) {
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)0x00000000 ) ;	// Set Sine Wave Input RAM
		RamWrite32A( HALL_RAM_HXOFF1		,	0x00000000 ) ;				// DelayRam Clear
	}else if( UcDirSel == Y_DIR ){
		SetTransDataAdr( SinWave_OutAddr	,	(UINT32)0x00000000 ) ;	// Set Sine Wave Input RAM
		RamWrite32A( HALL_RAM_HYOFF1		,	0x00000000 ) ;				// DelayRam Clear
	}
	RamRead32A( StMeasFunc_MFA_UiAmp1 , ( UINT32 * )&UlMsppVal ) ;	// amp1 value



	
	UcRsltSts = EXE_END ;
	if( UlMsppVal > ACT_THR ){
		if ( !UcDirSel ) {					// AXIS X
			UcRsltSts = EXE_HXMVER ;
		}else{								// AXIS Y
			UcRsltSts = EXE_HYMVER ;
		}
	}
	
	return( UcRsltSts ) ;
}

//********************************************************************************
// Function Name 	: RunGea
// Retun Value		: Result
// Argment Value	: NON
// Explanation		: Gyro Examination of Acceptance
// History			: First edition 						
//********************************************************************************
#define 	GEA_NUM		512			// 512times
UINT8	RunGea( void )
{
	UnllnVal	StMeasValueA , StMeasValueB ;
	INT32		SlMeasureParameterA , SlMeasureParameterB ;
	UINT8 		UcRst, UcCnt, UcXLowCnt, UcYLowCnt, UcXHigCnt, UcYHigCnt ;
	UINT16		UsGxoVal[10], UsGyoVal[10], UsDif;
	INT32		SlMeasureParameterNum , SlMeasureAveValueA , SlMeasureAveValueB ;

	
	UcRst = EXE_END ;
	UcXLowCnt = UcYLowCnt = UcXHigCnt = UcYHigCnt = 0 ;

	MesFil( THROUGH ) ;				// 測定用フィルターを設定する。

	for( UcCnt = 0 ; UcCnt < 10 ; UcCnt++ )
	{
		//平均値測定

		MesFil( THROUGH ) ;					// Set Measure Filter

		SlMeasureParameterNum	=	GEA_NUM ;					// Measurement times
		SlMeasureParameterA		=	GYRO_RAM_GX_ADIDAT ;		// Set Measure RAM Address
		SlMeasureParameterB		=	GYRO_RAM_GY_ADIDAT ;		// Set Measure RAM Address

		MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;					// Start measure

		MeasureWait() ;					// Wait complete of measurement

		RamRead32A( StMeasFunc_MFA_LLiIntegral1 		, &StMeasValueA.StUllnVal.UlLowVal ) ;	// X axis
		RamRead32A( StMeasFunc_MFA_LLiIntegral1 + 4		, &StMeasValueA.StUllnVal.UlHigVal ) ;
		RamRead32A( StMeasFunc_MFB_LLiIntegral2 		, &StMeasValueB.StUllnVal.UlLowVal ) ;	// Y axis
		RamRead32A( StMeasFunc_MFB_LLiIntegral2 + 4		, &StMeasValueB.StUllnVal.UlHigVal ) ;

		SlMeasureAveValueA = (INT32)( (INT64)StMeasValueA.UllnValue / SlMeasureParameterNum ) ;
		SlMeasureAveValueB = (INT32)( (INT64)StMeasValueB.UllnValue / SlMeasureParameterNum ) ;
		// X
		UsGxoVal[UcCnt] = (UINT16)( SlMeasureAveValueA >> 16 );	// 平均値測定

		// Y
		UsGyoVal[UcCnt] = (UINT16)( SlMeasureAveValueB >> 16 );	// 平均値測定

		
		


		if( UcCnt > 0 )
		{
			if ( (INT16)UsGxoVal[0] > (INT16)UsGxoVal[UcCnt] ) {
				UsDif = (UINT16)((INT16)UsGxoVal[0] - (INT16)UsGxoVal[UcCnt]) ;
			} else {
				UsDif = (UINT16)((INT16)UsGxoVal[UcCnt] - (INT16)UsGxoVal[0]) ;
			}
			
			if( UsDif > GEA_DIF_HIG ) {
				//UcRst = UcRst | EXE_GXABOVE ;
				UcXHigCnt ++ ;
			}
			if( UsDif < GEA_DIF_LOW ) {
				//UcRst = UcRst | EXE_GXBELOW ;
				UcXLowCnt ++ ;
			}
			
			if ( (INT16)UsGyoVal[0] > (INT16)UsGyoVal[UcCnt] ) {
				UsDif = (UINT16)((INT16)UsGyoVal[0] - (INT16)UsGyoVal[UcCnt]) ;
			} else {
				UsDif = (UINT16)((INT16)UsGyoVal[UcCnt] - (INT16)UsGyoVal[0]) ;
			}
			
			if( UsDif > GEA_DIF_HIG ) {
				//UcRst = UcRst | EXE_GYABOVE ;
				UcYHigCnt ++ ;
			}
			if( UsDif < GEA_DIF_LOW ) {
				//UcRst = UcRst | EXE_GYBELOW ;
				UcYLowCnt ++ ;
			}
		}
	}
	
	if( UcXHigCnt >= 1 ) {
		UcRst = UcRst | EXE_GXABOVE ;
	}
	if( UcXLowCnt > 8 ) {
		UcRst = UcRst | EXE_GXBELOW ;
	}
	
	if( UcYHigCnt >= 1 ) {
		UcRst = UcRst | EXE_GYABOVE ;
	}
	if( UcYLowCnt > 8 ) {
		UcRst = UcRst | EXE_GYBELOW ;
	}
	
	
	return( UcRst ) ;
}

//********************************************************************************
// Function Name 	: FrqDet
// Retun Value		: 0:PASS, 1:OIS X NG, 2:OIS Y NG, 4:CLAF NG
// Argment Value	: NON
// Explanation		: Module Check 
// History			: First edition 						
//********************************************************************************
UINT8 FrqDet( void )
{
	INT32 SlMeasureParameterA , SlMeasureParameterB ;
	INT32 SlMeasureParameterNum ;
	UINT32 UlXasP_P , UlYasP_P ;
#ifdef	SEL_CLOSED_AF
	UINT32 UlAasP_P ;
#endif	// SEL_CLOSED_AF

	UINT8 UcRtnVal;

	UcRtnVal = 0;

	//Measurement Setup
	MesFil( OSCCHK ) ;													// Set Measure Filter

	SlMeasureParameterNum	=	1000 ;									// 1000times( 50ms )
	SlMeasureParameterA		=	(UINT32)HALL_RAM_HXOUT0 ;		// Set Measure RAM Address
	SlMeasureParameterB		=	(UINT32)HALL_RAM_HYOUT0 ;		// Set Measure RAM Address

	// impulse Set
//	RamWrite32A( HALL_RAM_HXOFF1 , STEP1 ) ;							// X manual 
//	RamWrite32A( HALL_RAM_HYOFF1 , STEP1 ) ;							// Y manual

//	RamWrite32A( HALL_RAM_HXOFF1 , STEP2 ) ;							// X manual 
//	RamWrite32A( HALL_RAM_HYOFF1 , STEP2 ) ;							// Y manual
	WitTim( 300 ) ;

	// Start measure
	MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;	
	SetWaitTime(1) ;
	MeasureWait() ;														// Wait complete of measurement
	RamRead32A( StMeasFunc_MFA_UiAmp1, &UlXasP_P ) ;					// X Axis Peak to Peak
	RamRead32A( StMeasFunc_MFB_UiAmp2, &UlYasP_P ) ;					// Y Axis Peak to Peak

	WitTim( 50 ) ;

	// Osc Value Check X
	if(  UlXasP_P > ULTHDVAL ){
		UcRtnVal = 1;
	}
	// Osc Value Check Y
	if(  UlYasP_P > ULTHDVAL ){
		UcRtnVal |= 2;
	}

#ifdef	SEL_CLOSED_AF
	// CLAF
	SlMeasureParameterA		=	(UINT32)CLAF_RAMA_AFDEV ;		// Set Measure RAM Address
	SlMeasureParameterB		=	(UINT32)CLAF_RAMA_AFDEV ;		// Set Measure RAM Address

	// impulse Set
//	RamWrite32A( CLAF_RAMA_AFTARGET , STEP1 ) ;							// CLAF manual 

//	RamWrite32A( CLAF_RAMA_AFTARGET , STEP2 ) ;							// CLAF manual 
	WitTim( 300 ) ;

	// Start measure
	MeasureStart( SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ;	
	SetWaitTime(1) ;
	MeasureWait() ;														// Wait complete of measurement
	RamRead32A( StMeasFunc_MFA_UiAmp1, &UlAasP_P ) ;					// CLAF Axis Peak to Peak

	WitTim( 50 ) ;

	// Osc Value Check CLAF
	if(  UlAasP_P > ULTHDVAL ){
		UcRtnVal |= 4;
	}
	
#endif	// SEL_CLOSED_AF

	return(UcRtnVal);													// Retun Status value
}


//********************************************************************************
// Function Name 	: MeasureGyroAmp
// Retun Value		: NON
// Argment Value	: measurement amplitude at 6Hz 1deg
// Explanation		: Measurement the amplitude of gyro function
//********************************************************************************
#define	MESGYR_NUM	4000
void	MeasureGyroAmp( void )
{
	INT32		SiMeasureA_Amp, SiMeasureB_Amp ;
	INT32		SlMeasureParameterMax, SlMeasureParameterA , SlMeasureParameterB ;
	UINT8 		UcCnt ;
	INT32		SsGxoVal, SsGyoVal;

	SsGxoVal = SsGyoVal = 0 ;

	MesFil( THROUGH ) ;				// 測定用フィルターを設定する。

	for( UcCnt = 0 ; UcCnt < 4 ; UcCnt++ )
	{
		//平均値測定
		SlMeasureParameterMax	=	MESGYR_NUM ;				// Measurement times
		SlMeasureParameterA		=	GYRO_RAM_GX_ADIDAT ;		// Set Measure RAM Address
		SlMeasureParameterB		=	GYRO_RAM_GY_ADIDAT ;		// Set Measure RAM Address

		MeasureStart( SlMeasureParameterMax , SlMeasureParameterA , SlMeasureParameterB ) ;					// Start measure

		MeasureWait() ;					// Wait complete of measurement

		RamRead32A( StMeasFunc_MFA_UiAmp1 		, &SiMeasureA_Amp ) ;	// X axis
		RamRead32A( StMeasFunc_MFB_UiAmp2 		, &SiMeasureB_Amp ) ;	// Y axis

		// X
		SsGxoVal += (UINT16)( SiMeasureA_Amp >> 16 ) ;
			
		// Y
		SsGyoVal += (UINT16)( SiMeasureB_Amp >> 16 ) ;
	}

	SiMeasureA_Amp = SsGxoVal / 4 ;
	SiMeasureB_Amp = SsGyoVal / 4 ;

	RamWrite32A( StMeasFunc_MFA_UiAmp1 		, SiMeasureA_Amp ) ;	// X axis
	RamWrite32A( StMeasFunc_MFB_UiAmp2 		, SiMeasureB_Amp ) ;	// Y axis


}
