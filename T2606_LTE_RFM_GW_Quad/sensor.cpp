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
#include "msg.h"
#include "r69.h"
#include "atask.h"



#define MAX_TOKENS 16
#define MAX_TOKEN_LEN 16

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



