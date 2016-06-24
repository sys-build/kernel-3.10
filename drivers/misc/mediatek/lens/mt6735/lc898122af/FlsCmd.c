//********************************************************************************
//
//		<< LC898123 Evaluation Soft >>
//	    Program Name	: FlsCmd.c
//		Design			: K.abe
//		History			: First edition						
//********************************************************************************
//**************************
//	Include Header File		
//**************************
#include	"Ois.h"
#include	"md5.h"

#include	"FromCode.h"	// LC898123A firmware

void WPBCtrl(int flag)
{
}

//********************************************************************************
// Function Name 	: WPB level read
// Retun Value		: 0: WPB LOW, 1: WPB HIGH
// Argment Value	: NON
// Explanation		: Read WPB level
// History			: First edition 						
//********************************************************************************
UINT8 ReadWPB( void )
{
	UINT32	UlReadVal;

	RamWrite32A( CMD_IO_ADR_ACCESS, IOPLEVR ) ;		
	RamRead32A ( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
	return ( (UlReadVal & 0x0400) != 0 ? 1 : 0 ) ;
}

//********************************************************************************
// Function Name 	: Verison check
// Retun Value		: EXE_ERR, EXE_END
// Argment Value	: NON
// Explanation		: Check relation the firmware version
// History			: First edition 						
//********************************************************************************
UINT16 CheckVersion( void )
{
	UINT32	UlReadVal ;

	// Confirm the firmware version of LC898123
	RamRead32A( SiVerNum, &UlReadVal );

	if ( UlReadVal != VERNUM )
		return EXE_ERR ;
	else
		return EXE_END ;
}

//********************************************************************************
// Function Name 	: XC XD firmware check
// Retun Value		: 0: PASS, 1: NG
// Argment Value	: NON
// Explanation		: firmware with check CVER
// History			: First edition 						
//********************************************************************************

UINT16	XC_XD_Check()
{
	UINT32	UlCVER;
	UINT32	UlFromVer;

	RamWrite32A( CMD_IO_ADR_ACCESS, CVER_123 ) ;
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlCVER ) ;

	UlFromVer = GetFromVer();
	UlFromVer = (UlFromVer >> 8) & 0xFF;	// Model No
	if ( UlCVER == 0xB4 ) {					
		// AXC
		if ( (UlFromVer != 0x02) && (UlFromVer != 0x03) && (UlFromVer != 0x04) ) {
			return ( 1 );
		}
	}
	else if ( UlCVER == 0xB6 ) {
		// AXD
		if ( (UlFromVer != 0x05) && (UlFromVer != 0x06) && (UlFromVer != 0x07) ) {
			return ( 1 );
		}
	}
	else {
		return ( 1 );
	}

	return ( 0 );
}


//********************************************************************************
// Function Name 	: Initial Setting
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: <Flash Memory> Initial Setting for Program & Erase
// History			: First edition 						
//********************************************************************************

void FlashInitialSetting( char val )
{
	UINT32 UlReadVal;
	INT16 i;

#if 0
	// If CVER is 123*XD, skip this function
	RamWrite32A( CMD_IO_ADR_ACCESS, CVER_123 ) ;
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
	if( UlReadVal > 0xB4 ) {
		return ;
	}
#endif

	// Release RESET
	RamWrite32A( CMD_IO_ADR_ACCESS, SOFTRESET );
	RamRead32A ( CMD_IO_DAT_ACCESS, &UlReadVal );
	UlReadVal |= 0x00000010;									// Reset release
	
	RamWrite32A( CMD_IO_DAT_ACCESS, UlReadVal );

	// val not = 0 extend initialize
	if( val ) {
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_TPGS ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 118 ) ;					// TPGS Flash spec.  min. 2.5usec max. 3.15uec

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_TPROG ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 70 ) ;					// TPROG Flash spec.  min. 6usec max. 7.5usec

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_TERASES ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 92 ) ;					// TERASES Flash spec.  Flash spec.  min. 4msec max. 5msec

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_TERASEC ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 115 ) ;					// TERASEC Flash spec.  min. 40msec max. 50msec

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;

		//set Configuration Initialize
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 0x00000000	  ) ;		// FLA_NVR=1, A[8]=1, A[7..0]=0x00
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;		// 1 byte access
		RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;		// Disable write protect
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
//		WPBCtrl(WPB_OFF) ;										// Disable write protect
//		if ( ReadWPB() != 1 )	return ( 5 );					// WPB LOW ERROR

		for( i = 0; i < 8; i++ )
		{
			RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;
			RamWrite32A( CMD_IO_DAT_ACCESS, 0xFFFFFFFF ) ; 

			RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
			RamWrite32A( CMD_IO_DAT_ACCESS, 3) ;  				// Setconfig
		}

		// set auto configuration
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010100	  ) ;		// FLA_NVR=1, A[8]=1, A[7..0]=0x00
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;  					// Auto configuration

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;		// Enable write protect
		RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
		WPBCtrl(WPB_ON) ;										// Enable write protect
	}
}


//********************************************************************************
// Function Name 	: Reset Flash
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: <Flash Memory> Reset flash memory
// History			: First edition 						
//********************************************************************************

void FlashReset(void)
{
	UINT32 UlReadVal;

#if 0
	// If CVER is 123*XD, skip this function
	RamWrite32A( CMD_IO_ADR_ACCESS, CVER_123 ) ;
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
	if( UlReadVal > 0xB4 ) {
		return ;
	}
#endif

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

	// Set RESET
	RamWrite32A( CMD_IO_ADR_ACCESS, SOFTRESET ) ;
	RamRead32A ( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
	UlReadVal &= ~0x00000010;									// Reset set

	RamWrite32A( CMD_IO_DAT_ACCESS, UlReadVal ) ;
}






//********************************************************************************
// Function Name 	: Chip Erase <Main Array>
// Retun Value		: 0: PASS, 5: WPB_LOW ERROR
// Argment Value	: NON
// Explanation		: <Flash Memory> Main Aray Chip Erase
// History			: First edition 						
//********************************************************************************
INT16 FlashMainArrayChipErase_ALL(void)
{
	UINT16 UsCnt;
	UINT32 UlReadVal;
	
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 5 ) ; 	 					// Chip Erase Start

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_INT ) ;	
	for ( UsCnt = 0; UsCnt < 1000; UsCnt++ )					// TimeOut x4
	{
		RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( !(UlReadVal == 0x80) ){
			break;
		}
	}
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

	FlashReset();												// ***** FLASH RESET *****
	return ( 0 );
}


//********************************************************************************
// Function Name 	: FlashMainArraySectorErase_Byte
// Retun Value		: 0: PASS, 5: WPB_LOW_ERROR
// Argment Value	: (UINT32 )sector ( under 1Byte is ignore )
// Explanation		: <Flash Memory> Sector Erase
// History			: First edition 						
//********************************************************************************
INT16 FlashMainArraySectorErase_Byte( UINT16 SetAddress )
{
	UINT8 UcCnt;
	UINT32 UlReadVal;
	
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****
	// WP disable	
	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS , 1 ) ;
	// MainArray Set	
	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_ADR ) ;			// Set address
	RamWrite32A( CMD_IO_DAT_ACCESS , ( SetAddress  & 0x3F00) ) ;	// 256 = 1 sector
	// Sel Set
	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS , MakeMainSel( SetAddress) ) ;

	// Execute
	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS , 4 ) ;						// Sector Erase Start

	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_INT ) ;	
	for ( UcCnt = 0; UcCnt < 100; UcCnt++ )						// TimeOut 
	{
		RamRead32A(  CMD_IO_DAT_ACCESS , &UlReadVal ) ;
		if( !(UlReadVal ==  0x80) ){
			break;
		}
	}
	// WriteProtect Enable
	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS , 0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

	FlashReset();												// ***** FLASH RESET *****
	return ( 0 );
}

//********************************************************************************
// Function Name 	: FlashNVRSectorErase_Byte
// Retun Value		: 0: PASS, 5: WPB LOW ERROR
// Argment Value	: Address : 0 ~ 767 (Byte)  3 sesion
// Explanation		: <Flash Memory> Sector Erase
// History			: First edition 						
//********************************************************************************
INT16 FlashNVRSectorErase_Byte( UINT16 SetAddress )
{
	UINT8 UcCnt;
	UINT32 UlReadVal;
	
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	// NVR Set
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// Set NVR address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000	  ) ;
	// Sel Set
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, MakeNVRSel( SetAddress ) ) ;

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	// WP disable
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
	
	// Execute
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 4 ) ;						// Sector Erase Start

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_INT ) ;	
	for ( UcCnt = 0; UcCnt < 100; UcCnt++ )						// TimeOut 
	{
		RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( !(UlReadVal ==  0x80) ){
			break;
		}
	}
	// WriteProtect Enable
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS,0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

	FlashReset();												// ***** FLASH RESET *****
	return ( 0 );
}


//********************************************************************************
// Function Name 	: FlashNVR_ReadData_Byte
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: <Flash Memory> Read Data
// History			: First edition 						
//********************************************************************************
void FlashNVR_ReadData_Byte( UINT16 SetAddress, UINT8 * ReadPtr, UINT16 Num )
{
	UINT16 UsNum;
	UINT32 UlReadVal;

	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	// Count Set
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 1 byte read count
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

	for ( UsNum = 0; UsNum < Num; UsNum++ )
	{
		// NVR Addres Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Set NVR address
		RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 + ((SetAddress + UsNum) & 0x000000FF) ) ;
		// Sel Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS, MakeNVRSel(SetAddress + UsNum) ) ;

		// Read Start
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;					// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;
		RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		
		ReadPtr[ UsNum ] = MakeDatNVR( (SetAddress + UsNum), UlReadVal );
	}
	FlashReset();												// ***** FLASH RESET *****
}

//********************************************************************************
// Function Name 	: FlashNVR_WriteData_Byte
// Retun Value		: 0: PASS, 5: WPB_LOW ERROR
// Argment Value	: NON
// Explanation		: <Flash Memory> Read Data
// History			: First edition 						
//********************************************************************************
INT16 FlashNVR_WriteData_Byte( UINT16 SetAddress, UINT8 * WritePtr, UINT16 Num )
{
	UINT16 UsNum;

	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	// WP disable	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
	// Count Set
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 1 byte read count
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	
	for ( UsNum = 0; UsNum < Num; UsNum++ )
	{
		// NVR Addres Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Set NVR address
		RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 + ( (SetAddress + UsNum) & 0x000000FF) ) ;
		// Sel Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS, MakeNVRSel(SetAddress + UsNum) ) ;

		// Data Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;		// Ser write data
		RamWrite32A( CMD_IO_DAT_ACCESS, MakeNVRDat((SetAddress + UsNum), WritePtr[ UsNum ]) ) ;

		// Execute
		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS , 2) ;					// Program Start
	}
	// WriteProtect Enable	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

	FlashReset();												// ***** FLASH RESET *****
	return ( 0 );
}


//********************************************************************************
// Function Name 	: Write data to flash
// Retun Value		: 0: PASS, 5: WPB LOW ERROR
// Argment Value	: NON
// Explanation		: <Flash Memory> Read Data
// History			: First edition 						
//********************************************************************************
INT16 FlashWriteData_3B( UINT32 SetAddress, UINT32 * WritePtr, UINT16 Num )
{
	UINT16 UsNum;
	
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Disable write enable
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 1 byte access
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	
	for ( UsNum = 0; UsNum < Num; UsNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Write address set
		RamWrite32A( CMD_IO_DAT_ACCESS, (SetAddress + UsNum) ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;		// Write data set
		RamWrite32A( CMD_IO_DAT_ACCESS, WritePtr[ UsNum ] ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 2) ;  					// Program Start
	}

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

	FlashReset();												// ***** FLASH RESET *****
	return ( 0 ) ;
}


//********************************************************************************
// Function Name 	: FlashMainArray_ReadData_Byte
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: <Flash Memory> Read Data
// History			: First edition 						
//********************************************************************************
void FlashMainArray_ReadData_Byte( UINT16 SetAddress, UINT8 * ReadPtr, UINT16 Num )
{
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	ReadBytes( SetAddress, ReadPtr, Num );						// Read bytes function

	FlashReset();												// ***** FLASH RESET *****
}


//********************************************************************************
// Function Name 	: FlashMainArray_WriteData_Byte
// Retun Value		: 0: PASS, 5: WPB LOW ERROR
// Argment Value	: NON
// Explanation		: <Flash Memory> Read Data
// History			: First edition 						
//********************************************************************************
INT16 FlashMainArray_WriteData_Byte( UINT16 SetAddress, UINT8 * WritePtr, UINT16 Num )
{
//	UINT16 UsNum;
	
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	// WP disable
	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS , 1 ) ;

	WriteBytes( SetAddress, WritePtr, Num );					// Write bytes function

	// WriteProtect Enable	
	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS ,0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

	FlashReset();												// ***** FLASH RESET *****
	return ( 0 );
}

//********************************************************************************
// Function Name 	: Read data from flash
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: <Flash Memory> Read Data
// History			: First edition 						
//********************************************************************************
void FlashReadData_3B( UINT32 SetAddress, UINT32 * ReadPtr, UINT16 Num )
{
	UINT16 UsNum;

	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 1 byte access
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

	for (UsNum= 0 ; UsNum < Num ; UsNum++)
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Read address set
		RamWrite32A( CMD_IO_DAT_ACCESS, (SetAddress + UsNum)  ) ;
	
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;  					// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;		// Read data get
		RamRead32A(  CMD_IO_DAT_ACCESS, &(ReadPtr[ UsNum ]) ) ;
	}
	FlashReset();												// ***** FLASH RESET *****
}


//********************************************************************************
// Function Name 	: Verify NVR byte
// Retun Value		: 0:OK, -1:NG
// Argment Value	: UsAddress: flash address, pBuff: local memory address
// Explanation		: Verify flash bytes and local memory buffer bytes
// History			: First edition 						
//********************************************************************************
INT16 FlashNVRVerify_Byte( UINT16 UsAddress, UINT8 * pBuff, UINT16 Num  )
{
	UINT16 UsNum;
	UINT32 UlReadVal;
	INT16 iRetVal = 0;

	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_ACSCNT ) ;		// for 1 byte read count
	RamWrite32A( CMD_IO_DAT_ACCESS , 0 ) ;

	for( UsNum = 0; UsNum < Num; UsNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_ADR ) ;		// Set Address
		RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 + ((UsAddress + UsNum) & 0x000000FF) ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS , MakeNVRSel(UsAddress + UsNum) ) ;		

		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS , 1 ) ;  				// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_RDAT ) ;		
		RamRead32A ( CMD_IO_DAT_ACCESS , &UlReadVal ) ;
		
		if( pBuff[UsNum] != MakeDatNVR((UsAddress + UsNum), UlReadVal) ) {
			iRetVal = -1;
			break;
		}
	}
	FlashReset();												// ***** FLASH RESET *****
	return iRetVal;
}

//********************************************************************************
// Function Name 	: Verify main array byte
// Retun Value		: 0:OK, -1:NG
// Argment Value	: UsAddress: flash address, pBuff: local memory address
// Explanation		: Verify flash bytes and local memory buffer bytes
// History			: First edition 						
//********************************************************************************
INT16 FlashMainArrayVerify_Byte( UINT16 UsAddress, UINT8 * pBuff, UINT16 Num  )
{
	UINT16 UsNum;
	UINT32 UlReadVal;
	INT16 iRetVal = 0;

	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

	RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_ACSCNT ) ;		// for 1 byte read count
	RamWrite32A( CMD_IO_DAT_ACCESS , 0 ) ;

	for( UsNum = 0; UsNum < Num; UsNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_ADR ) ;		// Set Address
		RamWrite32A( CMD_IO_DAT_ACCESS , (UsAddress + UsNum) & 0x0FFF ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS , MakeMainSel(UsAddress + UsNum) ) ;		

		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS , 1 ) ;  				// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_RDAT ) ;		
		RamRead32A ( CMD_IO_DAT_ACCESS , &UlReadVal ) ;
		
		if( pBuff[UsNum] != MakeDatMain((UsAddress + UsNum), UlReadVal) ) {
			iRetVal = -1;
			break;
		}
	}
	FlashReset();												// ***** FLASH RESET *****
	return iRetVal;
}




//********************************************************************************
// Function Name 	: FlashUpdate
// Retun Value		: 0: PASS, 1: MAGIC CODE ERASED, 2: VERIFY ERROR 3: NVR VERIFY ERROR
//					: 4: LSI ERROR, 5: WPB LOW ERROR
// Argment Value	: NON
// Explanation		: Flash Write Hall Calibration Data Function
// History			: First edition 						
//********************************************************************************
INT16 FlashUpdate(void)
{
	UINT32 UlCnt;	
	UINT32 UlCalData[ 256 ] ;
	UINT32 UlNum;
	UINT32 UlReadVal; 

	// AXC AXD check for FromCode
	if ( XC_XD_Check() != 0 ) {
		return ( 4 );
	}

	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

//--------------------------------------------------------------------------------
/* 0. Start up to boot exection */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, SYS_DSP_REMAP) ;			// Read remap flag
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;

	if( UlReadVal != 0) {	
		// Remaped
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;

		WPBCtrl(WPB_OFF) ;										// Disable write protect
		if ( ReadWPB() != 1 )	return ( 5 );					// WPB LOW ERROR

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;		// Disable write protect
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// NVR SELECT
		RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010001 ) ;			// Set address of Magic code[1]

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT	 ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;					// for 1 byte program

		// Magic code erase
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;		// write data '00' set
		RamWrite32A( CMD_IO_DAT_ACCESS, 0) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;		// Address 0x0001
		RamWrite32A( CMD_IO_DAT_ACCESS, 2) ;  					// 1 byte Program

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;		// Enable write protect
		RamWrite32A( CMD_IO_DAT_ACCESS,0 ) ;
		WPBCtrl(WPB_ON) ;										// Enable write protect

		FlashReset();											// ***** FLASH RESET *****

		RamWrite32A( 0xF003 , 0 ) ;								// exit from boot
		return ( 1 );

	} else {
		// DSP CLOCK SET

		// Sel Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
		// Count Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;		// for 2 byte read
		RamWrite32A( CMD_IO_DAT_ACCESS, 2 - 1 ) ;
		// NVR Addres Set
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 + 32 ) ;		// from NVR[32]
		// Read Start
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;  					// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;		// NVR[32]
		RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, OSCRSEL ) ;				// OSCRSEL
		RamWrite32A( CMD_IO_DAT_ACCESS, UlReadVal ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;		// NVR[33]
		RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, OSCCURSEL ) ;			// OSCCURSEL
		RamWrite32A( CMD_IO_DAT_ACCESS, UlReadVal ) ;

	}

//--------------------------------------------------------------------------------
/* 2. <Main area> Chip erase */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;						// Select 3 flash

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 5 ) ;						// Chip Erase Start

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_INT ) ;	
	for ( UlCnt = 0; UlCnt < 1000; UlCnt++ )					// TimeOut
	{
		RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( !(UlReadVal ==  0x80) ){
			break;
		}
	}

//--------------------------------------------------------------------------------
/* 3. <Main area> Program  */
//--------------------------------------------------------------------------------
	for ( UlCnt = 0; UlCnt < 4096; UlCnt += 128 )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Set write address
		RamWrite32A( CMD_IO_DAT_ACCESS, UlCnt ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;		// for 1 byte write count
		RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

		for( UlNum = 0; UlNum < 128; UlNum++ )
		{
			RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;
			RamWrite32A( CMD_IO_DAT_ACCESS, ClFromCode[ UlNum + UlCnt ] ) ; 

			RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
			RamWrite32A( CMD_IO_DAT_ACCESS, 2) ;  				//  1 byte program each flash
		}
	}
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

//--------------------------------------------------------------------------------	
/* 4. <Main area> Verify  */
//--------------------------------------------------------------------------------
	for ( UlCnt = 0; UlCnt < 4096; UlCnt += 128 )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Set read address
		RamWrite32A( CMD_IO_DAT_ACCESS, UlCnt ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT	 ) ;	// for 128 bytes read count
		RamWrite32A( CMD_IO_DAT_ACCESS, 128 - 1 ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 1) ;  					// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;		
		for ( UlNum = 0; UlNum < 128; UlNum++ )
		{
			RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
			// Verify 
			// Check calibration data area
			if ( UlReadVal != ClFromCode[ UlNum + UlCnt ] ){
				FlashReset();								// ***** FLASH RESET *****
				return( 2 );
			}
		}
	}
//--------------------------------------------------------------------------------	
/* 5. <NVR1> Back up Parameter  */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;						// Select 1 flash

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// NVR address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;
		
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 80 bytes read count
	RamWrite32A( CMD_IO_DAT_ACCESS, 80 - 1 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 1) ;  						// Read Start
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;
	for ( UlNum = 0; UlNum < 80; UlNum++ )						// 80 bytes
	{
		RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		// 64 byte Parameters escape from erase and add Magic code. 
		if ( (UlNum <= 7) || ((UlNum >= 16 )&&(UlNum <= 31)) ){
				UlCalData[ UlNum ]  = (UlReadVal & 0xFFFFFF00) | CcMagicCode[ UlNum ] ;
		}else{
        // Do not copy NVRdata( Tester calibration data, OSC, AF, PWM )
				UlCalData[ UlNum ]  = UlReadVal;			
		}				
	}

//--------------------------------------------------------------------------------	
/* 6. <NVR1> Sector erase  */
//--------------------------------------------------------------------------------
//	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
//	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;						// Select 1 flash

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// NVR address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 4 ) ;						// Sector Erase Start

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_INT ) ;	
	for ( UlCnt = 0; UlCnt < 100; UlCnt++ )						// TimeOut 
	{
		RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( !(UlReadVal ==  0x80) ){
			break;
		}
	}

//--------------------------------------------------------------------------------	
/* 7. <NVR1> Program magic code again  */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// NVR address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT	 ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;						// for 1 byte write count

	for( UlNum = 0; UlNum < 80; UlNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;		// Set write data
		RamWrite32A( CMD_IO_DAT_ACCESS, UlCalData[ UlNum ] ) ;
		RamWrite32A( CMD_IO_ADR_ACCESS , FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS , 2) ;  					// Program Start
	}

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

//--------------------------------------------------------------------------------	
/* 8. <NVR1> Verify  */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// NVR address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;
		
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 80 - 1 ) ;					// for 80 bytes read count

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 1) ;						// Read Start

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;
	for ( UlNum = 0; UlNum < 80; UlNum++ )
	{
		RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		
		if( UlReadVal != UlCalData[ UlNum ] ){
			FlashReset();										// ***** FLASH RESET *****
			return( 3 );
		}
	}
//--------------------------------------------------------------------------------	
/* 9. Exit from boot  */
//--------------------------------------------------------------------------------
	RamWrite32A( 0xF000 , 0 ) ;									// exit from boot
	return( 0 );
}

//********************************************************************************
// Function Name 	: FlashMainMD5
// Retun Value		: NON
// Argment Value	: UINT8 * md5[16]
// Explanation		: Calculation for MD5 of Flash data Function
// History			: First edition 						
//********************************************************************************
void FlashMainMd5( UINT8 * pMD5 )
{
	UINT32 UlCnt;
	UINT32 UlNum;
	UINT32 UlReadVal;
	UINT8 UcFlaData[128][3];
    md5_context ctx;


	md5_starts( &ctx );
//--------------------------------------------------------------------------------
/* 1. <Main area> Read  */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 7 ) ;
	for ( UlCnt = 0; UlCnt < 4096; UlCnt += 128 )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, UlCnt ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT	 ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 128 - 1 ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 1) ;  					// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;		
		for (UlNum= 0 ; UlNum< 128 ; UlNum++)
		{
			RamRead32A( CMD_IO_DAT_ACCESS , &UlReadVal ) ;
//			UcFlaData[ UlNum ][0] = (UlReadVal >> 24) & 0xFF;
			UcFlaData[ UlNum ][0] = (UlReadVal >> 16) & 0xFF;
			UcFlaData[ UlNum ][1] = (UlReadVal >> 8) & 0xFF;
			UcFlaData[ UlNum ][2] = UlReadVal & 0xFF;
		}
		md5_update( &ctx, (UINT8 *)UcFlaData, UlNum * 3 );
	}

	md5_finish( &ctx, pMD5 );

}


//********************************************************************************
// Function Name 	: FlashNvrMD5
// Retun Value		: NON
// Argment Value	: UINT8 * md5[16]
// Explanation		: Calculation for MD5 of Flash data Function
// History			: First edition 						
//********************************************************************************
void FlashNvrMd5( UINT8 * pMD5 )
{
	UINT8 UcNvrData[32][2];
	UINT32 UlCnt, UlNum;
	UINT32 UlReadVal;
    md5_context ctx;


	md5_starts( &ctx );
//--------------------------------------------------------------------------------
/* 1. <NVR1> Read  */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 3 ) ;						// NVR1_1 + NVR1_2
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// Set address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 256 - 1 ) ;					// for 256 bytes read count

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;  						// Read Start

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;
	for ( UlCnt = 0; UlCnt < 256; UlCnt += 32 )
	{
		for ( UlNum = 0; UlNum < 32; UlNum++ )
		{
			RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
//			UcNvrData[ UlNum ] = UlReadVal & 0xFF ;
			UcNvrData[ UlNum ][0] = (UlReadVal >> 8) & 0xFF;		// NVR1_2
			UcNvrData[ UlNum ][1] = UlReadVal & 0xFF;				// NVR1_1
		}
		md5_update( &ctx, (UINT8 *)UcNvrData, UlNum * 2 );
	}

	md5_finish( &ctx, pMD5 );


}

//********************************************************************************
// Function Name 	: FlashWriteMd5
// Retun Value		: INT16 0:Ok, 1:CVER error, 5: WPB LOW ERROR
// Argment Value	: NON
// Explanation		: Calculation for MD5 of Flash data Function
// History			: First edition 						
//********************************************************************************
INT16 FlashWriteMd5( void )
{
	UINT32 UlCnt, UlNum;
	UINT8 UcMainMD5[16] ;
	UINT8 UcNvrMD5[16] ;
	UINT32 UlReadVal; 

//--------------------------------------------------------------------------------
/* 0. Communication check by CVER read */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, CVER_123 ) ;
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
	if( UlReadVal < 0xB3 ) {
		return ( 1 );
	}

//--------------------------------------------------------------------------------
/* 1. Flash initialize */
//--------------------------------------------------------------------------------
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

//--------------------------------------------------------------------------------
/* 2. Calculation for Main array */
//--------------------------------------------------------------------------------
	FlashMainMd5( UcMainMD5 );
//--------------------------------------------------------------------------------
/* 3. Calculation for NVR */
//--------------------------------------------------------------------------------
	FlashNvrMd5( UcNvrMD5 );

//--------------------------------------------------------------------------------
/* 4. Sector erase  */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 4 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// Set address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;

	WPBCtrl(WPB_OFF) ;											// Disable write protect
	if ( ReadWPB() != 1 )	return ( 5 );						// WPB LOW ERROR

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Disable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 4 ) ;  						// Sector Erase Start

	// wait for end of sector erase
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_INT ) ;
	for ( UlCnt = 0; UlCnt < 100; UlCnt++ )						// TimeOut 
	{
		RamRead32A( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( !(UlReadVal ==  0x80) ){
			break;
		}
	}

//--------------------------------------------------------------------------------
/* 5. Program MD5  */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// Set address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 1 byte write count
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

	// write MD5 of main array
	for( UlNum = 0; UlNum < 16; UlNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;		// Set data
		RamWrite32A( CMD_IO_DAT_ACCESS, UcMainMD5[ UlNum ] << 16 ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 2) ;  					// Program Start
	}

	// write MD5 of NVR
	for( UlNum = 0; UlNum < 16; UlNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;		// Set data
		RamWrite32A( CMD_IO_DAT_ACCESS, UcNvrMD5[ UlNum ] << 16 ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 2) ;  					// Program Start
	}

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WPB ) ;			// Enable write protect
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;
	WPBCtrl(WPB_ON) ;											// Enable write protect

//--------------------------------------------------------------------------------
/* 6. Flash reset  */
//--------------------------------------------------------------------------------
	FlashReset();												// ***** FLASH RESET *****
	
	return ( 0 );
}


//********************************************************************************
// Function Name 	: FlashCheckNvr
// Retun Value		: INT16 0:Ok, 2:NG NVR, 4:CVER error
// Argment Value	: NON
// Explanation		: Check the MD5 of Flash data Function
// History			: First edition 						
//********************************************************************************
INT16 FlashCheckNvr( void )
{
	UINT32 UlNum;
	UINT8 UcMD5[16] ;
	UINT32 UlReadVal ;
	INT16 iResult = 0;

//--------------------------------------------------------------------------------
/* 0. Communication check by CVER read */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, CVER_123 ) ;
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
	if( UlReadVal < 0xB3 ) {
		return ( 4 );
	}

//--------------------------------------------------------------------------------
/* 1. Check firmware version */
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
/* 2. Flash initialize */
//--------------------------------------------------------------------------------
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

//--------------------------------------------------------------------------------	
/* 3. Calculation for NVR */
//--------------------------------------------------------------------------------
	FlashNvrMd5( UcMD5 );

//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 4 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// Set address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010010 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 16 bytes read count
	RamWrite32A( CMD_IO_DAT_ACCESS, 16 - 1 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;		  				// Read Start
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;	
	for ( UlNum = 0; UlNum < 16; UlNum++ )
	{
		RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( UcMD5[ UlNum ] != ((UlReadVal >> 16) & 0xFF) )
		{
			iResult = 2;
//			break;
		}
	}

//--------------------------------------------------------------------------------
/* 4. Flash reset  */
//--------------------------------------------------------------------------------
	FlashReset();												// ***** FLASH RESET *****
	return ( iResult );
}

//********************************************************************************
// Function Name 	: FlashCheckMd5
// Retun Value		: INT16 0:Ok, 1:NG main array, 2:NG NVR, 4:CVER error, 5:FW version error(under 0x13)
// Argment Value	: NON
// Explanation		: Check the MD5 of Flash data Function
// History			: First edition 						
//********************************************************************************
INT16 FlashCheckMd5( void )
{
	UINT32 UlNum;
	UINT8 UcMD5[16] ;
	UINT32 UlReadVal ;
	INT16 iResult = 0;

//--------------------------------------------------------------------------------
/* 0. Communication check by CVER read */
//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, CVER_123 ) ;
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
	if( UlReadVal < 0xB3 ) {
		return ( 4 );
	}

//--------------------------------------------------------------------------------
/* 1. Check firmware version */
//--------------------------------------------------------------------------------
//	RamRead32A(  SiVerNum, &UlReadVal ) ;
//	if( (UlReadVal & 0xFF) < 0x13 ) {
//		return ( 5 );
//	}

//--------------------------------------------------------------------------------
/* 2. Flash initialize */
//--------------------------------------------------------------------------------
	FlashInitialSetting(1);										// ***** FLASH RELEASE *****

//--------------------------------------------------------------------------------
/* 3. Calculation for Main array */
//--------------------------------------------------------------------------------
	FlashMainMd5( UcMD5 );

//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 4 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// Set address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010000 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 16 bytes read count
	RamWrite32A( CMD_IO_DAT_ACCESS, 16 - 1 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ; 						// Read Start
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;	
	for (UlNum= 0 ; UlNum< 16 ; UlNum++)
	{
		RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( UcMD5[ UlNum ] != ((UlReadVal >> 16) & 0xFF) )
		{
			iResult = 1;
//			break;
		}
	}

//--------------------------------------------------------------------------------	
/* 4. Calculation for NVR */
//--------------------------------------------------------------------------------
	FlashNvrMd5( UcMD5 );

//--------------------------------------------------------------------------------
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;			// Flash selector
	RamWrite32A( CMD_IO_DAT_ACCESS, 4 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;			// Set address
	RamWrite32A( CMD_IO_DAT_ACCESS, 0x00010010 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 16 bytes read count
	RamWrite32A( CMD_IO_DAT_ACCESS, 16 - 1 ) ;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
	RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;		  				// Read Start
	
	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;	
	for ( UlNum = 0; UlNum < 16; UlNum++ )
	{
		RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		if( UcMD5[ UlNum ] != ((UlReadVal >> 16) & 0xFF) )
		{
			iResult |= 2;
//			break;
		}
	}

//--------------------------------------------------------------------------------
/* 5. Flash reset  */
//--------------------------------------------------------------------------------
	FlashReset();												// ***** FLASH RESET *****
	return ( iResult );
}

//********************************************************************************
// Function Name 	: FlashClearMd5
// Retun Value		: INT16 0:Ok, 5: WPB LOW ERROR
// Argment Value	: NON
// Explanation		: Clear the MD5 of Flash data Function
// History			: First edition 						
//********************************************************************************
INT16 FlashClearMd5( void )
{
	return ( FlashNVRSectorErase_Byte( 512 ) );					// 0x0200
}


//********************************************************************************
// Function Name 	: MakeMD5
// Retun Value		: NON
// Argment Value	: UINT8 * data[], UINT16 len, UINT8 * md5[16]
// Explanation		: Calculation for MD5 of data Function
// History			: First edition 						
//********************************************************************************
void MakeMd5( UINT8 * pDATA, UINT16 len, UINT8 * pMD5 )
{
    md5_context ctx;

	md5_starts( &ctx );
	md5_update( &ctx, (UINT8 *)pDATA, len );
	md5_finish( &ctx, pMD5 );

}

//********************************************************************************
// Function Name 	: WriteMagicCode
// Retun Value		: NON
// Argment Value	: mode -1:remove 0:inquiry 1:make
// Explanation		: Write Magic Code Command
// History			: First edition 						
//********************************************************************************
INT16 WriteMagicCode( INT16 mode )
{
	UINT8 Disable[ 8 ] = {0,0,0,0,0,0,0,0};
	UINT8 Enable[ 8 ] = {0,1,2,3,4,5,6,7};

	if( mode < 0 ) {
		// Magic code disable
		FlashNVR_WriteData_Byte( 0, Disable, 8 );
	}
	else if ( mode > 0 ) {
		// Magic code enable
		FlashNVR_WriteData_Byte( 0, Enable, 8 );
	}
	else {
                INT16 i = 0;
		// Magic code check
		FlashNVR_ReadData_Byte( 0, Disable, 8 );
		for( i = 0; i < 8; i++ ) {
			if( Enable[i] != Disable[i] )
				return -1;
		}
		return 1;
	}
	return 0;
}

//********************************************************************************
// Function Name 	: GetFromVer
// Retun Value		: NON
// Argment Value	: VERNUM
// Explanation		: Read From code version Command
// History			: First edition 						
//********************************************************************************
UINT32 GetFromVer( )
{
	return VERNUM;
}


UINT16 MakeMainSel( UINT16 UsAddress )
{
	// 0x0000 ~ 0x0FFF -> SEL 1
	// 0x1000 ~ 0x1FFF -> SEL 2
	// 0x2000 ~ 0x2FFF -> SEL 4
	return 1 << ((UsAddress >> 12) & 0x03);
}

UINT16 MakeNVRSel( UINT16 UsAddress )
{
	// 0x0000 ~ 0x00FF -> SEL 1
	// 0x0100 ~ 0x01FF -> SEL 2
	// 0x0200 ~ 0x02FF -> SEL 4
	return 1 << ((UsAddress >> 8) & 0x03);
}

UINT32 MakeNVRDat( UINT16 UsAddress, UINT8 UcData )
{
	// 0x0000 ~ 0x00FF -> 00 00 00 xx
	// 0x0100 ~ 0x01FF -> 00 00 xx 00
	// 0x0200 ~ 0x02FF -> 00 xx 00 00
	return (UINT32)UcData << (((UsAddress >> 8) & 0x03) * 8);
}

UINT8 MakeDatNVR( UINT16 UsAddress, UINT32 UlData )
{
	return (UINT8)((UlData >> (((UsAddress >> 8) & 0x03) * 8)) & 0xFF);
}

UINT32 MakeMainDat( UINT16 UsAddress, UINT8 UcData )
{
	// 0x0000 ~ 0x0FFF -> 00 00 00 xx
	// 0x1000 ~ 0x1FFF -> 00 00 xx 00
	// 0x2000 ~ 0x2FFF -> 00 xx 00 00
	return (UINT32)UcData << (((UsAddress >> 12) & 0x03) * 8);
}

UINT8 MakeDatMain( UINT16 UsAddress, UINT32 UlData )
{
	return (UINT8)((UlData >> (((UsAddress >> 12) & 0x03) * 8)) & 0xFF);
}

void ReadBytes( UINT16 UsAddress, UINT8 * pBuff, UINT16 UsLength )
{
	UINT16 UsNum;
	UINT32 UlReadVal;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 1 byte read count
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

	for( UsNum = 0; UsNum < UsLength; UsNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Set Address
		RamWrite32A( CMD_IO_DAT_ACCESS, (UsAddress + UsNum) & 0x0FFF ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS, MakeMainSel(UsAddress + UsNum) ) ;		

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 1 ) ;  					// Read Start

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_RDAT ) ;		
		RamRead32A ( CMD_IO_DAT_ACCESS, &UlReadVal ) ;
		
		pBuff[UsNum] = MakeDatMain((UsAddress + UsNum), UlReadVal) ;
	}
}

void WriteBytes( UINT16 UsAddress, UINT8 * pBuff, UINT16 UsLength )
{
	UINT16 UsNum;

	RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ACSCNT ) ;			// for 1 byte write count
	RamWrite32A( CMD_IO_DAT_ACCESS, 0 ) ;

	for( UsNum = 0; UsNum < UsLength; UsNum++ )
	{
		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_ADR ) ;		// Set Address
		RamWrite32A( CMD_IO_DAT_ACCESS, (UsAddress + UsNum) & 0x0FFF ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_SEL ) ;		// Flash selector
		RamWrite32A( CMD_IO_DAT_ACCESS, MakeMainSel(UsAddress + UsNum) ) ;		

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_WDAT ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, MakeMainDat((UsAddress + UsNum), pBuff[ UsNum ]) ) ;

		RamWrite32A( CMD_IO_ADR_ACCESS, FLASHROM_CMD ) ;
		RamWrite32A( CMD_IO_DAT_ACCESS, 2) ;  					// Program Start
	}
}
