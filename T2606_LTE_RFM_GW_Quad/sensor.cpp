/*******************************************************************************
  sensor.cpp
********************************************************************************
    Sensors:  
      T Temperature   fp max 2 decimals
      H Humidity      integer 0..100
      L Lux           16 bit integer 
      l LDR           16 bit integer 
      C CO2           16 bit integer 
      ! Remark        max 16 char 
      P PIR           1/0
      N Counter       (16 bit)
      B Battery       fp 2 decimals

<S;#;RANTA;T1;24.1>
<S;#;RANTA;T1;24.1;W1;12.8>
<S;#;PIHA;T1;22.1;L1;2009.1>

<S;PIHA1;T;25.0;H;45;L;7>
<T;#;PING;2026;07;10;05;55>

********************************************************************************

*******************************************************************************/

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "sensor.h"
#include "msg.h"
#include "r69.h"
#include "atask.h"


extern msg_st msg;
extern r69_st r69;

sensor_value_st value_array[20] = {0};

sensor_st sensor[SENSOR_NBR_OF] =
{   //                      Label      undef,   temp    hum     pres    lux     pir     bat     val1    val2 u  pdated
    [SENSOR_UNDEFINED]  = {"Undef",     {   0,      0,      0,      0,      0,      0,      0,      0,      0}, false}, 
    [SENSOR_PIHA1]      = {"PIHA1",     {   0,      1,      2,      0,      3,      0,      0,      0,      0}, false}, 
    [SENSOR_RANTA]      = {"RANTA",     {   0,      6,      7,      0,      0,      0,      0,      0,      0}, false}, 
    [SENSOR_KHH]        = {"KHH",       {   0,     10,     11,      0,      0,      0,      0,      0,      0}, false}, 
    [SENSOR_PARVEKE]    = {"Parveke",   {   0,     12,     13,      0,      0,      0,     14,      0,      0}, false}, 
};

char value_label[VALUE_NBR_OF][10] =
{   //                     123456789
    [VALUE_UNDEFINED]   = "Undef    ",
    [VALUE_TEMPERATURE] = "Temp     ",
    [VALUE_HUMIDITY]    = "Hum      ",
    [VALUE_PRESS]       = "Press    ",
    [VALUE_LUX]         = "LUX      ",
    [VALUE_PIR]         = "PIR      ",
    [VALUE_BAT]         = "Bat      ",
    [VALUE_1]           = "Value 1  ",
    [VALUE_2]           = "Value 2  ",

};

char value_tag[VALUE_NBR_OF] =
{   //                     123456789
    [VALUE_UNDEFINED]   = VALUE_TAG_UNDEF,
    [VALUE_TEMPERATURE] = VALUE_TAG_TEMP,
    [VALUE_HUMIDITY]    = VALUE_TAG_HUM,
    [VALUE_PRESS]       = VALUE_TAG_PRESS,
    [VALUE_LUX]         = VALUE_TAG_LUX,
    [VALUE_PIR]         = VALUE_TAG_PIR,
    [VALUE_BAT]         = VALUE_TAG_BAT,
    [VALUE_1]           = VALUE_TAG_1,
    [VALUE_2]           = VALUE_TAG_2,

};


void sensor_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st sensor_th           =    {"Sensor Task    ", 100,     0,     0,  255,    0,  1,  sensor_task };



void sensor_initialize(void)
{
    atask_add_new(&sensor_th);
}


void sensor_clear(uint8_t sindx)
{
    Serial.printf("Sensor Clear: %s\n", sensor[sindx].label);
    //sensor_print(sindx);

    for(uint8_t val_indx = VALUE_TEMPERATURE; val_indx < VALUE_NBR_OF; val_indx++ )
    {
        uint8_t arr_indx = sensor[sindx].value_indx[val_indx];
        if(arr_indx != 0)
        {
            value_array[arr_indx].last          = 0.0;
            value_array[arr_indx].min           = value_array[arr_indx].average;
            value_array[arr_indx].max           = value_array[arr_indx].average;
            value_array[arr_indx].average       = 0.0;
            value_array[arr_indx].daily_sum     = 0.0;
            value_array[arr_indx].daily_cntr    = 0;
            value_array[arr_indx].updated       = false;
        } 
    }
}

void sensor_clear_all(void)
{
    for (uint8_t sindx = 0; sindx < SENSOR_NBR_OF; sindx++)
    {
        sensor_clear(sindx);
    }
}

void sensor_print(uint8_t sindx)
{
    Serial.printf("Sensor: %s\n", sensor[sindx].label);
    for(uint8_t val_indx = VALUE_TEMPERATURE; val_indx < VALUE_NBR_OF; val_indx++ )
    {
        uint8_t arr_indx = sensor[sindx].value_indx[val_indx];
        if(arr_indx != 0)
        {
            Serial.printf("ArrIndx: %d - ", arr_indx);
            Serial.printf("%s  %0.2f: ",value_label[val_indx], value_array[arr_indx].last);
            Serial.printf("Min %0.2f: ",value_array[arr_indx].min);
            Serial.printf("Max %0.2f: ",value_array[arr_indx].max);
            Serial.printf("Avg %0.2f: ",value_array[arr_indx].average);
            Serial.printf("Sum %0.2f: ",value_array[arr_indx].daily_sum);
            Serial.printf("Cntr %d: ",value_array[arr_indx].daily_cntr);
            Serial.printf("Updated %d: \n",value_array[arr_indx].updated);
        } 
    }
}


uint8_t sensor_value_send(uint8_t sindx, uint8_t vindx)
{
    char buff[80] = {0};

    //Serial.printf("Write: %s\n", sensor[sindx].label);
    uint8_t arr_indx = sensor[sindx].value_indx[vindx];
    if(arr_indx != 0)
    {
        sprintf(buff,"<%s;%c;%0.2f;%0.2f;%0.2f;%0.2f;>",
            sensor[sindx].label,
            value_tag[vindx],
            value_array[arr_indx].last,
            value_array[arr_indx].min,
            value_array[arr_indx].max,
            value_array[arr_indx].average
        );
        Serial.println(buff);
    }
    return arr_indx;
}

void sensor_send(uint8_t sindx)
{
    Serial.printf("Sending: %s\n", sensor[sindx].label);
    for(uint8_t vindx = VALUE_TEMPERATURE; vindx < VALUE_NBR_OF; vindx++ )
    {
        sensor_value_send(sindx, vindx);
    }
}


uint8_t sensor_find_label(char *label)
{
    for (uint8_t i = 0; i < SENSOR_NBR_OF; i++)
    {
        if (strncmp(label, sensor[i].label, MAX_TOKEN_LEN) == 0)
        {
            return i;
        }
    }
    return 0;   // default index if not found
}

void sensor_store_value(uint8_t sindx, uint8_t vindx, float fval)
{
    uint8_t arr_indx = sensor[sindx].value_indx[vindx];
    // Serial.printf("sindx: %d vindx: %d arr_indx: %d fval: %f\n", sindx, vindx, arr_indx, fval);
    if((vindx > VALUE_UNDEFINED) && (vindx < VALUE_NBR_OF))
    {
        value_array[arr_indx].last = fval;
        if(value_array[arr_indx].daily_cntr == 0) 
            value_array[arr_indx].min = fval;
        else
            if(fval < value_array[arr_indx].min) value_array[arr_indx].min = fval;
        if(fval > value_array[arr_indx].max) value_array[arr_indx].max = fval;
        value_array[arr_indx].daily_sum += fval;
        value_array[arr_indx].daily_cntr++;
        value_array[arr_indx].updated = true;
        value_array[arr_indx].average = value_array[arr_indx].daily_sum / value_array[arr_indx].daily_cntr;
    }
    //sensor_print(sindx);
}

uint8_t sensor_save_values(uint8_t sindx)
{
    uint8_t findx = 2;
    bool    do_continue = true;
    uint8_t saved_values = 0;

    while ((findx < msg.field_count-1) && do_continue)
    {
        char *end;
        float fval = strtof(msg.fields[findx+1], &end);
        if ((end == msg.fields[findx+1]) || (*end != '\0'))
        {
            do_continue = false;
        }
        else
        {
            switch(msg.fields[findx][0])
            {
                case VALUE_TAG_TEMP:
                    sensor_store_value(sindx, VALUE_TEMPERATURE, fval);
                    break;
                case VALUE_TAG_HUM:
                    sensor_store_value(sindx, VALUE_HUMIDITY, fval);
                    break;
                case VALUE_TAG_PRESS:
                    sensor_store_value(sindx, VALUE_PRESS, fval);
                    break;
                case VALUE_TAG_LUX:
                    sensor_store_value(sindx, VALUE_LUX, fval);
                    break;
                case VALUE_TAG_PIR:
                    sensor_store_value(sindx, VALUE_PIR, fval);
                    break;
                case VALUE_TAG_BAT:
                    sensor_store_value(sindx, VALUE_BAT, fval);
                    break;
                case VALUE_TAG_1:
                    sensor_store_value(sindx, VALUE_1, fval);
                    break;
                case VALUE_TAG_2:
                    sensor_store_value(sindx, VALUE_2, fval);
                    break;
            }
        }
        findx += 2;
    }
    sensor_print(sindx);
    sensor[sindx].updated = true;

    return saved_values;

} 

void sensor_process_msg(uint8_t  nbr_fields)
{
    uint8_t sindx = SENSOR_UNDEFINED;

    // Serial.printf("Split nbr %d\n",nbr_fields);
    // msg_sub_print();

    switch(msg.fields[0][0])
    {
        case 'S':
            sindx = sensor_find_label(msg.fields[1]);
            switch(sindx)
            {
                case SENSOR_UNDEFINED:
                    break;
                case SENSOR_PIHA1:
                    sensor_save_values(sindx);
                    break;
                case SENSOR_RANTA:
                    sensor_save_values(sindx);
                    break;
                case SENSOR_KHH:
                    sensor_save_values(sindx);
                    break;
                case SENSOR_PARVEKE:
                    sensor_save_values(sindx);
                    break;
            }
            break;
        case 'T':
            break;
         
    }
    

}


void sensor_task(void)
{
    static uint32_t next_send;
    static uint8_t sensor_indx;
    static uint8_t value_indx;
    // Serial.printf("sensor_task: state: %d sensor: %d value: %d\n", sensor_th.state, sensor_indx, value_indx);
    switch(sensor_th.state)
    {
        case 0:
            sensor_th.state = 10;
            sensor_indx = SENSOR_UNDEFINED;
            value_indx = VALUE_TEMPERATURE;
            break;
        case 10:
            sensor_indx++;
            if (sensor_indx >= SENSOR_NBR_OF){
                sensor_indx = SENSOR_PIHA1;
                value_indx = VALUE_TEMPERATURE;
            } 
            sensor_th.state = 15;
            break;
        case 15:
            if (sensor[sensor_indx].updated) sensor_th.state = 30;
            else sensor_th.state = 10;
            break;
        case 20:
            sensor_th.state = 10;
            break;
        case 30:
            if(sensor_value_send(sensor_indx, value_indx) > 0)
            {
                next_send = millis() + 100;
                sensor_th.state = 40;
            }            
            else sensor_th.state = 10;
            break;
        case 40:
            if(millis() > next_send) sensor_th.state = 50;
            break;
        case 50:
            value_indx++;
            if(value_indx >= VALUE_NBR_OF) 
            {
                value_indx = VALUE_TEMPERATURE;
                sensor_th.state = 10;
            }    
            else {
                sensor_th.state = 15;
            }
            break;
        case 100:
            sensor_th.state = 10;
            break;
    }
}

// <S;PIHA1;T;25.3;H;43;L;9>
// Split nbr 8
// Message fields; 8
// [0] = S
// [1] = PIHA1
// [2] = T
// [3] = 25.3
// [4] = H
// [5] = 43
// [6] = L
// [7] = 9





