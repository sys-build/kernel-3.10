#ifndef _CUST_BATTERY_METER_TABLE_H
#define _CUST_BATTERY_METER_TABLE_H
#include "battery_param.h"


#include <mach/mt_typedefs.h>

// ============================================================
// define
// ============================================================
#define BAT_NTC_10 0
#define BAT_NTC_47 0
#define BAT_NTC_100 1

#if (BAT_NTC_10 == 1)
#define RBAT_PULL_UP_R             24000
#endif

#if (BAT_NTC_47 == 1)
#define RBAT_PULL_UP_R             61900
#endif

#if (BAT_NTC_100 == 1)
#define RBAT_PULL_UP_R             100000
#define NTC_TABLE_SIZE  101
#define ZCV_TABLE_SIZE  83
#endif

#define RBAT_PULL_UP_VOLT          1800
#define BATTERY_ID_CHANNEL_NUM 12
#define MTK_GET_BATTERY_ID_BY_AUXADC
#define MTK_MULTI_BAT_PROFILE_SUPPORT

kal_int32 g_battery_id_voltage[] = {250, 400, -1};//0~0.3V for battery 0, 0.3~0.52V for battery 1, -1 for the last one (battery 2)
#define TOTAL_BATTERY_NUMBER (sizeof(g_battery_id_voltage) / sizeof(kal_int32))

/* Qmax for battery  */
kal_int32 g_Q_MAX_POS_50[] = {2850, 2825, 2825};
kal_int32 g_Q_MAX_POS_25[] = {2716, 2716, 2716};
kal_int32 g_Q_MAX_POS_0[] = {2700, 2847, 2847};
kal_int32 g_Q_MAX_NEG_10[] = {1200, 1200, 1200};
kal_int32 g_Q_MAX_POS_50_H_CURRENT[] = {2850, 2825, 2825};
kal_int32 g_Q_MAX_POS_25_H_CURRENT[] = {2716, 2716, 2716};
kal_int32 g_Q_MAX_POS_0_H_CURRENT[] = {2700, 2847, 2847};
kal_int32 g_Q_MAX_NEG_10_H_CURRENT[] = {1200, 1200, 1200};


// ============================================================
// ENUM
// ============================================================

// ============================================================
// structure
// ============================================================

// ============================================================
// typedef
// ============================================================
typedef struct _BATTERY_PROFILE_STRUC
{
    kal_int32 percentage;
    kal_int32 voltage;
} BATTERY_PROFILE_STRUC, *BATTERY_PROFILE_STRUC_P;

typedef struct _R_PROFILE_STRUC
{
    kal_int32 resistance; // Ohm
    kal_int32 voltage;
} R_PROFILE_STRUC, *R_PROFILE_STRUC_P;

typedef enum
{
    T1_0C,
    T2_25C,
    T3_50C
} PROFILE_TEMPERATURE;

// ============================================================
// External Variables
// ============================================================

// ============================================================
// External function
// ============================================================

// ============================================================
// <DOD, Battery_Voltage> Table
// ============================================================
#if (BAT_NTC_100 == 1)
BATT_TEMPERATURE Batt_Temperature_Table[TOTAL_BATTERY_NUMBER][101] = {
        Batt100_Temperature_Table,
        Batt100_Temperature_Table,
        Batt100_Temperature_Table
    };
#endif

#if (BAT_NTC_47== 1)
    BATT_TEMPERATURE Batt_Temperature_Table[TOTAL_BATTERY_NUMBER][17] = {
         Batt47_Temperature_Table,
         Batt47_Temperature_Table,
         Batt47_Temperature_Table
    };
#endif

#if (BAT_NTC_10== 1)
    BATT_TEMPERATURE Batt_Temperature_Table[TOTAL_BATTERY_NUMBER][17] = {
         Batt10_Temperature_Table,
         Batt10_Temperature_Table,
         Batt10_Temperature_Table
    };
#endif
// T0 -10C
BATTERY_PROFILE_STRUC battery_profile_t0[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_profile_t0,
	battery1_profile_t0,
	battery0_profile_t0
};
// T1 0C 
BATTERY_PROFILE_STRUC battery_profile_t1[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_profile_t1,
	battery1_profile_t1,
	battery0_profile_t1
};
// T2 25C
BATTERY_PROFILE_STRUC battery_profile_t2[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_profile_t2,
	battery1_profile_t2,
	battery0_profile_t2
};
// T3 50C
BATTERY_PROFILE_STRUC battery_profile_t3[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_profile_t3,
	battery1_profile_t3,
	battery0_profile_t3
};
// battery profile for actual temperature. The size should be the same as T1, T2 and T3
BATTERY_PROFILE_STRUC battery_profile_temperature[] =VALUE_83_ZERO;

// ============================================================
// <Rbat, Battery_Voltage> Table
// ============================================================
// T0 -10C
R_PROFILE_STRUC r_profile_t0[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_r_profile_t0,
	battery1_r_profile_t0,
	battery0_r_profile_t0
};
// T1 0C
R_PROFILE_STRUC r_profile_t1[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_r_profile_t1,
	battery1_r_profile_t1,
	battery0_r_profile_t1
};
// T2 25C
R_PROFILE_STRUC r_profile_t2[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_r_profile_t2,
	battery1_r_profile_t2,
	battery0_r_profile_t2
};
// T3 50C
R_PROFILE_STRUC r_profile_t3[TOTAL_BATTERY_NUMBER][83] =
{
	battery0_r_profile_t3,
	battery1_r_profile_t3,
	battery0_r_profile_t3
};

// r-table profile for actual temperature. The size should be the same as T1, T2 and T3
R_PROFILE_STRUC r_profile_temperature[] =VALUE_83_ZERO;

// ============================================================
// function prototype
// ============================================================
int fgauge_get_saddles(void);
BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature);

int fgauge_get_saddles_r_table(void);
R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature);

#endif	//#ifndef _CUST_BATTERY_METER_TABLE_H

