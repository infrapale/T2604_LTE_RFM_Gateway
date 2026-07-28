#ifndef __CLOCK_H__
#define __CLOCK_H__


typedef struct 
{
    struct tm   my_time;
    char        buff[64];
    uint32_t    next_minute;
    uint8_t     last_hour;
    bool        new_hour_event;
} clock_st;

void clock_initialize(void);

void clock_set_date_time(void);

#endif