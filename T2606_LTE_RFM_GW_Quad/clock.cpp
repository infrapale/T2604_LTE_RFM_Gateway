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
    clock_mgr.next_minute = millis() + 60000;
    clock_mgr.last_hour = 0;
    clock_mgr.new_hour_event = false;
}

void clock_print_date_time(struct tm   *date_time)
{
    strftime(clock_mgr.buff, sizeof(clock_mgr.buff), "%Y-%m-%d %H:%M:%S (%Z)", date_time);
    Serial.println(clock_mgr.buff);
    
}

void clock_set_date_time(void)
{
    struct tm   tmp_time;
    Serial.println("clock_set_date_time");

    uint8_t errors = 0;
    Serial.printf("Time: %s\n", msg.raw);
    if((msg.field_count == 8) && (msg.fields[1][0] == '#'))
    {
        tmp_time.tm_year  = (uint16_t)msg_robust_atoi(msg.fields[3],&errors,2000,2100) -1900;    
        tmp_time.tm_mon   = (uint8_t)msg_robust_atoi(msg.fields[4],&errors,1,12);    
        tmp_time.tm_mday  = (uint8_t)msg_robust_atoi(msg.fields[5],&errors,1,31);
        tmp_time.tm_hour  = (uint8_t)msg_robust_atoi(msg.fields[6],&errors,0,24);
        tmp_time.tm_min   = (uint8_t)msg_robust_atoi(msg.fields[7],&errors,0,60);
        Serial.printf("time errors %d\n",errors);
        if (errors==0) {
            time_t t = mktime(&tmp_time);
            clock_mgr.my_time = *localtime(&t);
        }
        else Serial.println("!!!msg_time_action: Integer conversion Error");
    }
    else {
        Serial.println("Incorrect Time message");
    }



    strftime(clock_mgr.buff, sizeof(clock_mgr.buff), "%Y-%m-%d %H:%M:%S (%Z)", &clock_mgr.my_time);
    Serial.println(clock_mgr.buff);
    clock_mgr.next_minute = millis() + 60000;
}

void clock_task(void)
{
    if (millis() > clock_mgr.next_minute)
    {
        clock_mgr.my_time.tm_min += 1;
        time_t t = mktime(&clock_mgr.my_time);  // normalize (handles overflow)
        clock_mgr.my_time = *localtime(&t);     // write back normalized result
        clock_print_date_time(&clock_mgr.my_time);

        clock_mgr.next_minute += 60000;
    }
    if ( clock_mgr.last_hour != clock_mgr.my_time.tm_hour)
    {
        clock_mgr.last_hour = clock_mgr.my_time.tm_hour;
        Serial.printf("Hour: %d", clock_mgr.my_time.tm_hour);
        switch(clock_mgr.my_time.tm_hour)
        {
            case 8:
                msg_send_repo1();
                break;
        }
    }
}
