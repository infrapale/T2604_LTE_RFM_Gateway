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


sensor_st sensor[SENSOR_NBR_OF] =
{
    [SENSOR_UNDEFINED]  = {"Undef", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false},
    [SENSOR_PIHA1]      = {"PIHA1", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false},
    [SENSOR_RANTA]      = {"RANTA", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false},
    [SENSOR_KHH]        = {"KHH",   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false}
};

void sensor_print(uint8_t sindx)
{
    Serial.printf("Sensor: %s: Temp: %0.2f Hum: %0.2f Lux: %0.2f Pir: %0.2f Value1: %0.2f Value2: %0.2f\n",
        sensor[sindx].label,
        sensor[sindx].temperature,
        sensor[sindx].humidity,
        sensor[sindx].lux,
        sensor[sindx].pir,
        sensor[sindx].value1,
        sensor[sindx].value2
        );
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
                case SENSOR_VALUE_TEMP:
                    sensor[sindx].temperature = fval;
                    saved_values++;
                    break;
                case SENSOR_VALUE_HUM:
                    sensor[sindx].humidity = fval;
                    saved_values++;
                    break;
                case SENSOR_VALUE_PRESS:
                    sensor[sindx].value1 = fval;
                    saved_values++;
                    break;
                case SENSOR_VALUE_LUX:
                    sensor[sindx].lux = fval;
                    saved_values++;
                    break;
                case SENSOR_VALUE_PIR:
                    sensor[sindx].pir = fval;
                    saved_values++;
                    break;
                case SENSOR_VALUE_1:
                    sensor[sindx].value1 = fval;
                    saved_values++;
                    break;
                case SENSOR_VALUE_2:
                    sensor[sindx].value2 = fval;
                    saved_values++;
                    break;
            }
        }
        findx += 2;
    }
    sensor_print(sindx);
    if(saved_values > 0) sensor[sindx].updated = true;

    return saved_values;

} 

void sensor_process_msg(void)
{
    uint8_t  nbr_fields =0;

    nbr_fields = msg_split(r69.rxbuff,';');
    Serial.printf("Split nbr %d\n",nbr_fields);
    msg_sub_print();

    if(msg.fields[0][0] == 'S')
    {
        uint8_t sindx = sensor_find_label(msg.fields[1]);
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
        }
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



void sensor_split_message(const char *msg, char tokens[MAX_TOKENS][MAX_TOKEN_LEN], int *count)
{
    int i = 0, t = 0, c = 0;

    while (msg[i] != '\0' && t < MAX_TOKENS) {

        if (msg[i] == '<' || msg[i] == '>') {
            i++;
            continue;
        }

        if (msg[i] == ';') {
            tokens[t][c] = '\0';   // end current token
            t++;
            c = 0;
            i++;
            continue;
        }

        if (c < (MAX_TOKEN_LEN - 1)) {
            tokens[t][c++] = msg[i];
        }

        i++;
    }

    tokens[t][c] = '\0';
    *count = t + 1;
}

int sensor_test() {
    const char *msg = "<S;PIHA1;T;25.0;H;45;L;7>";
    char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int count = 0;

    sensor_split_message(msg, tokens, &count);

    for (int i = 0; i < count; i++) {
        Serial.printf("Token %d: %s\n", i, tokens[i]);
    }

    return 0;
}

void sensor_initialize(void)
{
    Serial.println("Sensor Test:");
    sensor_test();
}




