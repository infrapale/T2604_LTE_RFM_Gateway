/*******************************************************************************
clock.cpp
********************************************************************************
Radio message:
  <T;#;PING;2026;07;10;05;55>

********************************************************************************

*******************************************************************************/

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "clock.h"
#include "main.h"
#include "sensor.h"
#include "msg.h"
#include "r69.h"
#include "atask.h"

extern msg_st msg;
extern r69_st r69;
extern main_ctrl_st main_ctrl;

void clock_task(void);

clock_st clock_mgr;



// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st clock_th       = {"Clock Task     ", 1000,    0,     0,  255,    0,   1, clock_task};

void clock_initialize(void)
{
    atask_add_new(&clock_th);
    clock_mgr.time_epoch = 0;  
    memset(&clock_mgr.my_time, 0, sizeof(clock_mgr.my_time));

    clock_mgr.next_minute = millis() + 60000;
    clock_mgr.last_hour = 0;
    clock_mgr.new_hour_event = false;
}

void clock_print_date_time(const struct tm *t)
{ 
    char buff[40];
    //strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S (%Z)", t);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", t);
    Serial.println(buff);
    
}

void clock_set_date_time(void)
{
    struct tm tmp_time = {0};
    Serial.println("clock_set_date_time");

    uint8_t errors = 0;
    Serial.printf("Time: %s\n", msg.raw);

    if ((msg.field_count == 8) && (msg.fields[1][0] == '#'))
    {
        // Year: 2026 → tm_year = 126
        tmp_time.tm_year = (uint16_t)msg_robust_atoi(msg.fields[3], &errors, 2000, 2100) - 1900;

        // Month: 1–12 in message → 0–11 in tm
        uint8_t month = (uint8_t)msg_robust_atoi(msg.fields[4], &errors, 1, 12);
        tmp_time.tm_mon = month - 1;

        tmp_time.tm_mday = (uint8_t)msg_robust_atoi(msg.fields[5], &errors, 1, 31);
        tmp_time.tm_hour = (uint8_t)msg_robust_atoi(msg.fields[6], &errors, 0, 23);
        tmp_time.tm_min  = (uint8_t)msg_robust_atoi(msg.fields[7], &errors, 0, 59);
        tmp_time.tm_sec  = 0;
        tmp_time.tm_isdst = -1;   // let mktime figure it out

        Serial.printf("time errors %d\n", errors);
        clock_print_date_time(&tmp_time);

        if (errors == 0) {
            time_t t = mktime(&tmp_time);   // normalize into time_t

            if (t == (time_t)-1) {
                Serial.println("mktime failed (out of range)");
            } else {
                // Store both time_t and normalized tm
                clock_mgr.time_epoch = t;
                clock_mgr.my_time    = tmp_time;
                clock_print_date_time(&clock_mgr.my_time);
            }
        } else {
            Serial.println("!!!msg_time_action: Integer conversion Error");
        }
    }
    else {
        Serial.println("Incorrect Time message");
    }

    clock_mgr.next_minute = millis() + 60000;
}


void xxclock_set_date_time(void)  // deprecated
{
    struct tm   tmp_time = {0};
    Serial.println("clock_set_date_time");

    uint8_t errors = 0;
    Serial.printf("Time: %s\n", msg.raw);
    if((msg.field_count == 8) && (msg.fields[1][0] == '#'))
    {
        tmp_time.tm_year  = (uint16_t)msg_robust_atoi(msg.fields[3],&errors,2000,2100) -1900;    
        tmp_time.tm_mon   = (uint8_t)msg_robust_atoi(msg.fields[4],&errors,1,12) -1;    
        tmp_time.tm_mday  = (uint8_t)msg_robust_atoi(msg.fields[5],&errors,1,31);
        tmp_time.tm_hour  = (uint8_t)msg_robust_atoi(msg.fields[6],&errors,0,23);
        tmp_time.tm_min   = (uint8_t)msg_robust_atoi(msg.fields[7],&errors,0,59);
        tmp_time.tm_sec = 0;
        tmp_time.tm_isdst = -1;
        Serial.println();
        Serial.printf("time errors %d\n",errors);
        clock_print_date_time(&tmp_time);
        if (errors==0) {
            time_t t = mktime(&tmp_time);
            //clock_print_date_time(&t);
            clock_mgr.my_time = *localtime(&t);
            clock_print_date_time(&clock_mgr.my_time);
        }
        else Serial.println("!!!msg_time_action: Integer conversion Error");
    }
    else {
        Serial.println("Incorrect Time message");
    }
    clock_mgr.next_minute = millis() + 60000;
}

void clock_task(void)
{
    if (millis() > clock_mgr.next_minute)
    {
        // Advance canonical epoch time by one minute
        clock_mgr.time_epoch += 60;

        if (clock_mgr.time_epoch < 0 || clock_mgr.time_epoch > 4102444800) {
            // 2100-01-01 safety range
            return;
        }
        // Derive broken-down time from epoch
        struct tm *lt = localtime(&clock_mgr.time_epoch);
        if (lt != nullptr) {
            clock_mgr.my_time = *lt;
            // clock_print_date_time(&clock_mgr.my_time);
        }

        clock_mgr.next_minute += 60000;   // real minute; use 6000 only for testing
    }

    if (clock_mgr.last_hour != clock_mgr.my_time.tm_hour)
    {
        clock_mgr.last_hour = clock_mgr.my_time.tm_hour;
        msg_send_repo1();
        sensor_clear_all();
    }
}


void xxclock_task(void)  // deprecated
{
    if (millis() > clock_mgr.next_minute)
    {
        clock_mgr.my_time.tm_min += 1;
        time_t t = mktime(&clock_mgr.my_time);  // normalize (handles overflow)
        clock_mgr.my_time = *localtime(&t);     // write back normalized result
        // clock_print_date_time(&clock_mgr.my_time);

        //clock_mgr.next_minute += 60000;
        clock_mgr.next_minute += 6000;
    }
    if ( clock_mgr.last_hour != clock_mgr.my_time.tm_hour)
    {
        clock_mgr.last_hour = clock_mgr.my_time.tm_hour;
        // Serial.printf("Hour: %d\n", clock_mgr.my_time.tm_hour);
        msg_send_repo1();
        sensor_clear_all();
        // switch(clock_mgr.my_time.tm_hour)
        // {
        //     case 8:
        //         msg_send_repo1();
        //         break;
        // }
    }
}
