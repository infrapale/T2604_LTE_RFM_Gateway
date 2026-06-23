// ============================================================
//  Pico 2 W + SIMCom A7683E — SMS Receive + Auto‑Reply Framework
//  Clean, robust, production‑ready version
// ============================================================

#ifndef __LTE_H__
#define __LTE_H__


#include <Arduino.h>
#include "io.h"


#define LteSerial Serial1
#define SMS_LEN     160
#define MSG_LEN     512

typedef enum
{
    LTE_MSG_UNDEF = 0,
    LTE_MSG_EMPTY,
    LTE_MSG_HEADER_OK,
    LTE_MSG_BODY_OK,
    LTE_MSG_HEADER_INCORRECT,
    LTE_MSG_BODY_INCORRECT,
    LTE_MSG_NBR_OF
} lte_msg_et;

typedef struct
{
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   hour;
    uint8_t   minute;
    uint8_t   second;
 } date_time_st;

typedef enum 
{
    NAME_UNKNOWN = 0,
    NAME_TOM,
    NAME_HESSU,
    NAME_SAULI,
    NAME_NBR_OF
} contact_indx_st;


typedef struct 
{
    char message[MSG_LEN];
    char sender[32];
    char timestamp[32];
    char body[SMS_LEN];
    uint16_t cursor;
    date_time_st date_time;
    contact_indx_st contact_indx;
    bool available;
    bool complete;
}  lte_msg_st;


typedef struct
{
    char sender[32];
    char name[32];
} contact_st;

void lte_initialize(void);
uint16_t lte_read_line(char *lp, uint16_t max_len, uint32_t timeout);


#endif