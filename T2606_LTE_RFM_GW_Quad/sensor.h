#ifndef __SENSOR_H__
#define __SENSOR_H__

#define SENSOR_NBR_OF_SAVED_VALUES  8
typedef struct 
{
    float value[SENSOR_NBR_OF_SAVED_VALUES],
    uint8_t next_indx,
    uint32_t next_save
}


#endif