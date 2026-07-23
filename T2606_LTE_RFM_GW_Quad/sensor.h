#ifndef __SENSOR_H__
#define __SENSOR_H__

#define MAX_TOKENS      16
#define MAX_TOKEN_LEN   16

typedef enum
{
    SENSOR_VALUE_TEMP   = 'T',
    SENSOR_VALUE_HUM    = 'H',    
    SENSOR_VALUE_PRESS  = 'P',
    SENSOR_VALUE_LUX    = 'L',
    SENSOR_VALUE_PIR    = 'N',   
    SENSOR_VALUE_1      = '1',   
    SENSOR_VALUE_2      = '2',   
} sensor_value_et;  

typedef enum
{
    UNIT_TYPE_UNDEFINED = 0,
    UNIT_TYPE_TEMPERATURE,
    UNIT_TYPE_HUMIDITY,
    UNIT_TYPE_PRESSURE,
    UNIT_TYPE_LUX,
    UNIT_TYPE_PIR,
    UNIT_TYPE_FLOAT1
} sensor_unit_type_et;

typedef enum
{
    SENSOR_UNDEFINED,
    SENSOR_PIHA1,
    SENSOR_RANTA,
    SENSOR_KHH,
    SENSOR_NBR_OF,
} sensor_et;

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
} sensor_st;



// #define SENSOR_NBR_OF_SAVED_VALUES  8

// typedef struct 
// {
//     float value[SENSOR_NBR_OF_SAVED_VALUES];
//     uint8_t next_indx;
//     uint32_t next_save;
// } sensor_st;


void sensor_initialize(void);

void sensor_process_msg(void);
#endif