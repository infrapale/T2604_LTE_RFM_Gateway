#ifndef __SENSOR_H__
#define __SENSOR_H__

#define MAX_TOKENS      16
#define MAX_TOKEN_LEN   16

typedef enum
{
    VALUE_TAG_UNDEF  = '#',
    VALUE_TAG_TEMP   = 'T',
    VALUE_TAG_HUM    = 'H',    
    VALUE_TAG_PRESS  = 'P',
    VALUE_TAG_LUX    = 'L',
    VALUE_TAG_PIR    = 'N',   
    VALUE_TAG_BAT    = 'B',   
    VALUE_TAG_1      = '1',   
    VALUE_TAG_2      = '2',   
} sensor_value_et;  

// typedef enum
// {
//     UNIT_TYPE_TEMPERATURE = 0,
//     UNIT_TYPE_HUMIDITY,
//     UNIT_TYPE_PRESSURE,
//     UNIT_TYPE_LUX,
//     UNIT_TYPE_PIR,
//     UNIT_TYPE_FLOAT1,
//     UNIT_TYPE_FLOAT2,
//     UNIT_TYPE_NBR_OF,
// } sensor_unit_type_et;


typedef enum
{
    SENSOR_UNDEFINED,
    SENSOR_PIHA1,
    SENSOR_RANTA,
    SENSOR_KHH,
    SENSOR_PARVEKE,
    SENSOR_RUUVI_3072,
    SENSOR_RUUVI_939B,
    SENSOR_RUUVI_1FEA,
    SENSOR_RUUVI_MH1,
    SENSOR_NBR_OF,
} sensor_et;

typedef enum
{
    VALUE_UNDEFINED = 0,
    VALUE_TEMPERATURE,
    VALUE_HUMIDITY,
    VALUE_PRESS,
    VALUE_LUX,
    VALUE_PIR,
    VALUE_BAT,
    VALUE_1,
    VALUE_2,
    VALUE_NBR_OF
}  value_et;

typedef struct
{
    float last;
    float min;
    float max;
    float average;
    float daily_sum;
    uint16_t    daily_cntr;
    bool        updated;

} sensor_value_st;


typedef struct
{
    char            label[MAX_TOKEN_LEN];
    uint8_t         value_indx[VALUE_NBR_OF];
    bool            updated;
    //uint16_t        enable_bm;
    //sensor_value_st value[VALUE_NBR_OF];
} sensor_st;


typedef struct
{
    char        label[MAX_TOKEN_LEN];
    float       temperature;
    float       humidity;
    float       lux;
    float       pir;
    float       value1;
    float       value2;
    bool        updated;
} xxsensor_st;

// typedef struct
// {
//     char        label[MAX_TOKEN_LEN];
//     sensor_value_st data[UNIT_TYPE_NBR_OF];
//     //bool        updated;
// } zzz;




// #define SENSOR_NBR_OF_SAVED_VALUES  8

// typedef struct 
// {
//     float value[SENSOR_NBR_OF_SAVED_VALUES];
//     uint8_t next_indx;
//     uint32_t next_save;
// } sensor_st;


void sensor_initialize(void);

void sensor_clear(uint8_t sindx);

void sensor_clear_all(void);

void sensor_print(uint8_t sindx);

void sensor_process_msg(uint8_t nbr_fields);

#endif