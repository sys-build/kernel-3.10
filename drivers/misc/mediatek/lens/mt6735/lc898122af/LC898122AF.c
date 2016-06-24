/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "LC898122AF.h"
#include "../camera/kd_camera_hw.h"
#include "Ois.h"
#include "ois_otp_def.h"
//#include "OisDef.h"
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif


#define LENS_I2C_BUSNUM 0
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("LC898122AF", 0x7C)};


#define LC898123AF_DRVNAME "LC898122AF"
#define LC898123AF_VCM_WRITE_ID           0x7C

#define LC898123AF_DEBUG
#ifdef LC898123AF_DEBUG
#define LC898123AFDB printk
#else
#define LC898123AFDB(x,...)
#endif

static spinlock_t g_LC898123AF_SpinLock;

static struct i2c_client * g_pstLC898123AF_I2Cclient = NULL;

static dev_t g_LC898123AF_devno;
static struct cdev * g_pLC898123AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4LC898123AF_Opened = 0;
static int g_i4MotorStatus = 0;
static int g_i4Dir = 0;
static unsigned int g_u4LC898123AF_INF = 0;
static unsigned int g_u4LC898123AF_MACRO = 1023;
static unsigned int g_u4TargetPosition = 0;
static unsigned int g_u4CurrPosition   = 0;
static unsigned int g_u4InitPosition   = 100;

static int g_sr = 3;

void RegWriteA(unsigned short RegAddr, unsigned char RegData)
{
    int  i4RetValue = 0;
    char puSendCmd[3] = {(char)((RegAddr>>16)&0xFFFF),(char)(RegAddr&0xFFFF),RegData};
    //LC898123AFDB("[LC898123AF]I2C w (%x %x) \n",RegAddr,RegData);

    g_pstLC898123AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    g_pstLC898123AF_I2Cclient->addr = (LC898123AF_VCM_WRITE_ID >> 1);
    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, puSendCmd, 3);
    if (i4RetValue < 0) 
    {
        LC898123AFDB("[LC898123AF]I2C send failed!! \n");
        return;
    }
}
void RegReadA(unsigned short RegAddr, unsigned char *RegData)
{
    int  i4RetValue = 0;
    char pBuff[2] = {(char)(RegAddr >> 16) , (char)(RegAddr & 0xFFFF)};

    g_pstLC898123AF_I2Cclient->addr = (LC898123AF_VCM_WRITE_ID >> 1);

    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, pBuff, 2);
    if (i4RetValue < 0 ) 
    {
        LC898123AFDB("[CAMERA SENSOR] read I2C send failed!!\n");
        return;
    }

    i4RetValue = i2c_master_recv(g_pstLC898123AF_I2Cclient, (u8*)RegData, 1);

    //LC898123AFDB("[LC898123AF]I2C r (%x %x) \n",RegAddr,*RegData);
    if (i4RetValue != 1) 
    {
        LC898123AFDB("[CAMERA SENSOR] I2C read failed!! \n");
        return;
    }
}
void RamWriteA( unsigned short RamAddr, unsigned short RamData )

{
    int  i4RetValue = 0;
    char puSendCmd[4] = {(char)((RamAddr >>  16)&0xFFFF), 
                         (char)( RamAddr       &0xFFFF),
                         (char)((RamData >>  8)&0xFFFF), 
                         (char)( RamData       &0xFFFF)};
    //LC898123AFDB("[LC898123AF]I2C w2 (%x %x) \n",RamAddr,RamData);

    g_pstLC898123AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    g_pstLC898123AF_I2Cclient->addr = (LC898123AF_VCM_WRITE_ID >> 1);
    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, puSendCmd, 4);
    if (i4RetValue < 0) 
    {
        LC898123AFDB("[LC898123AF]I2C send failed!! \n");
        return;
    }
}
void RamReadA( unsigned short RamAddr, void * ReadData )
{
    int  i4RetValue = 0;
    char pBuff[2] = {(char)(RamAddr >> 16) , (char)(RamAddr & 0xFFFF)};
    unsigned short  vRcvBuff=0;
	unsigned int *pRcvBuff;
    pRcvBuff =(unsigned int *)ReadData;

    g_pstLC898123AF_I2Cclient->addr = (LC898123AF_VCM_WRITE_ID >> 1);

    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, pBuff, 2);
    if (i4RetValue < 0 ) 
    {
        LC898123AFDB("[CAMERA SENSOR] read I2C send failed!!\n");
        return;
    }

    i4RetValue = i2c_master_recv(g_pstLC898123AF_I2Cclient, (u8*)&vRcvBuff, 2);
    if (i4RetValue != 2) 
    {
        LC898123AFDB("[CAMERA SENSOR] I2C read failed!! \n");
        return;
    }
    *pRcvBuff=    ((vRcvBuff&0xFFFF) <<16) + ((vRcvBuff>> 16)&0xFFFF) ;
    
    //LC898123AFDB("[LC898123AF]I2C r2 (%x %x) \n",RamAddr,(unsigned int)*pRcvBuff);

}
void RamWrite32A(unsigned short RamAddr, unsigned int RamData )
{
    int  i4RetValue = 0;
    char puSendCmd[6] = {(char)((RamAddr >>  8)&0xFF), 
                         (char)( RamAddr       &0xFF),
                         (char)((RamData >>  24)&0xFF), 
                         (char)((RamData >>  16)&0xFF), 
                         (char)((RamData >>  8)&0xFF), 
                         (char)( RamData       &0xFF)};
    int read_data;
    //LC898123AFDB("[LC898123AF]I2C w4 (%x %x) \n",RamAddr,(unsigned int)RamData);
    //printk("%s: puSendCmd[2-5] = 0x%x, 0x%x, 0x%x, 0x%x \n", __func__, 
		//puSendCmd[2], puSendCmd[3], puSendCmd[4], puSendCmd[5]);
    
    g_pstLC898123AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    g_pstLC898123AF_I2Cclient->addr = (LC898123AF_VCM_WRITE_ID >> 1);
    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, puSendCmd, 6);
    if (i4RetValue < 0) 
    {
        LC898123AFDB("[LC898123AF]I2C send failed!! i4RetValue=%d\n", i4RetValue);
        return;
    }
   
}
void RamRead32A(unsigned short RamAddr, void * ReadData )
{
    int  i4RetValue = 0;
    char pBuff[2] = {(char)(RamAddr >> 8) , (char)(RamAddr & 0xFF)};
    unsigned int *pRcvBuff, vRcvBuff=0;
    pRcvBuff =(unsigned int *)ReadData;

    g_pstLC898123AF_I2Cclient->addr = (LC898123AF_VCM_WRITE_ID >> 1);

    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, pBuff, 2);
    if (i4RetValue < 0 ) 
    {
        LC898123AFDB("[CAMERA SENSOR] read I2C send failed!!\n");
        return;
    }

    i4RetValue = i2c_master_recv(g_pstLC898123AF_I2Cclient, (u8*)&vRcvBuff, 4);
    if (i4RetValue != 4) 
    {
        LC898123AFDB("[CAMERA SENSOR] I2C read failed!! \n");
        return;
    }
    *pRcvBuff=   ((vRcvBuff     &0xFF) <<24) 
               +(((vRcvBuff>> 8)&0xFF) <<16) 
               +(((vRcvBuff>>16)&0xFF) << 8) 
               +(((vRcvBuff>>24)&0xFF)     );

        //LC898123AFDB("[LC898123AF]I2C r4 (%x %x) \n",RamAddr,(unsigned int)*pRcvBuff);
}
void WitTim(unsigned short  UsWitTim )
{
    msleep(UsWitTim);
}
void LC898prtvalue(unsigned short  prtvalue )
{
    LC898123AFDB("[LC898123AF]printvalue ======%x   \n",prtvalue);
}

static unsigned char s4LC898OTP_ReadReg(unsigned short RegAddr)
{ 
    int  i4RetValue = 0;
    unsigned char pBuff = (unsigned char)RegAddr;
    unsigned char RegData=0xFF;

    g_pstLC898123AF_I2Cclient->addr = (0xA0 >> 1);
    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, &pBuff, 1);
    if (i4RetValue < 0 ) 
    {
        LC898123AFDB("[CAMERA SENSOR] read I2C send failed!!\n");
        return 0xff;
    }

    i4RetValue = i2c_master_recv(g_pstLC898123AF_I2Cclient, &RegData, 1);

    LC898123AFDB("[LC898123AF]OTPI2C r (%x %x) \n",RegAddr,RegData);
    if (i4RetValue != 1) 
    {
        LC898123AFDB("[CAMERA SENSOR] I2C read failed!! \n");
        return 0xff;
    }
    return RegData;

}
#if 0
static void s4LC898OTP_WriteReg(unsigned short RegAddr, unsigned char RegData)
{
    int  i4RetValue = 0;
    char puSendCmd[2] = {(unsigned char)RegAddr, RegData};
    LC898123AFDB("[LC898123AF]OTPI2C w (%x %x) \n",RegAddr,RegData);

    g_pstLC898123AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    g_pstLC898123AF_I2Cclient->addr = (0xA0 >> 1);
    i4RetValue = i2c_master_send(g_pstLC898123AF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) 
    {
        LC898123AFDB("[LC898123AF]I2C send failed!! \n");
        return;
    }
}
#endif 
inline static int getLC898123AFInfo(__user stLC898122AF_MotorInfo * pstMotorInfo)
{
    stLC898122AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4LC898123AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4LC898123AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR      = TRUE;

    if (g_i4MotorStatus == 1)    {stMotorInfo.bIsMotorMoving = 1;}
    else                        {stMotorInfo.bIsMotorMoving = 0;}

    if (g_s4LC898123AF_Opened >= 1)    {stMotorInfo.bIsMotorOpen = 1;}
    else                        {stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stLC898122AF_MotorInfo)))
    {
        LC898123AFDB("[LC898123AF] copy to user failed when getting motor information \n");
    }
    return 0;
}

void LC898123AF_init_drv(void)
{
	//OnSemi Temp
	unsigned int	UlReadVal ;
	RamWrite32A( 0xC000, 0xD000AC) ;			// Read remap flag
	RamRead32A(  0xD000, &UlReadVal ) ;
	printk("[LC898123AFMAPFLAG] 0x%x\n",(unsigned int) UlReadVal);
	RamWrite32A( 0xC000, 0xD00100) ;			// Read IC flag
	RamRead32A(  0xD000, &UlReadVal ) ;

	RamWrite32A( 0xF012, 0x01) ;	
	printk("[LC898123AFICFLAG] 0x%x\n", (unsigned int)UlReadVal);
	return;
#if 0	
    unsigned short addrotp;
    unsigned int dataotp=0;
    IniSetAf();
    IniSet();
RamAccFixMod(ON); //16bit Fix mode
    addrotp=0x30;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);    
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp, (unsigned int)dataotp);
    //RamWriteA(0x1479,dataotp);  //Hall offset X

    addrotp=0x32;    dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RamWriteA(0x14F9,dataotp);  //Hall offset Y

    addrotp=0x34;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RamWriteA(0x147A,dataotp);  //Hall bias X

    addrotp=0x36;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RamWriteA(0x14FA,dataotp);  //Hall bias Y

    addrotp=0x38;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RamWriteA(0x1450,dataotp);  //Hall AD offset X

    addrotp=0x3A;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RamWriteA(0x14D0,dataotp);  //Hall AD offset Y

    addrotp=0x3C;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RamWriteA(0x10D3,dataotp);  //Loop gain X

    addrotp=0x3E;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RamWriteA(0x11D3,dataotp);  //Loop gain Y

RamAccFixMod(OFF); //32bit Float mode
    addrotp=0x44;dataotp=s4LC898OTP_ReadReg(addrotp);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RegWriteA(0x02a0,dataotp);  //Gyro offset X M
    addrotp=0x45;dataotp=s4LC898OTP_ReadReg(addrotp);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RegWriteA(0x02a1,dataotp);  //Gyro offset X L
    addrotp=0x46;dataotp=s4LC898OTP_ReadReg(addrotp);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RegWriteA(0x02a2,dataotp);  //Gyro offset Y M
    addrotp=0x47;dataotp=s4LC898OTP_ReadReg(addrotp);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RegWriteA(0x02a3,dataotp);  //Gyro offset Y L

    addrotp=0x48;dataotp=s4LC898OTP_ReadReg(addrotp);  
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    //RegWriteA(0x0257,dataotp);//OSC

    addrotp=0x49;
    dataotp=(s4LC898OTP_ReadReg(addrotp)<<24)
            +(s4LC898OTP_ReadReg(addrotp+1)<<16)
            +(s4LC898OTP_ReadReg(addrotp+2)<<8)
            +s4LC898OTP_ReadReg(addrotp+3); 
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);    
    //RamWrite32A(0x1020,dataotp);  //Gyro gain X
    
    addrotp=0x4D;
    dataotp=(s4LC898OTP_ReadReg(addrotp)<<24)
            +(s4LC898OTP_ReadReg(addrotp+1)<<16)
            +(s4LC898OTP_ReadReg(addrotp+2)<<8)
            +s4LC898OTP_ReadReg(addrotp+3); 
    LC898123AFDB("[LC898123AFOTP]0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);    
    //RamWrite32A(0x1120,dataotp);  //Gyro gain Y

    RamWriteA(TCODEH, g_u4InitPosition); // focus position
    RtnCen(0);
    msleep(100);
    SetPanTiltMode(ON);
    msleep(10);
    OisEna();
    SetH1cMod(MOVMODE);  //movie mode
   // SetH1cMod(0);          //still mode

    addrotp=0x20;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]AF start current 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    LC898123AFDB("[LC898123AFOTP]AF start current 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    LC898123AFDB("[LC898123AFOTP]AF start current 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);

    addrotp=0x22;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]AF Infinit 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    LC898123AFDB("[LC898123AFOTP]AF Infinit 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    LC898123AFDB("[LC898123AFOTP]AF Infinit 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);

    addrotp=0x24;dataotp=(s4LC898OTP_ReadReg(addrotp)<<8)+s4LC898OTP_ReadReg(addrotp+1);  
    LC898123AFDB("[LC898123AFOTP]AF Macro 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    LC898123AFDB("[LC898123AFOTP]AF Macro 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    LC898123AFDB("[LC898123AFOTP]AF Macro 0x%x 0x%x\n", addrotp,  (unsigned int)dataotp);
    LC898123AFDB("[LC898123AF] LC898123AF_Open - End\n");
    #endif
}

#define AF_CENTER_POS   512
static unsigned short SetVCMPos(unsigned short _wData)
{
    unsigned short TargetPos;
    unsigned short ExistentPos = 0;
    int i2cret=0;

    LC898123AFDB("%s: Before cal, _wData = %d", __func__, _wData);
    if(_wData >= AF_CENTER_POS) {
        _wData -= AF_CENTER_POS;
        _wData *= 44;
        _wData += 0x300;
    }
    else {
        _wData = AF_CENTER_POS -_wData;
        _wData *= 44;
        _wData = 0xffff - _wData;
        _wData -= 0x300;
    }

    LC898123AFDB("After cal ,  _wData = 0x%x . \n", _wData );

    return _wData;
}

inline static int moveLC898123AF(unsigned int a_u4Position)
{
    int i = 0;
    unsigned char read_data[64];
    if((a_u4Position > g_u4LC898123AF_MACRO) || (a_u4Position < g_u4LC898123AF_INF))
    {
        LC898123AFDB("[LC898123AF] out of range \n");
        return -EINVAL;
    }

    printk("%s: a_u4Position = %d \n", __func__, a_u4Position);
    if (g_s4LC898123AF_Opened == 1)
    {
        //LC898123AF_init_drv();
        spin_lock(&g_LC898123AF_SpinLock);
        g_u4CurrPosition = g_u4InitPosition;
        g_s4LC898123AF_Opened = 2;
        spin_unlock(&g_LC898123AF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(&g_LC898123AF_SpinLock);    
        g_i4Dir = 1;
        spin_unlock(&g_LC898123AF_SpinLock);    
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(&g_LC898123AF_SpinLock);    
        g_i4Dir = -1;
        spin_unlock(&g_LC898123AF_SpinLock);            
    }
    else   return 0; 

    spin_lock(&g_LC898123AF_SpinLock);    
    g_u4TargetPosition = a_u4Position;
    g_sr = 3;
    g_i4MotorStatus = 0;
    spin_unlock(&g_LC898123AF_SpinLock);    
    //OnSemiTemp
	//RamWriteA(TCODEH, g_u4TargetPosition); 
    //RamWrite32A(0xF01A, SetVCMPos((unsigned short)g_u4TargetPosition) );
    RamWrite32A(0xF01A, g_u4TargetPosition);
/*
    FlashNVR_ReadData_Byte(0x0000, &read_data, 64);
    for(i = 0; i < 64; i++){
        printk("read_data[%d] = 0x%x  ", i, read_data[i]);
        if( (i+1) % 8 == 0) printk(" \n");
    }
*/

    spin_lock(&g_LC898123AF_SpinLock);        
    g_u4CurrPosition = (unsigned int)g_u4TargetPosition;
    spin_unlock(&g_LC898123AF_SpinLock);                

    return 0;
}

inline static int setLC898123AFInf(unsigned int a_u4Position)
{
    spin_lock(&g_LC898123AF_SpinLock);
    g_u4LC898123AF_INF = a_u4Position;
    spin_unlock(&g_LC898123AF_SpinLock);    
    return 0;
}

inline static int setLC898123AFMacro(unsigned int a_u4Position)
{
    spin_lock(&g_LC898123AF_SpinLock);
    g_u4LC898123AF_MACRO = a_u4Position;
    spin_unlock(&g_LC898123AF_SpinLock);    
    return 0;    
}

inline static int setLC898123OisControlRegister(int reg,unsigned long a_enable)
{
    unsigned int ret = 0;
    while(RamRead32A(0xF100, &ret), ret != 0)
        msleep(10);

    RamRead32A(reg, &ret);
    printk("%s: reg = %x read ois enable, ret = %d, flag = %ld \n", __func__,reg,ret,a_enable);
    if(ret == a_enable){
        printk("%s: return directly set ois success \n", __func__);
        return 0;
    }
    RamWrite32A(reg, a_enable);
    printk("%s: set ois success \n", __func__);
    return 0;
}

inline static int setLC898123OisEnalbe(unsigned long a_enable)
{
    unsigned int ret = 0;
    printk("%s: set ois start, flag = %ld \n", __func__, a_enable);

    while(RamRead32A(0xF100, &ret), ret != 0)
        msleep(10);

    RamRead32A(0xF012, &ret);
    printk("%s: read ois enable, ret = %d, flag = %ld \n", __func__,ret,a_enable);
    if(ret == a_enable){
        printk("%s: return directly set ois success \n", __func__);
        return 0;
    }
    RamWrite32A(0xF012, a_enable);
    printk("%s: set ois success \n", __func__);
    return 0;
}

#define VCM_PREVIEW_MODE (0)
#define VCM_CAPTURE_MODE (1)
#define VCM_VIDEO_MODE      (2)

inline static int setLC898123Mode(unsigned long mode)
{
    unsigned int ret = 0;
    printk("%s: set VCM mode to %ld \n", __func__, mode);

    switch(mode){
        case VCM_PREVIEW_MODE:
            setLC898123OisControlRegister(0xF013, 1);
            setLC898123OisControlRegister(0xF012,1);
            break;
        case VCM_CAPTURE_MODE:
            setLC898123OisControlRegister(0xF013, 1);
            setLC898123OisControlRegister(0xF012,1);
            break;
        case VCM_VIDEO_MODE:
            setLC898123OisControlRegister(0xF012,1);
            setLC898123OisControlRegister(0xF013, 0);
            break;
    }
    printk("%s: set vcm mode success \n", __func__);
    return 0;
}


////////////////////////////////////////////////////////////////
static int LC898123AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    int i4RetValue = 0;

    switch(a_u4Command)
    {
        case LC898122AFIOC_G_MOTORINFO :
            i4RetValue = getLC898123AFInfo((__user stLC898122AF_MotorInfo *)(a_u4Param));
        break;

        case LC898122AFIOC_T_MOVETO :
            i4RetValue = moveLC898123AF(a_u4Param);
        break;
 
        case LC898122AFIOC_T_SETINFPOS :
            i4RetValue = setLC898123AFInf(a_u4Param);
        break;

        case LC898122AFIOC_T_SETMACROPOS :
            i4RetValue = setLC898123AFMacro(a_u4Param);
        break;

        //HTC START
        case LC898122AFIOC_T_OIS_ENABLE:
            printk("%s: OIS command = %ld", __func__, a_u4Param);
            i4RetValue = setLC898123OisControlRegister(0xF012,a_u4Param);
        break;

        case LC898123AFIOC_T_SET_MODE:
            printk("%s: set vcm mode to %ld \n", __func__, a_u4Param);
            i4RetValue = setLC898123Mode(a_u4Param);
        break;
        //HTC END
        default :
              LC898123AFDB("[LC898123AF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

/************************ AXC workaround related code *****************************/
static char nvr_mem[NVR_BLOCK_SIZE * 2];
static void mem_print(char*data_p, int size)
{
	int i = 0;
	printk("\n--ois_nvr dump--");
	while(size--){
		if(!(i & 0x0f))
			printk("\nois_nvr 0x%03x: ", i);
		printk("%02x ", data_p[i]);
		i++;
	}
	printk("\n----------------\n");
}

static int mem_equal(char * p1, char * p2, int size)
{
	while(size--){
		if(*p1++ != *p2++)
			return 0;
	}
	return 1;
}

static int verify_ois_otp(char *p)
{
	int i = 8;
	while(i--){
		LC898123AFDB("[LC898123AF] [%d]=0x%02x.\n", i, p[i]);
		if((i) != p[i])
			return 0;
	}
	return 1;
}

static int verify_nvr1(char *p)
{
	if(p[0] == 0x02 && p[1] == 0x3f)
		return 1;
	else
		return 0;
}

//byte16~byte31
#define FW_CK_OFFSET 16
const unsigned char OTP_NVR0_FW_checksum[] = {
0x14,
0x00,
0xE0,
0x0A,
0x10,
0x06,
0x00,
0xE3,
0x34,
0x01,
0x30,
0x0B,
0xE4,
0x00,
0x80,
0x65,
};

unsigned int remapNvr(char *ois_otp_buf_p)
{
	unsigned int flag1 = 0, flag2 = 0;
	unsigned int remap;

        if(ois_otp_buf_p) {
            LC898123AFDB("[LC898123AF] nvr read\n");
            FlashNVR_ReadData_Byte(NVR0_ADDR, nvr_mem, NVR_BLOCK_SIZE * 2);
            mem_print(nvr_mem + NVR0_OFFSET, NVR_BLOCK_SIZE * 2);

            if(verify_ois_otp(nvr_mem)){
                LC898123AFDB("[LC898123AF] nvr0 data seems right.\n");
            }
            else{
                LC898123AFDB("[LC898123AF] !!!nvr data seems not right.\n");
            }
            if(verify_nvr1(nvr_mem + NVR_BLOCK_SIZE)){
                LC898123AFDB("[LC898123AF] nvr1 data seems right\n");
            }
            else{
                LC898123AFDB("[LC898123AF] !!!nvr1 data seems error\n");
            }

            if(verify_ois_otp(ois_otp_buf_p)){
                LC898123AFDB("[LC898123AF] ois_otp data seems right.\n");
                if(!mem_equal(ois_otp_buf_p + NVR0_OFFSET, nvr_mem + NVR0_OFFSET, NVR0_SIZE)){
                    LC898123AFDB("[LC898123AF] nvr0 not match\n");
                    flag1 = 1;
                    memcpy(nvr_mem + NVR0_OFFSET, ois_otp_buf_p + NVR0_OFFSET, NVR0_SIZE);
                }
                else
                    LC898123AFDB("[LC898123AF] nvr0 match!\n");

                if(!mem_equal(ois_otp_buf_p + NVR11_OFFSET, nvr_mem + NVR_BLOCK_SIZE, NVR11_SIZE)){
                    LC898123AFDB("[LC898123AF] nvr11 not match\n");
                    flag2 = 1;
                    memcpy(nvr_mem + NVR_BLOCK_SIZE, ois_otp_buf_p + NVR11_OFFSET, NVR11_SIZE);
                }
                else
                    LC898123AFDB("[LC898123AF] nvr11 match!\n");

                if(!mem_equal(ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, nvr_mem + NVR12_ADDR, NVR12_SIZE)){
                    LC898123AFDB("[LC898123AF] nvr12 not match\n");
                    flag2 = 1;
                    memcpy(nvr_mem + NVR12_ADDR, ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, NVR12_SIZE);
                }
                else
                    LC898123AFDB("[LC898123AF] nvr12 match!\n");

                if(flag1){
                    LC898123AFDB("[LC898123AF] nvr0 writing\n");
                    FlashNVRSectorErase_Byte(NVR0_ADDR);
                    FlashNVR_WriteData_Byte(NVR0_ADDR, nvr_mem + NVR0_OFFSET, NVR_BLOCK_SIZE);
                }
                if(flag2){
                    if(verify_nvr1(nvr_mem + NVR_BLOCK_SIZE)){
                        LC898123AFDB("[LC898123AF] nvr1 writing\n");
                        FlashNVRSectorErase_Byte(NVR11_ADDR);
                        FlashNVR_WriteData_Byte(NVR11_ADDR, nvr_mem + NVR_BLOCK_SIZE, NVR_BLOCK_SIZE);
                    }
                    else{
                        LC898123AFDB("[LC898123AF] otp nvr1 data error, will not write nvr1\n");
                    }
                }
                if(flag1 || flag2){
                    LC898123AFDB("[LC898123AF] nvr read after flash\n");
                    FlashNVR_ReadData_Byte(NVR0_ADDR, nvr_mem, NVR_BLOCK_SIZE * 2);

                    if(!mem_equal(ois_otp_buf_p + NVR0_OFFSET, nvr_mem + NVR0_OFFSET, NVR0_SIZE)){
                        LC898123AFDB("[LC898123AF] nvr0 not match after flash!!!!!\n");
                    }
                    else{
                        LC898123AFDB("[LC898123AF] nvr0 match\n");
                    }
                    if(!mem_equal(ois_otp_buf_p + NVR11_OFFSET, nvr_mem + NVR_BLOCK_SIZE, NVR11_SIZE)){
                        LC898123AFDB("[LC898123AF] nvr11 not match after flash!!!!\n");
                    }
                    else{
                        LC898123AFDB("[LC898123AF] nvr11 match\n");
                    }
                    if(!mem_equal(ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, nvr_mem + NVR12_ADDR, NVR12_SIZE)){
                        LC898123AFDB("[LC898123AF] nvr12 not match after flash!!!!\n");
                    }
                    else{
                        LC898123AFDB("[LC898123AF] nvr12 match\n");
                    }
                }
            }
            else
                LC898123AFDB("[LC898123AF] !!!ois_otp data seems not right, will not write to nvram.\n");
        }

        RamWrite32A( CMD_IO_ADR_ACCESS, SYS_DSP_REMAP) ;// Read remap flag
        RamRead32A(  CMD_IO_DAT_ACCESS, &remap ) ;
        LC898123AFDB("[LC898123AF] remap %d\n", remap);

        return remap;
}

int len_sysboot_check(char * ois_otp_buf_p)
{
	unsigned int ret = 0, flag1 = 0, flag2 = 0;
	unsigned int remap, UlReadVal, retry = 3;

	LC898123AFDB("[LC898123AF] sysboot check\n");
	RamWrite32A( CMD_IO_ADR_ACCESS, CVER_123) ;
	RamRead32A(  CMD_IO_DAT_ACCESS, &UlReadVal) ;
	LC898123AFDB("[LC898123AF] read chipver %x\n", UlReadVal);
	if(UlReadVal != 0xB4){
		OscStb();
		return 0;
	}

        if(ois_otp_buf_p)
                    //creat new OTP.data for FW_V000B
                    memcpy(ois_otp_buf_p + FW_CK_OFFSET, OTP_NVR0_FW_checksum, sizeof(OTP_NVR0_FW_checksum)); 

/*
        //LC898123AXC status check
        while(RamRead32A(0xF100, &ret), ret != 0)
            msleep(10);
*/
        while(retry > 0) {
                remap = remapNvr(ois_otp_buf_p);
                //Check FW Version Read 0x8000 = 0505020B 
                if(1 == remap) {
                    RamRead32A( SiVerNum, &UlReadVal );
                    LC898123AFDB("[LC898123AF] read OIS_FW_Version %x\n", UlReadVal);
                    if(0x0505020B == UlReadVal) break;
                }
                LC898123AFDB("[LC898123AF] start flash update \n");
                ret = FlashUpdate();
                LC898123AFDB("[LC898123AF] end flash update ret = %d\n", ret);
                RamWrite32A( CMD_IO_ADR_ACCESS, SYS_DSP_REMAP) ;// Read remap flag after updated flash
                RamRead32A(  CMD_IO_DAT_ACCESS, &remap ) ;
                LC898123AFDB("[LC898123AF] remap %d\n", remap);
                if(remap != 1) {
                    LC898123AFDB("[LC898123AF] remap is still not 1 after update flash\n");
                }
                msleep(100);
                retry--;
        }

        LC898123AFDB("[LC898123AF] sysboot check end, update tried =%d\n", 3-retry);
/*
	if(ois_otp_buf_p){
		LC898123AFDB("[LC898123AF] nvr read\n");
		FlashNVR_ReadData_Byte(NVR0_ADDR, nvr_mem, NVR_BLOCK_SIZE * 2);
		mem_print(nvr_mem + NVR0_OFFSET, NVR_BLOCK_SIZE * 2);

		if(verify_ois_otp(nvr_mem)){
			LC898123AFDB("[LC898123AF] nvr0 data seems right.\n");
		}
		else{
			LC898123AFDB("[LC898123AF] !!!nvr data seems not right.\n");
		}
		if(verify_nvr1(nvr_mem + NVR_BLOCK_SIZE)){
			LC898123AFDB("[LC898123AF] nvr1 data seems right\n");
		}
		else{
			LC898123AFDB("[LC898123AF] !!!nvr1 data seems error\n");
		}

		if(verify_ois_otp(ois_otp_buf_p)){
			LC898123AFDB("[LC898123AF] ois_otp data seems right.\n");
			if(!mem_equal(ois_otp_buf_p + NVR0_OFFSET, nvr_mem + NVR0_OFFSET, NVR0_SIZE)){
				LC898123AFDB("[LC898123AF] nvr0 not match\n");
				flag1 = 1;
				memcpy(nvr_mem + NVR0_OFFSET, ois_otp_buf_p + NVR0_OFFSET, NVR0_SIZE);
			}
			else
				LC898123AFDB("[LC898123AF] nvr0 match!\n");

			if(!mem_equal(ois_otp_buf_p + NVR11_OFFSET, nvr_mem + NVR_BLOCK_SIZE, NVR11_SIZE)){
				LC898123AFDB("[LC898123AF] nvr11 not match\n");
				flag2 = 1;
				memcpy(nvr_mem + NVR_BLOCK_SIZE, ois_otp_buf_p + NVR11_OFFSET, NVR11_SIZE);
			}
			else
				LC898123AFDB("[LC898123AF] nvr11 match!\n");

			if(!mem_equal(ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, nvr_mem + NVR12_ADDR, NVR12_SIZE)){
				LC898123AFDB("[LC898123AF] nvr12 not match\n");
				flag2 = 1;
				memcpy(nvr_mem + NVR12_ADDR, ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, NVR12_SIZE);
			}
			else
				LC898123AFDB("[LC898123AF] nvr12 match!\n");

			if(flag1){
				LC898123AFDB("[LC898123AF] nvr0 writing\n");
				FlashNVRSectorErase_Byte(NVR0_ADDR);
				FlashNVR_WriteData_Byte(NVR0_ADDR, nvr_mem + NVR0_OFFSET, NVR_BLOCK_SIZE);
			}
			if(flag2){
				if(verify_nvr1(nvr_mem + NVR_BLOCK_SIZE)){
					LC898123AFDB("[LC898123AF] nvr1 writing\n");
					FlashNVRSectorErase_Byte(NVR11_ADDR);
					FlashNVR_WriteData_Byte(NVR11_ADDR, nvr_mem + NVR_BLOCK_SIZE, NVR_BLOCK_SIZE);
				}
				else{
					LC898123AFDB("[LC898123AF] otp nvr1 data error, will not write nvr1\n");
				}
			}
			if(flag1 || flag2){
				LC898123AFDB("[LC898123AF] nvr read after flash\n");
				FlashNVR_ReadData_Byte(NVR0_ADDR, nvr_mem, NVR_BLOCK_SIZE * 2);

				if(!mem_equal(ois_otp_buf_p + NVR0_OFFSET, nvr_mem + NVR0_OFFSET, NVR0_SIZE)){
					LC898123AFDB("[LC898123AF] nvr0 not match after flash!!!!!\n");
				}
				else{
					LC898123AFDB("[LC898123AF] nvr0 match\n");
				}
				if(!mem_equal(ois_otp_buf_p + NVR11_OFFSET, nvr_mem + NVR_BLOCK_SIZE, NVR11_SIZE)){
					LC898123AFDB("[LC898123AF] nvr11 not match after flash!!!!\n");
				}
				else{
					LC898123AFDB("[LC898123AF] nvr11 match\n");
				}
				if(!mem_equal(ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, nvr_mem + NVR12_ADDR, NVR12_SIZE)){
					LC898123AFDB("[LC898123AF] nvr12 not match after flash!!!!\n");
				}
				else{
					LC898123AFDB("[LC898123AF] nvr12 match\n");
				}
			}
		}
		else
			LC898123AFDB("[LC898123AF] !!!ois_otp data seems not right, will not write to nvram.\n");
#if 0
		mem_print(nvr_mem + NVR0_OFFSET, NVR0_SIZE);
		if(!mem_equal(ois_otp_buf_p + NVR0_OFFSET, nvr_mem + NVR0_OFFSET, NVR0_SIZE)){
			FlashNVR_WriteData_Byte(NVR0_ADDR, ois_otp_buf_p + NVR0_OFFSET, NVR0_SIZE);
			LC898123AFDB("[LC898123AF] nvr0 read again after write\n");
			FlashNVR_ReadData_Byte(NVR0_ADDR, nvr_mem + NVR0_OFFSET, NVR0_SIZE);
			mem_print(nvr_mem + NVR0_OFFSET, NVR0_SIZE);
			if(!mem_equal(ois_otp_buf_p + NVR0_OFFSET, nvr_mem + NVR0_OFFSET, NVR0_SIZE)){
				LC898123AFDB("[LC898123AF] nvr0 verify error again after write!!!\n");
			}
		}

		//nvr11 read and verify
		LC898123AFDB("[LC898123AF] nvr11 read\n");
		FlashNVR_ReadData_Byte(NVR11_ADDR, nvr_mem + NVR11_OFFSET, NVR11_SIZE);
		mem_print(nvr_mem + NVR11_OFFSET, NVR11_SIZE);
		if(!mem_equal(ois_otp_buf_p + NVR11_OFFSET, nvr_mem + NVR11_OFFSET, NVR11_SIZE)){
			FlashNVR_WriteData_Byte(NVR11_ADDR, ois_otp_buf_p + NVR11_OFFSET, NVR11_SIZE);
			LC898123AFDB("[LC898123AF] nvr11 read again after write\n");
			FlashNVR_ReadData_Byte(NVR11_ADDR, nvr_mem + NVR11_OFFSET, NVR11_SIZE);
			mem_print(nvr_mem + NVR11_OFFSET, NVR11_SIZE);
			if(!mem_equal(ois_otp_buf_p + NVR11_OFFSET, nvr_mem + NVR11_OFFSET, NVR11_SIZE)){
				LC898123AFDB("[LC898123AF] nvr11 verfity error again after write!!!\n");
			}
		}

		//nvr22 read and verify
		LC898123AFDB("[LC898123AF] nvr12 read\n");
		FlashNVR_ReadData_Byte(NVR12_ADDR, nvr_mem + NVR12_OFFSET, NVR12_SIZE);
		mem_print(nvr_mem + NVR12_OFFSET, NVR12_SIZE);
		if(!mem_equal(ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, nvr_mem + NVR12_OFFSET, NVR12_SIZE)){
			FlashNVR_WriteData_Byte(NVR12_ADDR, ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, NVR11_SIZE);
			LC898123AFDB("[LC898123AF] nvr12 read again after write\n");
			FlashNVR_ReadData_Byte(NVR12_ADDR, nvr_mem + NVR12_OFFSET, NVR12_SIZE);
			mem_print(nvr_mem + NVR12_OFFSET, NVR12_SIZE);
			if(!mem_equal(ois_otp_buf_p + NVR1_COORDINATE_ETC_DATA_OFFSET, nvr_mem + NVR12_OFFSET, NVR12_SIZE)){
				LC898123AFDB("[LC898123AF] nvr12 verfity error again after write!!!\n");
			}
		}
#endif

	}
#if 0
	if(remap != 1) {
		printk("%s: remap = %d, start flash update \n", __func__, remap);
		while(ret = FlashUpdate(), ret != 1){
			printk("%s: first call FlashUpdate ret = %d \n", __func__, ret);
			msleep(100);
		}
		msleep(200);
		while(ret = FlashUpdate(),  ret != 0 ){
			printk("%s: second call FlashUpdate ret = %d \n", __func__, ret);
			msleep(100);
		}
	}
#endif
	//ret = FlashUpdate();
	//RamRead32A( SiVerNum, &UlReadVal );
	//LC898123AFDB("[LC898123AF] read OIS_FW_Version %x\n", UlReadVal);
	RamWrite32A( CMD_IO_ADR_ACCESS, SYS_DSP_REMAP) ;// Read remap flag
	RamRead32A(  CMD_IO_DAT_ACCESS, &remap ) ;
	LC898123AFDB("[LC898123AF] remap %d\n", remap);

	if(remap != 1) {
		printk("%s: remap = %d, start flash update \n", __func__, remap);
		ret = FlashUpdate();
		printk("%s: end flash update ret = %d\n", __func__, ret);
		RamWrite32A( CMD_IO_ADR_ACCESS, SYS_DSP_REMAP) ;// Read remap flag after updated flash
		RamRead32A(  CMD_IO_DAT_ACCESS, &remap ) ;
		LC898123AFDB("[LC898123AF] remap %d\n", remap);
		if(remap != 1) {
			printk("remap is still not 1 after update flash\n");
		}
	}
	LC898123AFDB("[LC898123AF] sysboot check end\n");
    */
	return 0;
}
/************************ AXC workaround related code end *****************************/

//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int LC898123AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    unsigned int  ret;
    unsigned int remap;
    LC898123AFDB("[LC898123AF] LC898123AF_Open - Start\n");
    if(g_s4LC898123AF_Opened)
    {    
        LC898123AFDB("[LC898123AF] the device is opened \n");
        return -EBUSY;
    }
	
	spin_lock(&g_LC898123AF_SpinLock);
    g_s4LC898123AF_Opened = 1;
    spin_unlock(&g_LC898123AF_SpinLock);
    //LC898123AF_init_drv();
	//RamWrite32A( CMD_IO_ADR_ACCESS, SYS_DSP_REMAP) ;			// Read remap flag
	//RamRead32A(  CMD_IO_DAT_ACCESS, &remap ) ;
    #if 0
    if(remap != 1) {
           printk("%s: remap = %d, start flash update \n", __func__, remap);
	    while(ret = FlashUpdate(), ret != 1){
	        printk("%s: first call FlashUpdate ret = %d \n", __func__, ret);
	        msleep(100);
	    }
           msleep(200);
	    while(ret = FlashUpdate(),  ret != 0 ){
	        printk("%s: second call FlashUpdate ret = %d \n", __func__, ret);
	        msleep(100);
	    }
    }
    #endif
    #if 0
    if(remap != 1) {
        printk("%s: remap = %d, start flash update \n", __func__, remap);
        ret = FlashUpdate();
        printk("%s: end flash update ret = %d \n", __func__, ret);
        msleep(100);
    }
    #endif
    while(RamRead32A( 0xF100, &ret), ret != 0){
        msleep(10);
        LC898123AFDB("[LC898123AF] status not ready, wait \n");
    }
    setLC898123OisControlRegister(0xF012, 0x03) ;

    LC898123AFDB("[LC898123AF] LC898123AF_Open - End\n");
    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int LC898123AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    LC898123AFDB("[LC898123AF] LC898123AF_Release - Start\n");
	//OnSemi Temp
    if (g_s4LC898123AF_Opened == 2)
    {
        g_sr = 5;
		//RamWriteA(TCODEH, 100); // focus position
		RamWrite32A(0xF01A,100);
        msleep(10);
		//RamWriteA(TCODEH, 50); // focus position
		RamWrite32A(0xF01A,50);
        msleep(10);    

        RamWrite32A(0xF012,0x0);//OIS Disable
	//RamWrite32A( 0xF010 , 0x00000000 ) ;//Servo Off
        //RtnCen(0);
        //SrvCon(X_DIR,OFF);
        //SrvCon(Y_DIR,OFF);        
    }

    if (g_s4LC898123AF_Opened)
    {
        LC898123AFDB("[LC898123AF] feee \n");
                                            
        spin_lock(&g_LC898123AF_SpinLock);
        g_s4LC898123AF_Opened = 0;
        spin_unlock(&g_LC898123AF_SpinLock);

    }
    LC898123AFDB("[LC898123AF] LC898123AF_Release - End\n");

    return 0;
}

static const struct file_operations g_stLC898123AF_fops = 
{
    .owner = THIS_MODULE,
    .open = LC898123AF_Open,
    .release = LC898123AF_Release,
    .unlocked_ioctl = LC898123AF_Ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = LC898123AF_Ioctl,
#endif
};

inline static int Register_LC898123AF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    LC898123AFDB("[LC898123AF] Register_LC898123AF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_LC898123AF_devno, 0, 1,LC898123AF_DRVNAME) )
    {
        LC898123AFDB("[LC898123AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pLC898123AF_CharDrv = cdev_alloc();

    if(NULL == g_pLC898123AF_CharDrv)
    {
        unregister_chrdev_region(g_LC898123AF_devno, 1);

        LC898123AFDB("[LC898123AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pLC898123AF_CharDrv, &g_stLC898123AF_fops);

    g_pLC898123AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pLC898123AF_CharDrv, g_LC898123AF_devno, 1))
    {
        LC898123AFDB("[LC898123AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_LC898123AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv_OIS");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        LC898123AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_LC898123AF_devno, NULL, LC898123AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    LC898123AFDB("[LC898123AF] Register_LC898123AF_CharDrv - End\n");    
    return 0;
}

inline static void Unregister_LC898123AF_CharDrv(void)
{
    LC898123AFDB("[LC898123AF] Unregister_LC898123AF_CharDrv - Start\n");

    //Release char driver
    cdev_del(g_pLC898123AF_CharDrv);

    unregister_chrdev_region(g_LC898123AF_devno, 1);
    
    device_destroy(actuator_class, g_LC898123AF_devno);

    class_destroy(actuator_class);

    LC898123AFDB("[LC898123AF] Unregister_LC898123AF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////

static int LC898123AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int LC898123AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id LC898123AF_i2c_id[] = {{LC898123AF_DRVNAME,0},{}};   
struct i2c_driver LC898123AF_i2c_driver = {                       
    .probe = LC898123AF_i2c_probe,                                   
    .remove = LC898123AF_i2c_remove,                           
    .driver.name = LC898123AF_DRVNAME,                 
    .id_table = LC898123AF_i2c_id,                             
};  

#if 0 
static int LC898123AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, LC898123AF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int LC898123AF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int LC898123AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    LC898123AFDB("[LC898123AF] LC898123AF_i2c_probe\n");

    /* Kirby: add new-style driver { */
    g_pstLC898123AF_I2Cclient = client;
    
    g_pstLC898123AF_I2Cclient->addr = g_pstLC898123AF_I2Cclient->addr >> 1;
    
    //Register char driver
    i4RetValue = Register_LC898123AF_CharDrv();

    if(i4RetValue){

        LC898123AFDB("[LC898123AF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_LC898123AF_SpinLock);

    LC898123AFDB("[LC898123AF] Attached!! \n");

    return 0;
}

static int LC898123AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&LC898123AF_i2c_driver);
}

static int LC898123AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&LC898123AF_i2c_driver);
    return 0;
}

static int LC898123AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int LC898123AF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stLC898123AF_Driver = {
    .probe        = LC898123AF_probe,
    .remove    = LC898123AF_remove,
    .suspend    = LC898123AF_suspend,
    .resume    = LC898123AF_resume,
    .driver        = {
        .name    = "lens_actuator_ois",
        .owner    = THIS_MODULE,
    }
};
static struct platform_device g_stAF_device = {
    .name = "lens_actuator_ois",
    .id = 0,
    .dev = {}
};

static int __init LC898123AF_i2C_init(void)
{
    i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);

    if(platform_device_register(&g_stAF_device)){
        LC898123AFDB("failed to register AF driver\n");
        return -ENODEV;
    }

    if(platform_driver_register(&g_stLC898123AF_Driver)){
        LC898123AFDB("failed to register LC898123AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit LC898123AF_i2C_exit(void)
{
    platform_driver_unregister(&g_stLC898123AF_Driver);
}

module_init(LC898123AF_i2C_init);
module_exit(LC898123AF_i2C_exit);

MODULE_DESCRIPTION("LC898123AF lens module driver");
MODULE_AUTHOR("KY Chen <vend_james-cc.wu@Mediatek.com>");
MODULE_LICENSE("GPL");



